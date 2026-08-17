# app/services/project_service.py
from sqlalchemy.ext.asyncio import AsyncSession
from typing import Optional, List
from fastapi import HTTPException, status
from app.repositories.project_repo import ProjectRepository
from app.schemas.project import ProjectCreate, ProjectUpdate, ProjectResponse, ProjectDetailResponse, CheckAccessResponse
from app.models.project import ProjectStatus, Project


class ProjectService:
    def __init__(self, db: AsyncSession):
        self.repo = ProjectRepository(db)

    async def get_projects(self, skip: int, limit: int, status: Optional[ProjectStatus], user_id: Optional[int]) -> List[ProjectResponse]:
        if user_id:
            projects = await self.repo.get_user_projects(user_id)
        else:
            projects = await self.repo.get_all(skip, limit, status)
        return [self._to_response(p) for p in projects]

    async def create_project(self, data: ProjectCreate, created_by: int) -> ProjectResponse:
        project = await self.repo.create(
            title=data.title,
            description=data.description,
            created_by=created_by,
            status=data.status or ProjectStatus.DRAFT
        )
        return self._to_response(project)

    async def get_project(self, project_id: int) -> Optional[ProjectDetailResponse]:
        project = await self.repo.get_by_id(project_id)
        if not project:
            return None
        return self._to_detail_response(project)

    async def _check_owner(self, project_id: int, user_id: int) -> bool:
        project = await self.repo.get_by_id(project_id)
        if not project:
            return False
        return project.created_by == user_id

    async def update_project(self, project_id: int, data: ProjectUpdate, current_user_id: int) -> Optional[ProjectResponse]:
        if not await self._check_owner(project_id, current_user_id):
            return None
        update_data = data.model_dump(exclude_unset=True)
        project = await self.repo.update(project_id, **update_data)
        if not project:
            return None
        return self._to_response(project)

    async def update_project_status(self, project_id: int, status: ProjectStatus, current_user_id: int) -> Optional[ProjectResponse]:
        if not await self._check_owner(project_id, current_user_id):
            return None
        project = await self.repo.update(project_id, status=status)
        if not project:
            return None
        return self._to_response(project)

    async def delete_project(self, project_id: int, current_user_id: int) -> bool:
        if not await self._check_owner(project_id, current_user_id):
            return False
        return await self.repo.delete(project_id)

    async def add_member(self, project_id: int, user_id: int, role: str, current_user_id: int):
        if not await self._check_owner(project_id, current_user_id):
            return None
        return await self.repo.add_member(project_id, user_id, role)

    async def remove_member(self, project_id: int, user_id: int, current_user_id: int) -> bool:
        if not await self._check_owner(project_id, current_user_id):
            return False
        return await self.repo.remove_member(project_id, user_id)

    async def check_access(self, project_id: int, user_id: int) -> CheckAccessResponse:
        project = await self.repo.get_by_id(project_id)
        if not project:
            return CheckAccessResponse(has_access=False)
        
        member = await self.repo.get_member(project_id, user_id)
        if not member:
            return CheckAccessResponse(has_access=False, project_status=project.status)
        
        return CheckAccessResponse(
            has_access=True,
            role=member.role,
            project_status=project.status
        )

    def _to_response(self, project: Project) -> ProjectResponse:
        return ProjectResponse(
            project_id=project.project_id,
            title=project.title,
            description=project.description,
            status=project.status,
            created_by=project.created_by,
            created_at=project.created_at,
            updated_at=project.updated_at,
            members_count=0
        )

    def _to_detail_response(self, project: Project) -> ProjectDetailResponse:
        return ProjectDetailResponse(
            project_id=project.project_id,
            title=project.title,
            description=project.description,
            status=project.status,
            created_by=project.created_by,
            created_at=project.created_at,
            updated_at=project.updated_at,
            members_count=0,
            members=[]
        )
