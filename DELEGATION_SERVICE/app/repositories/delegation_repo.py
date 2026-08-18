# app/repositories/delegation_repo.py
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, update, delete, and_, or_, func
from sqlalchemy.orm import selectinload
from typing import Optional, List
from datetime import datetime, timezone
from app.models.delegation import Delegation, DelegationStatus, DelegationType


class DelegationRepository:
    def __init__(self, db: AsyncSession):
        self.db = db
    
    async def create(self, **kwargs) -> Delegation:
        delegation = Delegation(**kwargs)
        self.db.add(delegation)
        await self.db.commit()
        await self.db.refresh(delegation)
        return delegation
    
    async def get_by_id(self, delegation_id: int) -> Optional[Delegation]:
        result = await self.db.execute(
            select(Delegation).where(Delegation.delegation_id == delegation_id)
        )
        return result.scalar_one_or_none()
    
    async def get_active_by_user(self, user_id: int) -> List[Delegation]:
        now = datetime.now(timezone.utc)
        result = await self.db.execute(
            select(Delegation).where(
                or_(
                    Delegation.delegator_id == user_id,
                    Delegation.delegate_id == user_id
                ),
                Delegation.status == DelegationStatus.ACTIVE,
                Delegation.starts_at <= now,
                Delegation.expires_at > now
            ).order_by(Delegation.created_at.desc())
        )
        return result.scalars().all()
    
    async def get_active_delegations_for_user(self, user_id: int) -> List[Delegation]:
        now = datetime.now(timezone.utc)
        result = await self.db.execute(
            select(Delegation).where(
                Delegation.delegate_id == user_id,
                Delegation.status == DelegationStatus.ACTIVE,
                Delegation.starts_at <= now,
                Delegation.expires_at > now
            )
        )
        return result.scalars().all()
    
    async def get_delegations_by_delegator(self, delegator_id: int) -> List[Delegation]:
        result = await self.db.execute(
            select(Delegation).where(
                Delegation.delegator_id == delegator_id
            ).order_by(Delegation.created_at.desc())
        )
        return result.scalars().all()
    
    async def get_delegations_by_delegate(self, delegate_id: int) -> List[Delegation]:
        result = await self.db.execute(
            select(Delegation).where(
                Delegation.delegate_id == delegate_id
            ).order_by(Delegation.created_at.desc())
        )
        return result.scalars().all()
    
    async def update_status(self, delegation_id: int, status: DelegationStatus) -> Optional[Delegation]:
        delegation = await self.get_by_id(delegation_id)
        if not delegation:
            return None
        delegation.status = status
        await self.db.commit()
        await self.db.refresh(delegation)
        return delegation
    
    async def revoke(self, delegation_id: int, revoked_by: int, reason: Optional[str] = None) -> Optional[Delegation]:
        delegation = await self.get_by_id(delegation_id)
        if not delegation:
            return None
        delegation.status = DelegationStatus.REVOKED
        delegation.revoked_at = datetime.now(timezone.utc)
        delegation.revoked_by = revoked_by
        delegation.revoke_reason = reason
        await self.db.commit()
        await self.db.refresh(delegation)
        return delegation
    
    async def get_active_count(self, delegator_id: int) -> int:
        now = datetime.now(timezone.utc)
        result = await self.db.execute(
            select(func.count()).select_from(Delegation).where(
                Delegation.delegator_id == delegator_id,
                Delegation.status == DelegationStatus.ACTIVE,
                Delegation.starts_at <= now,
                Delegation.expires_at > now
            )
        )
        return result.scalar() or 0
    
    async def get_expired(self) -> List[Delegation]:
        now = datetime.now(timezone.utc)
        result = await self.db.execute(
            select(Delegation).where(
                Delegation.status == DelegationStatus.ACTIVE,
                Delegation.expires_at <= now
            )
        )
        return result.scalars().all()
    
    async def mark_expired(self, delegation_id: int) -> Optional[Delegation]:
        return await self.update_status(delegation_id, DelegationStatus.EXPIRED)
    
    async def get_all(self, skip: int = 0, limit: int = 100) -> List[Delegation]:
        result = await self.db.execute(
            select(Delegation)
            .order_by(Delegation.created_at.desc())
            .offset(skip)
            .limit(limit)
        )
        return result.scalars().all()
