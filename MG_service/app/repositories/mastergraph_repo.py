# app/repositories/mastergraph_repo.py
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from typing import Optional, List
from uuid import UUID
from app.models.mastergraph import Mastergraph
from app.models.mg_task import MgTask
from app.models.mg_dependency import MgDependency
from app.models.task_group import TaskGroup
from app.config import load_bus_instances


class MastergraphRepository:
    def __init__(self, db: AsyncSession):
        self.db = db

    async def list_by_project_ids(self, project_ids: List[int]) -> List[Mastergraph]:
        if not project_ids:
            return []
        result = await self.db.execute(
            select(Mastergraph).where(Mastergraph.project_id.in_(project_ids))
        )
        return result.scalars().all()

    async def get_by_id(self, mg_id: UUID) -> Optional[Mastergraph]:
        result = await self.db.execute(
            select(Mastergraph).where(Mastergraph.mg_id == mg_id)
        )
        return result.scalar_one_or_none()

    async def get_by_project_id(self, project_id: int) -> Optional[Mastergraph]:
        result = await self.db.execute(
            select(Mastergraph).where(Mastergraph.project_id == project_id)
        )
        return result.scalar_one_or_none()

    async def get_tasks(self, mg_id: UUID) -> List[MgTask]:
        result = await self.db.execute(
            select(MgTask).where(MgTask.mg_id == mg_id).order_by(MgTask.sort_order)
        )
        return result.scalars().all()

    async def get_dependencies(self, mg_id: UUID) -> List[MgDependency]:
        result = await self.db.execute(
            select(MgDependency).where(MgDependency.mg_id == mg_id)
        )
        return result.scalars().all()

    async def get_groups_for_mg(self, mg_id: UUID) -> List[dict]:
        """Возвращает группы, которые есть в МГ, с bus_url (актуальный конфиг)."""
        bus_instances = load_bus_instances()
        result = await self.db.execute(
            select(TaskGroup).join(MgTask, MgTask.group_id == TaskGroup.group_id)
            .where(MgTask.mg_id == mg_id).distinct()
        )
        groups = result.scalars().all()
        return [
            {
                "group_id": g.group_id,
                "name": g.name,
                "description": g.description,
                "bus_instance_id": g.bus_instance_id,
                "bus_url": bus_instances.get(g.group_id, {}).get("url"),
            }
            for g in groups
        ]

    async def create(self, project_id: int, name: str, created_by: Optional[int]) -> Mastergraph:
        mg = Mastergraph(project_id=project_id, name=name, created_by=created_by, version=1)
        self.db.add(mg)
        await self.db.commit()
        await self.db.refresh(mg)
        return mg
