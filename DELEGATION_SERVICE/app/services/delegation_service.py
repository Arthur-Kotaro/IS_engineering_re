# app/services/delegation_service.py
from typing import Optional, List
from datetime import datetime, timezone
from fastapi import HTTPException, status
from app.repositories.delegation_repo import DelegationRepository
from app.repositories.history_repo import HistoryRepository
from app.services.validation_service import ValidationService
from app.services.external_service import ExternalService
from app.schemas.delegation import DelegationCreate, DelegationResponse, DelegationRevoke
from app.models.delegation import DelegationStatus, DelegationType


class DelegationService:
    def __init__(
        self,
        delegation_repo: DelegationRepository,
        history_repo: HistoryRepository,
        validation_service: ValidationService,
    ):
        self.delegation_repo = delegation_repo
        self.history_repo = history_repo
        self.validation_service = validation_service

    async def create_delegation(
        self,
        data: DelegationCreate,
        created_by: int,
        token: Optional[str] = None,
        ip_address: Optional[str] = None,
        user_agent: Optional[str] = None,
    ) -> DelegationResponse:
        if data.delegation_type == "direct":
            await self.validation_service.validate_direct(
                initiator_id=created_by,
                delegator_id=data.delegator_id,
                delegate_id=data.delegate_id,
                starts_at=data.starts_at,
                expires_at=data.expires_at,
                token=token,
            )
        elif data.delegation_type == "temporary":
            await self.validation_service.validate_temporary(
                initiator_id=created_by,
                main_delegate_id=data.main_delegate_id,
                delegate_id=data.delegate_id,
                starts_at=data.starts_at,
                expires_at=data.expires_at,
                token=token,
            )

        delegator_info = await ExternalService.get_user_info(data.delegator_id, token)
        delegate_info = await ExternalService.get_user_info(data.delegate_id, token)

        delegator_name = (delegator_info or {}).get("full_name") or (delegator_info or {}).get("user_name") or f"User {data.delegator_id}"
        delegate_name = (delegate_info or {}).get("full_name") or (delegate_info or {}).get("user_name") or f"User {data.delegate_id}"

        delegation = await self.delegation_repo.create(
            initiator_id=created_by,
            delegator_id=data.delegator_id,
            delegator_name=delegator_name,
            delegate_id=data.delegate_id,
            delegate_name=delegate_name,
            main_delegate_id=data.main_delegate_id,
            delegation_type=data.delegation_type,
            starts_at=data.starts_at,
            expires_at=data.expires_at,
            reason=data.reason,
            created_by=created_by,
        )

        await self.history_repo.create(
            delegation_id=delegation.delegation_id,
            action="CREATED",
            user_id=created_by,
            details={
                "type": data.delegation_type,
                "delegator": data.delegator_id,
                "delegate": data.delegate_id,
                "starts_at": data.starts_at.isoformat(),
                "expires_at": data.expires_at.isoformat(),
            },
            ip_address=ip_address,
            user_agent=user_agent,
        )

        return DelegationResponse.model_validate(delegation)

    async def revoke_delegation(
        self,
        delegation_id: int,
        user_id: int,
        data: DelegationRevoke,
        is_super_admin: bool = False,
        ip_address: Optional[str] = None,
        user_agent: Optional[str] = None,
    ) -> DelegationResponse:
        can_revoke = await self.validation_service.can_revoke(delegation_id, user_id, is_super_admin)
        if not can_revoke:
            raise HTTPException(status_code=403, detail="Only initiator or super admin can revoke")

        delegation = await self.delegation_repo.revoke(delegation_id, revoked_by=user_id, reason=data.reason)
        if not delegation:
            raise HTTPException(status_code=404, detail="Delegation not found")

        await self.history_repo.create(
            delegation_id=delegation.delegation_id,
            action="REVOKED",
            user_id=user_id,
            details={"reason": data.reason},
            ip_address=ip_address,
            user_agent=user_agent,
        )
        return DelegationResponse.model_validate(delegation)

    async def get_active_for_user(self, user_id: int) -> List[DelegationResponse]:
        items = await self.delegation_repo.get_active_for_delegate(user_id)
        return [DelegationResponse.model_validate(d) for d in items]

    async def get_active_as_delegator(self, user_id: int) -> List[DelegationResponse]:
        items = await self.delegation_repo.get_active_for_delegator(user_id)
        return [DelegationResponse.model_validate(d) for d in items]

    async def check_active(self, delegate_id: int, delegator_id: int) -> dict:
        d = await self.delegation_repo.find_active_pair(delegator_id, delegate_id)
        if d:
            return {
                "has_delegation": True,
                "delegation_id": d.delegation_id,
                "delegator_id": d.delegator_id,
                "delegation_type": d.delegation_type,
                "expires_at": d.expires_at,
            }
        return {"has_delegation": False}

    async def expire_expired_delegations(self) -> int:
        expired = await self.delegation_repo.get_expired()
        count = 0
        for d in expired:
            await self.delegation_repo.update_status(d.delegation_id, DelegationStatus.EXPIRED)
            await self.history_repo.create(
                delegation_id=d.delegation_id,
                action="EXPIRED",
                user_id=0,
                details={"auto_expired": True},
            )
            count += 1
        return count

    async def revoke_all_by_donor(self, donor_id: int, reason: str = "donor_inactive") -> int:
        return await self.delegation_repo.revoke_all_by_donor(donor_id, reason)
