# app/repositories/history_repo.py
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from typing import List
from app.models.history import DelegationHistory


class HistoryRepository:
    def __init__(self, db: AsyncSession):
        self.db = db

    async def create(self, **kwargs) -> DelegationHistory:
        h = DelegationHistory(**kwargs)
        self.db.add(h)
        await self.db.commit()
        await self.db.refresh(h)
        return h

    async def get_by_delegation(self, delegation_id: int, limit: int = 50) -> List[DelegationHistory]:
        result = await self.db.execute(
            select(DelegationHistory)
            .where(DelegationHistory.delegation_id == delegation_id)
            .order_by(DelegationHistory.timestamp.desc())
            .limit(limit)
        )
        return result.scalars().all()

    async def get_by_user(self, user_id: int, limit: int = 50) -> List[DelegationHistory]:
        result = await self.db.execute(
            select(DelegationHistory)
            .where(DelegationHistory.user_id == user_id)
            .order_by(DelegationHistory.timestamp.desc())
            .limit(limit)
        )
        return result.scalars().all()
