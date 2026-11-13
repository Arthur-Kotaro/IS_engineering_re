# app/repositories/milestone_repo.py
from datetime import date, timedelta
from typing import List, Optional
from uuid import UUID
from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession

from app.models.mg_milestone import MgMilestone, MgMilestoneStatus


class MilestoneRepository:
    def __init__(self, db: AsyncSession):
        self.db = db

    async def list_by_mg(self, mg_id: UUID) -> List[MgMilestone]:
        result = await self.db.execute(
            select(MgMilestone)
            .where(MgMilestone.mg_id == mg_id, MgMilestone.is_deleted == False)
            .order_by(MgMilestone.sort_order)
        )
        return result.scalars().all()

    async def get_by_id(self, milestone_id: UUID) -> Optional[MgMilestone]:
        result = await self.db.execute(
            select(MgMilestone).where(
                MgMilestone.milestone_id == milestone_id,
                MgMilestone.is_deleted == False,
            )
        )
        return result.scalar_one_or_none()

    async def create_many(self, mg_id: UUID, milestones: List[dict], project_start: date) -> List[MgMilestone]:
        """
        Создаёт вехи. planned_date = project_start + week_offset * 7.
        Если попадает на выходные — сдвиг на следующий понедельник.
        """
        created = []
        for m in milestones:
            planned = project_start + timedelta(weeks=m["week_offset"])
            # сдвиг на понедельник, если выходной
            while planned.weekday() >= 5:
                planned += timedelta(days=1)
            ms = MgMilestone(
                mg_id=mg_id,
                abbreviation=m["abbreviation"],
                full_name=m["full_name"],
                tooltip=m.get("tooltip"),
                week_offset=m["week_offset"],
                planned_date=planned,
                status=MgMilestoneStatus.PLANNED,
                sort_order=m["sort_order"],
                version=1,
            )
            self.db.add(ms)
            created.append(ms)
        await self.db.flush()
        return created

    async def first_milestone(self, mg_id: UUID) -> Optional[MgMilestone]:
        result = await self.db.execute(
            select(MgMilestone)
            .where(MgMilestone.mg_id == mg_id, MgMilestone.is_deleted == False)
            .order_by(MgMilestone.sort_order)
            .limit(1)
        )
        return result.scalar_one_or_none()
