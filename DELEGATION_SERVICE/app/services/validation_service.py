# app/services/validation_service.py
from fastapi import HTTPException, status
from datetime import datetime, timezone
from typing import Optional
from app.services.external_service import ExternalService
from app.repositories.delegation_repo import DelegationRepository
from app.repositories.rule_repo import RuleRepository


class ValidationService:
    def __init__(self, delegation_repo: DelegationRepository, rule_repo: RuleRepository):
        self.delegation_repo = delegation_repo
        self.rule_repo = rule_repo

    async def _get_rule_for(self, user_id: int, token: Optional[str]):
        roles = await ExternalService.get_user_roles(user_id, token)
        return await self.rule_repo.find_best_rule(roles)

    async def _check_duration(self, starts_at: datetime, expires_at: datetime, max_days: int):
        duration_days = (expires_at - starts_at).days
        if duration_days > max_days:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail=f"Delegation duration exceeds maximum ({max_days} days)",
            )

    async def validate_direct(
        self,
        initiator_id: int,
        delegator_id: int,
        delegate_id: int,
        starts_at: datetime,
        expires_at: datetime,
        token: Optional[str] = None,
    ):
        if delegator_id != initiator_id:
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="Only self-delegation allowed for direct type",
            )

        is_manager = await ExternalService.is_manager_of(delegator_id, delegate_id, token)
        if not is_manager:
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="delegate must be a direct subordinate",
            )

        rule = await self._get_rule_for(delegator_id, token)
        if not rule or not rule.can_delegate or not rule.direct_allowed:
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="Direct delegation is not allowed for this user",
            )

        active_count = await self.delegation_repo.get_active_count(delegator_id)
        if active_count >= rule.max_delegations:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail=f"Maximum delegations ({rule.max_delegations}) reached",
            )

        await self._check_duration(starts_at, expires_at, rule.max_duration_days)

        existing = await self.delegation_repo.find_active_pair(delegator_id, delegate_id)
        if existing:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="Delegation already exists for this pair",
            )

    async def validate_temporary(
        self,
        initiator_id: int,
        main_delegate_id: int,
        delegate_id: int,
        starts_at: datetime,
        expires_at: datetime,
        token: Optional[str] = None,
    ):
        is_manager_main = await ExternalService.is_manager_of(initiator_id, main_delegate_id, token)
        is_manager_temp = await ExternalService.is_manager_of(initiator_id, delegate_id, token)
        if not (is_manager_main and is_manager_temp):
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="Both must be direct subordinates of the initiator",
            )

        rule = await self._get_rule_for(initiator_id, token)
        if not rule or not rule.can_delegate or not rule.temporary_allowed:
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="Temporary delegation is not allowed for this user",
            )

        active_count = await self.delegation_repo.get_active_count(initiator_id)
        if active_count >= rule.max_delegations:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail=f"Maximum delegations ({rule.max_delegations}) reached",
            )

        await self._check_duration(starts_at, expires_at, rule.max_duration_days)

    async def can_revoke(self, delegation_id: int, user_id: int, is_super_admin: bool = False) -> bool:
        d = await self.delegation_repo.get_by_id(delegation_id)
        if not d:
            return False
        if is_super_admin:
            return True
        return d.initiator_id == user_id
