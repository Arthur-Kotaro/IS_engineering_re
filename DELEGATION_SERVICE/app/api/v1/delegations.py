# app/api/v1/delegations.py
from fastapi import APIRouter, Depends, HTTPException, status, Query, Request
from typing import List, Optional
from app.database import get_db
from app.schemas.delegation import (
    DelegationCreate,
    DelegationResponse,
    DelegationRevoke,
    DelegationListResponse,
    DelegationCheckResponse
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


async def get_current_user_id(request: Request) -> int:
    """Получить ID текущего пользователя из заголовка X-User-ID"""
    user_id = request.headers.get("X-User-ID")
    if not user_id:
        raise HTTPException(401, "Missing X-User-ID header")
    try:
        return int(user_id)
    except ValueError:
        raise HTTPException(401, "Invalid X-User-ID format")


async def get_token(request: Request) -> Optional[str]:
    """Получить токен из заголовка Authorization"""
    auth_header = request.headers.get("Authorization")
    if not auth_header:
        return None
    return auth_header.replace("Bearer ", "").strip()


async def get_is_super_admin(request: Request) -> bool:
    """Проверить, является ли пользователь супер-админом"""
    auth_header = request.headers.get("Authorization")
    if not auth_header:
        return False
    token = auth_header.replace("Bearer ", "").strip()
    try:
        payload = jwt.decode(token, options={"verify_signature": False})
        return payload.get("is_super_admin", False)
    except Exception:
        return False


@router.post("/direct", response_model=DelegationResponse)
async def create_direct_delegation(
    data: DelegationCreate,
    request: Request,
    current_user_id: int = Depends(get_current_user_id),
    token: Optional[str] = Depends(get_token),
    service: DelegationService = Depends(get_delegation_service)
):
    """
    Прямое делегирование: руководитель → подчиненный
    
    Используется когда руководитель на больничном/в отпуске.
    Подчиненный временно получает все полномочия руководителя.
    """
    if data.delegator_id != current_user_id:
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="You can only delegate your own permissions"
        )
    
    return await service.create_delegation(
        data,
        created_by=current_user_id,
        token=token,
        ip_address=request.client.host,
        user_agent=request.headers.get("user-agent")
    )


@router.post("/reverse", response_model=DelegationResponse)
async def create_reverse_delegation(
    data: DelegationCreate,
    request: Request,
    current_user_id: int = Depends(get_current_user_id),
    token: Optional[str] = Depends(get_token),
    service: DelegationService = Depends(get_delegation_service)
):
    """
    Обратное делегирование: руководитель → подчиненный
    
    Руководитель входит от своего имени, но с полномочиями подчиненного.
    Используется для решения задач, с которыми подчиненный не справляется.
    """
    if data.delegator_id != current_user_id:
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="You can only delegate your own permissions"
        )
    
    return await service.create_delegation(
        data,
        created_by=current_user_id,
        token=token,
        ip_address=request.client.host,
        user_agent=request.headers.get("user-agent")
    )


@router.post("/temporary", response_model=DelegationResponse)
async def create_temporary_delegation(
    data: DelegationCreate,
    request: Request,
    current_user_id: int = Depends(get_current_user_id),
    token: Optional[str] = Depends(get_token),
    service: DelegationService = Depends(get_delegation_service)
):
    """
    Временное делегирование: руководитель → другой подчиненный
    
    Используется когда основной подчиненный временно отсутствует.
    Полномочия отсутствующего сотрудника передаются другому подчиненному.
    """
    if data.delegator_id != current_user_id:
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="You can only delegate your own permissions"
        )
    
    return await service.create_delegation(
        data,
        created_by=current_user_id,
        token=token,
        ip_address=request.client.host,
        user_agent=request.headers.get("user-agent")
    )


@router.get("/active/me", response_model=List[DelegationResponse])
async def get_my_active_delegations(
    current_user_id: int = Depends(get_current_user_id),
    service: DelegationService = Depends(get_delegation_service)
):
    """Получить все активные делегирования текущего пользователя"""
    return await service.get_active_delegations(current_user_id)


@router.get("/active/{user_id}", response_model=List[DelegationResponse])
async def get_user_active_delegations(
    user_id: int,
    current_user_id: int = Depends(get_current_user_id),
    is_super_admin: bool = Depends(get_is_super_admin),
    service: DelegationService = Depends(get_delegation_service)
):
    """Получить активные делегирования пользователя (только для админов)"""
    if user_id != current_user_id and not is_super_admin:
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="You can only view your own delegations"
        )
    return await service.get_active_delegations(user_id)


@router.get("/check/{user_id}", response_model=dict)
async def check_user_delegation(
    user_id: int,
    service: DelegationService = Depends(get_delegation_service)
):
    """Проверить, есть ли у пользователя активное делегирование"""
    return await service.check_delegation_for_user(user_id)


@router.get("/", response_model=DelegationListResponse)
async def get_all_delegations(
    skip: int = Query(0, ge=0),
    limit: int = Query(100, ge=1, le=500),
    service: DelegationService = Depends(get_delegation_service),
    is_super_admin: bool = Depends(get_is_super_admin)
):
    """Получить все делегирования (только для супер-админов)"""
    if not is_super_admin:
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="Only super admin can view all delegations"
        )
    delegations = await service.delegation_repo.get_all(skip, limit)
    total = len(delegations)
    return DelegationListResponse(
        total=total,
        delegations=[DelegationResponse.model_validate(d) for d in delegations]
    )


@router.delete("/{delegation_id}", response_model=DelegationResponse)
async def revoke_delegation(
    delegation_id: int,
    data: Optional[DelegationRevoke] = None,
    request: Request = None,
    current_user_id: int = Depends(get_current_user_id),
    is_super_admin: bool = Depends(get_is_super_admin),
    service: DelegationService = Depends(get_delegation_service)
):
    """Отозвать делегирование (только создатель или супер-админ)"""
    return await service.revoke_delegation(
        delegation_id,
        user_id=current_user_id,
        data=data or DelegationRevoke(),
        is_super_admin=is_super_admin,
        ip_address=request.client.host if request else None,
        user_agent=request.headers.get("user-agent") if request else None
    )


@router.get("/history/{user_id}")
async def get_delegation_history(
    user_id: int,
    limit: int = Query(50, ge=1, le=500),
    current_user_id: int = Depends(get_current_user_id),
    is_super_admin: bool = Depends(get_is_super_admin),
    service: DelegationService = Depends(get_delegation_service)
):
    """Получить историю делегирований пользователя"""
    if user_id != current_user_id and not is_super_admin:
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="You can only view your own history"
        )
    history = await service.history_repo.get_by_user(user_id, limit)
    return {"user_id": user_id, "history": history}
