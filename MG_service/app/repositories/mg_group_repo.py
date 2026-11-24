# app/repositories/mg_group_repo.py
import logging
from typing import List, Optional
from uuid import UUID
from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession

from app.models.mg_group import MgGroup
from app.models.task_group import TaskGroup
from app.config import load_bus_instances

logger = logging.getLogger(__name__)


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
            # Диагностика: почему не нашли
            any_result = await self.db.execute(
                select(MgGroup).where(
                    MgGroup.mg_id == mg_id,
                    MgGroup.group_id == group_id,
                )
            )
            existing = any_result.scalar_one_or_none()
            if existing:
                logger.warning(
                    f"remove: MgGroup exists but is_deleted={existing.is_deleted}, "
                    f"mg={mg_id}, group={group_id!r}"
                )
            else:
                logger.warning(
                    f"remove: MgGroup not found at all, mg={mg_id}, group={group_id!r}"
                )
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

    async def get_sort_order(self, mg_id: UUID, group_id: str) -> Optional[int]:
        """Возвращает sort_order активной группы или None, если группы нет."""
        result = await self.db.execute(
            select(MgGroup.sort_order).where(
                MgGroup.mg_id == mg_id,
                MgGroup.group_id == group_id,
                MgGroup.is_deleted == False,
            )
        )
        return result.scalar_one_or_none()

    async def shift_sort_orders_after(self, mg_id: UUID, after_sort_order: int, delta: int) -> None:
        """Сдвигает sort_order всех активных групп со значением > after_sort_order на delta."""
        from sqlalchemy import update
        await self.db.execute(
            update(MgGroup)
            .where(
                MgGroup.mg_id == mg_id,
                MgGroup.sort_order > after_sort_order,
                MgGroup.is_deleted == False,
            )
            .values(sort_order=MgGroup.sort_order + delta)
        )
        await self.db.flush()
