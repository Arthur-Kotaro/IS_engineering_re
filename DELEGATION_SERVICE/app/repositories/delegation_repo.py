# app/repositories/delegation_repo.py
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, func, or_, and_, update
from typing import Optional, List
from datetime import datetime, timezone
from app.models.delegation import Delegation, DelegationStatus, DelegationType


class DelegationRepository:
    def __init__(self, db: AsyncSession):
        self.db = db

    async def create(self, **kwargs) -> Delegation:
        d = Delegation(**kwargs)
        self.db.add(d)
        await self.db.commit()
        await self.db.refresh(d)
        return d

    async def get_by_id(self, delegation_id: int) -> Optional[Delegation]:
        result = await self.db.execute(
            select(Delegation).where(Delegation.delegation_id == delegation_id)
        )
        return result.scalar_one_or_none()

    async def get_active_for_delegate(self, delegate_id: int) -> List[Delegation]:
        now = datetime.now(timezone.utc)
        result = await self.db.execute(
            select(Delegation).where(
                Delegation.delegate_id == delegate_id,
                Delegation.status == DelegationStatus.ACTIVE,
                Delegation.starts_at <= now,
                Delegation.expires_at > now,
            )
        )
        return result.scalars().all()

    async def get_active_for_delegator(self, delegator_id: int) -> List[Delegation]:
        now = datetime.now(timezone.utc)
        result = await self.db.execute(
            select(Delegation).where(
                Delegation.delegator_id == delegator_id,
                Delegation.status == DelegationStatus.ACTIVE,
                Delegation.starts_at <= now,
                Delegation.expires_at > now,
            )
        )
        return result.scalars().all()

    async def find_active_pair(self, delegator_id: int, delegate_id: int) -> Optional[Delegation]:
        now = datetime.now(timezone.utc)
        result = await self.db.execute(
            select(Delegation).where(
                Delegation.delegator_id == delegator_id,
                Delegation.delegate_id == delegate_id,
                Delegation.status == DelegationStatus.ACTIVE,
                Delegation.starts_at <= now,
                Delegation.expires_at > now,
            )
        )
        return result.scalars().first()

    async def get_active_count(self, initiator_id: int) -> int:
        now = datetime.now(timezone.utc)
        result = await self.db.execute(
            select(func.count()).select_from(Delegation).where(
                Delegation.initiator_id == initiator_id,
                Delegation.status == DelegationStatus.ACTIVE,
                Delegation.starts_at <= now,
                Delegation.expires_at > now,
            )
        )
        return result.scalar() or 0

    async def get_expired(self) -> List[Delegation]:
        now = datetime.now(timezone.utc)
        result = await self.db.execute(
            select(Delegation).where(
                Delegation.status == DelegationStatus.ACTIVE,
                Delegation.expires_at <= now,
            )
        )
        return result.scalars().all()

    async def get_all(self, skip: int = 0, limit: int = 100) -> List[Delegation]:
        result = await self.db.execute(
            select(Delegation).order_by(Delegation.created_at.desc()).offset(skip).limit(limit)
        )
        return result.scalars().all()

    async def update_status(self, delegation_id: int, status: str) -> Optional[Delegation]:
        d = await self.get_by_id(delegation_id)
        if not d:
            return None
        d.status = status
        await self.db.commit()
        await self.db.refresh(d)
        return d

    async def revoke(self, delegation_id: int, revoked_by: int, reason: Optional[str]) -> Optional[Delegation]:
        d = await self.get_by_id(delegation_id)
        if not d:
            return None
        d.status = DelegationStatus.REVOKED
        d.revoked_at = datetime.now(timezone.utc)
        d.revoked_by = revoked_by
        d.revoke_reason = reason
        await self.db.commit()
        await self.db.refresh(d)
        return d

    async def revoke_all_by_donor(self, donor_id: int, reason: str = "donor_inactive") -> int:
        now = datetime.now(timezone.utc)
        result = await self.db.execute(
            update(Delegation)
            .where(
                Delegation.delegator_id == donor_id,
                Delegation.status == DelegationStatus.ACTIVE,
            )
            .values(status=DelegationStatus.REVOKED, revoked_at=now, revoke_reason=reason)
        )
        await self.db.commit()
        return result.rowcount
