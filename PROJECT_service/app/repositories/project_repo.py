# app/repositories/project_repo.py
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, update, delete, and_, or_
from sqlalchemy.orm import selectinload
from typing import Optional, List
from app.models.project import Project, ProjectStatus
from app.models.project_member import ProjectMember


class ProjectRepository:
    def __init__(self, db: AsyncSession):
        self.db = db

    async def create(self, title: str, description: Optional[str], created_by: int, status: ProjectStatus = ProjectStatus.DRAFT) -> Project:
        project = Project(
            title=title,
            description=description,
            created_by=created_by,
            status=status
        )
        self.db.add(project)
        await self.db.commit()
        await self.db.refresh(project)
        return project

    async def get_by_id(self, project_id: int) -> Optional[Project]:
        result = await self.db.execute(
            select(Project)
            .options(selectinload(Project.members))
            .where(Project.project_id == project_id)
        )
        return result.unique().scalar_one_or_none()

    async def get_all(self, skip: int = 0, limit: int = 100, status: Optional[ProjectStatus] = None) -> List[Project]:
        query = select(Project).options(selectinload(Project.members))
        if status:
            query = query.where(Project.status == status)
        query = query.offset(skip).limit(limit)
        result = await self.db.execute(query)
        return result.unique().scalars().all()

    async def update(self, project_id: int, **kwargs) -> Optional[Project]:
        project = await self.get_by_id(project_id)
        if not project:
            return None
        for key, value in kwargs.items():
            if hasattr(project, key) and value is not None:
                setattr(project, key, value)
        await self.db.commit()
        await self.db.refresh(project)
        return project

    async def delete(self, project_id: int) -> bool:
        project = await self.get_by_id(project_id)
        if not project:
            return False
        await self.db.delete(project)
        await self.db.commit()
        return True

    async def add_member(self, project_id: int, user_id: int, role: str = "member") -> Optional[ProjectMember]:
        existing = await self.db.execute(
            select(ProjectMember).where(
                and_(
                    ProjectMember.project_id == project_id,
                    ProjectMember.user_id == user_id
                )
            )
        )
        if existing.scalar_one_or_none():
            return None
        member = ProjectMember(project_id=project_id, user_id=user_id, role=role)
        self.db.add(member)
        await self.db.commit()
        await self.db.refresh(member)
        return member

    async def remove_member(self, project_id: int, user_id: int) -> bool:
        result = await self.db.execute(
            delete(ProjectMember).where(
                and_(
                    ProjectMember.project_id == project_id,
                    ProjectMember.user_id == user_id
                )
            )
        )
        await self.db.commit()
        return result.rowcount > 0

    async def get_member(self, project_id: int, user_id: int) -> Optional[ProjectMember]:
        result = await self.db.execute(
            select(ProjectMember).where(
                and_(
                    ProjectMember.project_id == project_id,
                    ProjectMember.user_id == user_id
                )
            )
        )
        return result.scalar_one_or_none()

    async def get_user_projects(self, user_id: int) -> List[Project]:
        result = await self.db.execute(
            select(Project)
            .join(ProjectMember)
            .where(ProjectMember.user_id == user_id)
            .options(selectinload(Project.members))
        )
        return result.unique().scalars().all()
