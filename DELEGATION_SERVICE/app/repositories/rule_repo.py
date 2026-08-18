# app/repositories/rule_repo.py
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from typing import Optional, List
from app.models.rule import DelegationRule


class RuleRepository:
    def __init__(self, db: AsyncSession):
        self.db = db
    
    async def create(self, **kwargs) -> DelegationRule:
        rule = DelegationRule(**kwargs)
        self.db.add(rule)
        await self.db.commit()
        await self.db.refresh(rule)
        return rule
    
    async def get_by_role(self, role: str) -> Optional[DelegationRule]:
        result = await self.db.execute(
            select(DelegationRule).where(DelegationRule.role == role)
        )
        return result.scalar_one_or_none()
    
    async def get_all(self) -> List[DelegationRule]:
        result = await self.db.execute(select(DelegationRule))
        return result.scalars().all()
    
    async def update(self, rule_id: int, **kwargs) -> Optional[DelegationRule]:
        rule = await self.get_by_id(rule_id)
        if not rule:
            return None
        for key, value in kwargs.items():
            if hasattr(rule, key) and value is not None:
                setattr(rule, key, value)
        await self.db.commit()
        await self.db.refresh(rule)
        return rule
    
    async def get_by_id(self, rule_id: int) -> Optional[DelegationRule]:
        result = await self.db.execute(
            select(DelegationRule).where(DelegationRule.rule_id == rule_id)
        )
        return result.scalar_one_or_none()
