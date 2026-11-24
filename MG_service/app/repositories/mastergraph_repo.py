# app/repositories/mastergraph_repo.py
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, func
from typing import Optional, List, Tuple
from uuid import UUID
from app.models.mastergraph import Mastergraph
from app.models.mg_task import MgTask
from app.models.mg_dependency import MgDependency
from app.models.mg_group import MgGroup
from app.models.task_group import TaskGroup
from app.config import load_bus_instances


class MastergraphRepository:
    def __init__(self, db: AsyncSession):
        self.db = db

    async def list_by_project_ids(self, project_ids: List[int]) -> List[Mastergraph]:
        if not project_ids:
            return []
        result = await self.db.execute(
            select(Mastergraph).where(
                Mastergraph.project_id.in_(project_ids),
                Mastergraph.is_deleted == False,
            )
        )
        return result.scalars().all()

    async def get_by_id(self, mg_id: UUID) -> Optional[Mastergraph]:
        result = await self.db.execute(
            select(Mastergraph).where(
                Mastergraph.mg_id == mg_id,
                Mastergraph.is_deleted == False,
            )
        )
        return result.scalar_one_or_none()

    async def get_by_project_id(self, project_id: int) -> Optional[Mastergraph]:
        result = await self.db.execute(
            select(Mastergraph).where(
                Mastergraph.project_id == project_id,
                Mastergraph.is_deleted == False,
            )
        )
        return result.scalar_one_or_none()

    async def get_tasks(self, mg_id: UUID) -> List[MgTask]:
        result = await self.db.execute(
            select(MgTask).where(
                MgTask.mg_id == mg_id,
                MgTask.is_deleted == False,
            ).order_by(MgTask.sort_order)
        )
        return result.scalars().all()

    async def get_dependencies(self, mg_id: UUID) -> List[MgDependency]:
        result = await self.db.execute(
            select(MgDependency).where(
                MgDependency.mg_id == mg_id,
                MgDependency.is_deleted == False,
            )
        )
        return result.scalars().all()

    async def get_dependency(self, mg_id: UUID, dep_id: UUID) -> Optional[MgDependency]:
        result = await self.db.execute(
            select(MgDependency).where(
                MgDependency.mg_id == mg_id,
                MgDependency.dep_id == dep_id,
                MgDependency.is_deleted == False,
            )
        )
        return result.scalar_one_or_none()

    async def get_groups_for_mg(self, mg_id: UUID) -> List[dict]:
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
                "bus_url": bus_instances.get(g.group_id, bus_instances.get("other_bus", {})).get("url"),
            }
            for g, tg in rows
        ]

    async def get_group(self, mg_id: UUID, group_id: str) -> Optional[MgGroup]:
        result = await self.db.execute(
            select(MgGroup).where(
                MgGroup.mg_id == mg_id,
                MgGroup.group_id == group_id,
                MgGroup.is_deleted == False,
            )
        )
        return result.scalar_one_or_none()

    async def next_sort_order(self, mg_id: UUID) -> int:
        """Возвращает max(sort_order) + 1 для МГ (для 'снизу')."""
        result = await self.db.execute(
            select(func.coalesce(func.max(MgGroup.sort_order), -1)).where(
                MgGroup.mg_id == mg_id,
                MgGroup.is_deleted == False,
            )
        )
        return (result.scalar() or -1) + 1

    async def min_sort_order(self, mg_id: UUID) -> int:
        """Возвращает min(sort_order) - 1 для МГ (для 'сверху')."""
        result = await self.db.execute(
            select(func.coalesce(func.min(MgGroup.sort_order), 0)).where(
                MgGroup.mg_id == mg_id,
                MgGroup.is_deleted == False,
            )
        )
        return (result.scalar() or 0) - 1

    async def is_pure(self, mg_id: UUID) -> bool:
        groups_count = await self.db.execute(
            select(func.count(MgGroup.mg_group_id)).where(
                MgGroup.mg_id == mg_id,
                MgGroup.is_deleted == False,
            )
        )
        tasks_count = await self.db.execute(
            select(func.count(MgTask.task_id)).where(
                MgTask.mg_id == mg_id,
                MgTask.is_deleted == False,
            )
        )
        return (groups_count.scalar() or 0) == 0 and (tasks_count.scalar() or 0) == 0

    async def task_exists(self, mg_id: UUID, group_id: str, title: str) -> bool:
        result = await self.db.execute(
            select(func.count(MgTask.task_id)).where(
                MgTask.mg_id == mg_id,
                MgTask.group_id == group_id,
                MgTask.title == title,
                MgTask.is_deleted == False,
            )
        )
        return (result.scalar() or 0) > 0

    async def get_conflicts(self, mg_id: UUID, pairs: List[Tuple[str, str]]) -> List[Tuple[str, str]]:
        if not pairs:
            return []
        titles = [t for _, t in pairs]
        result = await self.db.execute(
            select(MgTask.group_id, MgTask.title).where(
                MgTask.mg_id == mg_id,
                MgTask.is_deleted == False,
                MgTask.title.in_(titles),
            )
        )
        existing = set((g, t) for g, t in result.all())
        return [(g, t) for (g, t) in pairs if (g, t) in existing]

    async def get_date_history(self, mg_id: UUID) -> dict:
        from app.models.mg_event import MgEvent
        result = await self.db.execute(
            select(MgEvent)
            .where(
                MgEvent.mg_id == mg_id,
                MgEvent.entity_type == "task",
                MgEvent.field == "dates",
            )
            .order_by(MgEvent.created_at.asc())
        )
        events = result.scalars().all()
        history = {}
        for e in events:
            tid = str(e.entity_id)
            old = e.old_value or {}
            new = e.new_value or {}
            history.setdefault(tid, []).append({
                "old_start": old.get("start_date"),
                "old_end": old.get("end_date"),
                "new_start": new.get("start_date"),
                "new_end": new.get("end_date"),
                "reason": e.reason,
                "created_at": e.created_at.isoformat() if e.created_at else None,
                "actor_id": e.actor_id,
            })
        return history

    async def create(self, project_id: int, name: str, created_by: Optional[int]) -> Mastergraph:
        mg = Mastergraph(project_id=project_id, name=name, created_by=created_by, version=1)
        self.db.add(mg)
        await self.db.commit()
        await self.db.refresh(mg)
        return mg
