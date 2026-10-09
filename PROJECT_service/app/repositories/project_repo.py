# app/repositories/project_repo.py
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, and_, delete
from sqlalchemy.orm import selectinload
from typing import Optional, List
from app.models.project import Project, ProjectStatus
from app.models.project_member import ProjectMember
from app.models.project_role import ProjectRole


class ProjectRepository:
    def __init__(self, db: AsyncSession):
        self.db = db

    async def create(self, title, description, created_by, priority=0,
                     chief_engineer_id=None, planning_engineer_id=None,
                     status=ProjectStatus.DRAFT) -> Project:
        p = Project(
            title=title,
            description=description,
            created_by=created_by,
            priority=priority,
            chief_engineer_id=chief_engineer_id,
            planning_engineer_id=planning_engineer_id,
            status=status,
        )
        self.db.add(p)
        await self.db.commit()
        await self.db.refresh(p)
        return p

    async def get_by_id(self, project_id: int, with_members: bool = False) -> Optional[Project]:
        stmt = select(Project).where(Project.project_id == project_id)
        if with_members:
            stmt = stmt.options(selectinload(Project.members))
        result = await self.db.execute(stmt)
        return result.scalar_one_or_none()

    async def get_all(self, skip=0, limit=100, status=None) -> List[Project]:
        stmt = select(Project)
        if status:
            stmt = stmt.where(Project.status == status)
        stmt = stmt.order_by(Project.priority.desc(), Project.created_at.desc()).offset(skip).limit(limit)
        result = await self.db.execute(stmt)
        return result.scalars().all()

    async def update(self, project_id, **kwargs) -> Optional[Project]:
        p = await self.get_by_id(project_id)
        if not p:
            return None
        for k, v in kwargs.items():
            if hasattr(p, k) and v is not None:
                setattr(p, k, v)
        await self.db.commit()
        await self.db.refresh(p)
        return p

    async def delete(self, project_id) -> bool:
        p = await self.get_by_id(project_id)
        if not p:
            return False
        await self.db.delete(p)
        await self.db.commit()
        return True

    async def get_member(self, project_id: int, user_id: int) -> Optional[ProjectMember]:
        result = await self.db.execute(
            select(ProjectMember).where(
                and_(ProjectMember.project_id == project_id, ProjectMember.user_id == user_id)
            )
        )
        return result.scalar_one_or_none()

    async def list_members(self, project_id: int) -> List[ProjectMember]:
        result = await self.db.execute(
            select(ProjectMember).where(ProjectMember.project_id == project_id)
        )
        return result.scalars().all()

    async def add_member(self, project_id, role_code, user_id) -> Optional[ProjectMember]:
        # Проверяем, что роль существует
        role = await self.db.execute(
            select(ProjectRole).where(ProjectRole.role_code == role_code)
        )
        if not role.scalar_one_or_none():
            return None
        # Проверяем дубликат
        existing = await self.get_member(project_id, user_id)
        if existing:
            return None
        m = ProjectMember(project_id=project_id, role_code=role_code, user_id=user_id)
        self.db.add(m)
        await self.db.commit()
        await self.db.refresh(m)
        return m

    async def remove_member(self, project_id, user_id) -> bool:
        result = await self.db.execute(
            delete(ProjectMember).where(
                and_(ProjectMember.project_id == project_id, ProjectMember.user_id == user_id)
            )
        )
        await self.db.commit()
        return result.rowcount > 0

    async def list_roles(self) -> List[ProjectRole]:
        result = await self.db.execute(select(ProjectRole).order_by(ProjectRole.role_code))
        return result.scalars().all()
