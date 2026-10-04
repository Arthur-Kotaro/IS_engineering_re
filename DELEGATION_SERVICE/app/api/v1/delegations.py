# app/api/v1/delegations.py
from fastapi import APIRouter, Depends, HTTPException, status, Query, Request
from typing import List, Optional
from app.database import get_db
from app.schemas.delegation import (
    DelegationCreate,
    DelegationResponse,
    DelegationRevoke,
    DelegationListResponse,
)
from app.services.delegation_service import DelegationService
from app.services.validation_service import ValidationService
from app.repositories.delegation_repo import DelegationRepository
from app.repositories.history_repo import HistoryRepository
from app.repositories.rule_repo import RuleRepository
from sqlalchemy.ext.asyncio import AsyncSession
import jwt
from app.config import settings

router = APIRouter(prefix="/api/v1/delegations", tags=["Delegations"])


async def get_delegation_service(db: AsyncSession = Depends(get_db)) -> DelegationService:
    delegation_repo = DelegationRepository(db)
    history_repo = HistoryRepository(db)
    rule_repo = RuleRepository(db)
    validation_service = ValidationService(delegation_repo, rule_repo)
    return DelegationService(delegation_repo, history_repo, validation_service)


def _get_header_user_id(request: Request) -> int:
    uid = request.headers.get("X-User-ID")
    if not uid:
        raise HTTPException(401, "Missing X-User-ID")
    try:
        return int(uid)
    except ValueError:
        raise HTTPException(401, "Invalid X-User-ID")


def _get_token(request: Request) -> Optional[str]:
    auth = request.headers.get("Authorization")
    if not auth:
        return None
    return auth.removeprefix("Bearer ").strip()


def _is_super_admin(request: Request) -> bool:
    role = request.headers.get("X-User-Role", "")
    return "super_admin" in role.split(",")


@router.post("", response_model=DelegationResponse)
async def create_delegation(
    data: DelegationCreate,
    request: Request,
    service: DelegationService = Depends(get_delegation_service),
):
    current_user_id = _get_header_user_id(request)
    if _is_super_admin(request) is False and data.delegator_id != current_user_id and data.delegation_type == "direct":
        raise HTTPException(403, "You can only delegate your own permissions")

    data.initiator_id = current_user_id

    return await service.create_delegation(
        data,
        created_by=current_user_id,
        token=_get_token(request),
        ip_address=request.client.host if request.client else None,
        user_agent=request.headers.get("user-agent"),
    )


@router.get("/active/me", response_model=List[DelegationResponse])
async def get_my_active(
    request: Request,
    service: DelegationService = Depends(get_delegation_service),
):
    user_id = _get_header_user_id(request)
    return await service.get_active_for_user(user_id)


@router.get("/active/as-delegator", response_model=List[DelegationResponse])
async def get_active_as_delegator(
    request: Request,
    service: DelegationService = Depends(get_delegation_service),
):
    user_id = _get_header_user_id(request)
    return await service.get_active_as_delegator(user_id)


@router.get("/active/{user_id}", response_model=List[DelegationResponse])
async def get_user_active(
    user_id: int,
    request: Request,
    service: DelegationService = Depends(get_delegation_service),
):
    current_user_id = _get_header_user_id(request)
    if user_id != current_user_id and not _is_super_admin(request):
        raise HTTPException(403, "Forbidden")
    return await service.get_active_for_user(user_id)


@router.get("/check", response_model=dict)
async def check_delegation(
    delegate_id: int = Query(...),
    delegator_id: int = Query(...),
    service: DelegationService = Depends(get_delegation_service),
):
    return await service.check_active(delegate_id, delegator_id)


@router.get("/", response_model=DelegationListResponse)
async def get_all(
    skip: int = Query(0, ge=0),
    limit: int = Query(100, ge=1, le=500),
    request: Request = None,
    service: DelegationService = Depends(get_delegation_service),
):
    if not _is_super_admin(request):
        raise HTTPException(403, "Only super admin can view all delegations")
    items = await service.delegation_repo.get_all(skip, limit)
    return DelegationListResponse(
        total=len(items),
        delegations=[DelegationResponse.model_validate(d) for d in items],
    )


@router.post("/{delegation_id}/revoke", response_model=DelegationResponse)
async def revoke_delegation(
    delegation_id: int,
    data: Optional[DelegationRevoke] = None,
    request: Request = None,
    service: DelegationService = Depends(get_delegation_service),
):
    user_id = _get_header_user_id(request)
    return await service.revoke_delegation(
        delegation_id,
        user_id=user_id,
        data=data or DelegationRevoke(),
        is_super_admin=_is_super_admin(request),
        ip_address=request.client.host if request.client else None,
        user_agent=request.headers.get("user-agent"),
    )


@router.get("/history/{user_id}")
async def get_history(
    user_id: int,
    limit: int = Query(50, ge=1, le=500),
    request: Request = None,
    service: DelegationService = Depends(get_delegation_service),
):
    current_user_id = _get_header_user_id(request)
    if user_id != current_user_id and not _is_super_admin(request):
        raise HTTPException(403, "Forbidden")
    history = await service.history_repo.get_by_user(user_id, limit)
    return {"user_id": user_id, "history": history}
