# app/services/validation_service.py
from fastapi import HTTPException, status
from datetime import datetime, timezone
from typing import Optional
from app.models.delegation import DelegationType
from app.services.external_service import ExternalService
from app.repositories.delegation_repo import DelegationRepository
from app.repositories.rule_repo import RuleRepository
from app.config import settings


class ValidationService:
    def __init__(self, delegation_repo: DelegationRepository, rule_repo: RuleRepository):
        self.delegation_repo = delegation_repo
        self.rule_repo = rule_repo
    
    async def validate_direct_delegation(
        self,
        delegator_id: int,
        delegate_id: int,
        starts_at: datetime,
        expires_at: datetime,
        token: Optional[str] = None
    ):
        """Валидация прямого делегирования"""
        
        # 1. Проверяем, что delegator является руководителем delegate
        is_manager = await ExternalService.is_manager_of(delegator_id, delegate_id, token)
        if not is_manager:
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="Only manager can delegate to subordinate"
            )
        
        # 2. Проверяем роли и правила
        roles = await ExternalService.get_user_roles(delegator_id, token)
        rule = await self.rule_repo.get_by_role("admin") if "admin" in roles else None
        if not rule:
            rule = await self.rule_repo.get_by_role("manager") if "manager" in roles else None
        
        if not rule or not rule.can_delegate:
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="User does not have permission to delegate"
            )
        
        # 3. Проверяем количество активных делегирований
        active_count = await self.delegation_repo.get_active_count(delegator_id)
        if active_count >= rule.max_delegations:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail=f"Maximum delegations ({rule.max_delegations}) reached"
            )
        
        # 4. Проверяем длительность
        duration_days = (expires_at - starts_at).days
        if duration_days > rule.max_duration_days:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail=f"Delegation duration exceeds maximum ({rule.max_duration_days} days)"
            )
        
        # 5. Проверяем, что delegate не имеет активного делегирования
        active_delegations = await self.delegation_repo.get_active_delegations_for_user(delegate_id)
        if active_delegations:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="Delegate already has active delegation"
            )
    
    async def validate_reverse_delegation(
        self,
        delegator_id: int,
        delegate_id: int,
        starts_at: datetime,
        expires_at: datetime,
        token: Optional[str] = None
    ):
        """Валидация обратного делегирования"""
        
        # 1. Проверяем, что delegator является руководителем delegate
        is_manager = await ExternalService.is_manager_of(delegator_id, delegate_id, token)
        if not is_manager:
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="Only manager can delegate to subordinate"
            )
        
        # 2. Проверяем правила для reverse делегирования
        roles = await ExternalService.get_user_roles(delegator_id, token)
        rule = await self.rule_repo.get_by_role("admin") if "admin" in roles else None
        if not rule:
            rule = await self.rule_repo.get_by_role("manager") if "manager" in roles else None
        
        if not rule or not rule.reverse_allowed:
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="Reverse delegation is not allowed for this user"
            )
    
    async def validate_temporary_delegation(
        self,
        delegator_id: int,
        delegate_id: int,
        main_delegate_id: int,
        starts_at: datetime,
        expires_at: datetime,
        token: Optional[str] = None
    ):
        """Валидация временного делегирования"""
        
        # 1. Проверяем, что delegator является руководителем обоих
        is_manager_main = await ExternalService.is_manager_of(delegator_id, main_delegate_id, token)
        is_manager_temp = await ExternalService.is_manager_of(delegator_id, delegate_id, token)
        if not (is_manager_main and is_manager_temp):
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="Manager must be superior to both subordinates"
            )
        
        # 2. Проверяем правила для temporary делегирования
        roles = await ExternalService.get_user_roles(delegator_id, token)
        rule = await self.rule_repo.get_by_role("admin") if "admin" in roles else None
        if not rule:
            rule = await self.rule_repo.get_by_role("manager") if "manager" in roles else None
        
        if not rule or not rule.temporary_allowed:
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="Temporary delegation is not allowed for this user"
            )
    
    async def can_revoke(self, delegation_id: int, user_id: int, is_super_admin: bool = False) -> bool:
        """Проверить, может ли пользователь отозвать делегирование"""
        delegation = await self.delegation_repo.get_by_id(delegation_id)
        if not delegation:
            return False
        
        if is_super_admin or delegation.delegator_id == user_id:
            return True
        
        return False
