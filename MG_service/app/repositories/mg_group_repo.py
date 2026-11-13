# app/repositories/mg_group_repo.py
from typing import List, Optional
from uuid import UUID
from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession

from app.models.mg_group import MgGroup
from app.models.task_group import TaskGroup
from app.config import load_bus_instances


class MgGroupRepository:
    def __init__(self, db: AsyncSession):
        self.db = db

    async def list_by_mg(self, mg_id: UUID) -> List[dict]:
        bus_instances = load_bus_instances()
        result = await self.db.execute(
            select(MgGroup, TaskGroup)
            .join(TaskGroup, TaskGroup.group_id == MgGroup.group_id)
            .where(
                MgGroup.mg_id == mg_id,
                MgGroup.is_deleted == False,
                TaskGroup.is_deleted == False,
            )
            .order_by(MgGroup.sort_order, MgGroup.group_id)
        )
        rows = result.all()
        return [
            {
                "group_id": g.group_id,
                "name": tg.name,
                "description": tg.description,
                "bus_instance_id": tg.bus_instance_id,
                "bus_url": bus_instances.get(g.group_id, {}).get("url"),
                "sort_order": g.sort_order,
            }
            for g, tg in rows
        ]

    async def add(self, mg_id: UUID, group_id: str, sort_order: int = 0) -> Optional[MgGroup]:
        tg_res = await self.db.execute(
            select(TaskGroup).where(TaskGroup.group_id == group_id, TaskGroup.is_deleted == False)
        )
        if not tg_res.scalar_one_or_none():
            return None

        existing = await self.db.execute(
            select(MgGroup).where(
                MgGroup.mg_id == mg_id,
                MgGroup.group_id == group_id,
                MgGroup.is_deleted == False,
            )
        )
        if existing.scalar_one_or_none():
            return None

        mg = MgGroup(mg_id=mg_id, group_id=group_id, sort_order=sort_order)
        self.db.add(mg)
        await self.db.flush()
        return mg

    async def remove(self, mg_id: UUID, group_id: str, deleted_by: int) -> bool:
        from sqlalchemy.sql import func
        result = await self.db.execute(
            select(MgGroup).where(
                MgGroup.mg_id == mg_id,
                MgGroup.group_id == group_id,
                MgGroup.is_deleted == False,
            )
        )
        mg = result.scalar_one_or_none()
        if not mg:
            return False
        mg.is_deleted = True
        mg.deleted_at = func.now()
        mg.deleted_by = deleted_by
        await self.db.flush()
        return True

    async def count_active(self, mg_id: UUID) -> int:
        from sqlalchemy import func
        result = await self.db.execute(
            select(func.count(MgGroup.mg_group_id)).where(
                MgGroup.mg_id == mg_id,
                MgGroup.is_deleted == False,
            )
        )
        return result.scalar() or 0

    async def get_task_group(self, group_id: str) -> Optional[TaskGroup]:
        result = await self.db.execute(
            select(TaskGroup).where(
                TaskGroup.group_id == group_id,
                TaskGroup.is_deleted == False,
            )
        )
        return result.scalar_one_or_none()
