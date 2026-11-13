# app/repositories/event_repo.py
from typing import Optional, List
from uuid import UUID
from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession

from app.models.mg_event import MgEvent


class EventRepository:
    def __init__(self, db: AsyncSession):
        self.db = db

    async def log(
        self,
        mg_id: UUID,
        entity_type: str,
        entity_id: UUID,
        event_type: str,
        field: Optional[str] = None,
        old_value=None,
        new_value=None,
        reason: Optional[str] = None,
        actor_id: Optional[int] = None,
    ) -> MgEvent:
        event = MgEvent(
            mg_id=mg_id,
            entity_type=entity_type,
            entity_id=entity_id,
            event_type=event_type,
            field=field,
            old_value=old_value,
            new_value=new_value,
            reason=reason,
            actor_id=actor_id,
        )
        self.db.add(event)
        await self.db.flush()
        return event

    async def list_by_mg(self, mg_id: UUID, limit: int = 500) -> List[MgEvent]:
        result = await self.db.execute(
            select(MgEvent)
            .where(MgEvent.mg_id == mg_id)
            .order_by(MgEvent.created_at.desc())
            .limit(limit)
        )
        return result.scalars().all()

    async def list_by_entity(
        self,
        mg_id: UUID,
        entity_type: Optional[str] = None,
        entity_id: Optional[UUID] = None,
        limit: int = 200,
    ) -> List[MgEvent]:
        q = select(MgEvent).where(MgEvent.mg_id == mg_id)
        if entity_type:
            q = q.where(MgEvent.entity_type == entity_type)
        if entity_id:
            q = q.where(MgEvent.entity_id == entity_id)
        q = q.order_by(MgEvent.created_at.desc()).limit(limit)
        result = await self.db.execute(q)
        return result.scalars().all()
