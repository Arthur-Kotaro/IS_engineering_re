# app/repositories/project_repo.py
from typing import Optional, List
from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy.orm import selectinload

from app.models.project import Project, ProjectStatus
from app.models.project_member import ProjectMember


class ProjectRepository:
    def __init__(self, db: AsyncSession):
        self.db = db

    async def create(
        self,
        title: str,
        description: Optional[str],
        created_by: int,
        project_type: str = "vehicle",
        typology: str = "brother",
        priority: int = 0,
        chief_engineer_id: Optional[int] = None,
        planning_engineer_id: Optional[int] = None,
        status: str = ProjectStatus.DRAFT,
    ) -> Project:
        p = Project(
            title=title,
            description=description,
            priority=priority,
            status=status,
            project_type=project_type,
            typology=typology,
            chief_engineer_id=chief_engineer_id,
            planning_engineer_id=planning_engineer_id,
            created_by=created_by,
        )
        self.db.add(p)
        await self.db.commit()
        await self.db.refresh(p)
        return p

    async def get_by_id(self, project_id: int, with_members: bool = False) -> Optional[Project]:
        q = select(Project).where(Project.project_id == project_id)
        if with_members:
            q = q.options(selectinload(Project.members))
        result = await self.db.execute(q)
        return result.scalar_one_or_none()

    async def get_all(self, skip: int = 0, limit: int = 100, status: Optional[str] = None) -> List[Project]:
        q = select(Project).order_by(Project.project_id)
        if status:
            q = q.where(Project.status == status)
        q = q.offset(skip).limit(limit)
        result = await self.db.execute(q)
        return result.scalars().all()

    async def update(self, project_id: int, **kwargs) -> Optional[Project]:
        p = await self.get_by_id(project_id)
        if not p:
            return None
        for k, v in kwargs.items():
            if hasattr(p, k) and v is not None:
                setattr(p, k, v)
        await self.db.commit()
        await self.db.refresh(p)
        return p

    async def get_member(self, project_id: int, user_id: int) -> Optional[ProjectMember]:
        result = await self.db.execute(
            select(ProjectMember).where(
                ProjectMember.project_id == project_id,
                ProjectMember.user_id == user_id,
            )
        )
        return result.scalar_one_or_none()

    async def add_member(self, project_id: int, role_code: str, user_id: int) -> Optional[ProjectMember]:
        # check if exists
        result = await self.db.execute(
            select(ProjectMember).where(
                ProjectMember.project_id == project_id,
                ProjectMember.user_id == user_id,
            )
        )
        if result.scalar_one_or_none():
            return None
        m = ProjectMember(project_id=project_id, role_code=role_code, user_id=user_id)
        self.db.add(m)
        await self.db.commit()
        await self.db.refresh(m)
        return m

    async def remove_member(self, project_id: int, user_id: int) -> bool:
        result = await self.db.execute(
            select(ProjectMember).where(
                ProjectMember.project_id == project_id,
                ProjectMember.user_id == user_id,
            )
        )
        m = result.scalar_one_or_none()
        if not m:
            return False
        await self.db.delete(m)
        await self.db.commit()
        return True
