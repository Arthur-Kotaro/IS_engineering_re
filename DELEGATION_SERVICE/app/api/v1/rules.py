# app/api/v1/rules.py
from fastapi import APIRouter, Depends, HTTPException, status, Request
from typing import List
from app.database import get_db
from app.schemas.rule import DelegationRuleCreate, DelegationRuleResponse
from app.repositories.rule_repo import RuleRepository
from sqlalchemy.ext.asyncio import AsyncSession
import jwt
from app.config import settings

router = APIRouter(prefix="/api/v1/delegation-rules", tags=["Delegation Rules"])


async def get_rule_repo(db: AsyncSession = Depends(get_db)) -> RuleRepository:
    return RuleRepository(db)


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


@router.get("/", response_model=List[DelegationRuleResponse])
async def get_all_rules(
    repo: RuleRepository = Depends(get_rule_repo),
    is_super_admin: bool = Depends(get_is_super_admin)
):
    """Получить все правила делегирования (только для супер-админов)"""
    if not is_super_admin:
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="Only super admin can view rules"
        )
    return await repo.get_all()


@router.get("/{role}", response_model=DelegationRuleResponse)
async def get_rule_by_role(
    role: str,
    repo: RuleRepository = Depends(get_rule_repo),
    is_super_admin: bool = Depends(get_is_super_admin)
):
    """Получить правило для роли (только для супер-админов)"""
    if not is_super_admin:
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="Only super admin can view rules"
        )
    rule = await repo.get_by_role(role)
    if not rule:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Rule for role '{role}' not found"
        )
    return rule


@router.post("/", response_model=DelegationRuleResponse, status_code=status.HTTP_201_CREATED)
async def create_rule(
    data: DelegationRuleCreate,
    repo: RuleRepository = Depends(get_rule_repo),
    is_super_admin: bool = Depends(get_is_super_admin)
):
    """Создать правило делегирования (только для супер-админов)"""
    if not is_super_admin:
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="Only super admin can create rules"
        )
    
    existing = await repo.get_by_role(data.role)
    if existing:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail=f"Rule for role '{data.role}' already exists"
        )
    
    return await repo.create(**data.model_dump())


@router.put("/{role}", response_model=DelegationRuleResponse)
async def update_rule(
    role: str,
    data: DelegationRuleCreate,
    repo: RuleRepository = Depends(get_rule_repo),
    is_super_admin: bool = Depends(get_is_super_admin)
):
    """Обновить правило делегирования (только для супер-админов)"""
    if not is_super_admin:
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="Only super admin can update rules"
        )
    
    rule = await repo.get_by_role(role)
    if not rule:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Rule for role '{role}' not found"
        )
    
    updated = await repo.update(rule.rule_id, **data.model_dump())
    return updated
