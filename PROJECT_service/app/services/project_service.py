# app/services/project_service.py
from typing import Optional, List
from fastapi import HTTPException
from app.repositories.project_repo import ProjectRepository
from app.services.access_service import (
    AccessService,
    PERM_VIEW_PROJECT,
    PERM_EDIT_PROJECT,
    PERM_EDIT_MASTERGRAPHIC,
    PERM_MANAGE_MEMBERS,
    PERM_APPROVE_MEMBERS,
)
from app.schemas.project import (
    ProjectCreate, ProjectUpdate, ProjectResponse,
    ProjectDetailResponse, CheckAccessResponse,
    ProjectMemberResponse, ProjectMemberCreate,
)
from app.models.project import Project, ProjectStatus


class ProjectService:
    def __init__(self, db):
        self.repo = ProjectRepository(db)
        self.access = AccessService(self.repo)

    def _to_response(self, p: Project, members_count: int = 0) -> ProjectResponse:
        return ProjectResponse(
            project_id=p.project_id,
            title=p.title,
            description=p.description,
            priority=p.priority,
            status=p.status,
            chief_engineer_id=p.chief_engineer_id,
            planning_engineer_id=p.planning_engineer_id,
            created_by=p.created_by,
            created_at=p.created_at,
            updated_at=p.updated_at,
            members_count=members_count,
        )

    async def create_project(self, data: ProjectCreate, created_by: int) -> ProjectResponse:
        p = await self.repo.create(
            title=data.title,
            description=data.description,
            priority=data.priority,
            chief_engineer_id=data.chief_engineer_id,
            planning_engineer_id=data.planning_engineer_id,
            created_by=created_by,
            status=data.status or ProjectStatus.DRAFT,
        )
        if data.chief_engineer_id:
            await self.repo.add_member(p.project_id, data.chief_engineer_id)
        if data.planning_engineer_id and data.planning_engineer_id != data.chief_engineer_id:
            await self.repo.add_member(p.project_id, data.planning_engineer_id)
        return self._to_response(p, members_count=len(await self.repo.list_members(p.project_id)))

    async def get_project(self, project_id: int) -> Optional[ProjectDetailResponse]:
        p = await self.repo.get_by_id(project_id, with_members=True)
        if not p:
            return None
        members = [
            ProjectMemberResponse(user_id=m.user_id, joined_at=m.joined_at)
            for m in p.members
        ]
        return ProjectDetailResponse(
            project_id=p.project_id,
            title=p.title,
            description=p.description,
            priority=p.priority,
            status=p.status,
            chief_engineer_id=p.chief_engineer_id,
            planning_engineer_id=p.planning_engineer_id,
            created_by=p.created_by,
            created_at=p.created_at,
            updated_at=p.updated_at,
            members_count=len(members),
            members=[m.model_dump() for m in members],
        )

    async def list_projects(self, skip: int, limit: int, status=None) -> List[ProjectResponse]:
        projects = await self.repo.get_all(skip, limit, status)
        return [self._to_response(p) for p in projects]

    async def update_project(self, project_id: int, data: ProjectUpdate, user_id: int, token=None) -> Optional[ProjectResponse]:
        access = await self.access.check_access(project_id, user_id, PERM_EDIT_PROJECT, token)
        if not access["has_access"]:
            raise HTTPException(403, "No edit_project access")
        update_dict = data.model_dump(exclude_unset=True)
        p = await self.repo.update(project_id, **update_dict)
        if not p:
            return None
        return self._to_response(p)

    async def add_member(self, project_id: int, data: ProjectMemberCreate, current_user_id: int, token=None) -> ProjectMemberResponse:
        access = await self.access.check_access(project_id, current_user_id, PERM_MANAGE_MEMBERS, token)
        if not access["has_access"]:
            raise HTTPException(403, "No manage_members access")
        m = await self.repo.add_member(project_id, data.user_id)
        if not m:
            raise HTTPException(400, "User already in project")
        return ProjectMemberResponse(user_id=m.user_id, joined_at=m.joined_at)

    async def remove_member(self, project_id: int, user_id: int, current_user_id: int, token=None) -> bool:
        access = await self.access.check_access(project_id, current_user_id, PERM_MANAGE_MEMBERS, token)
        if not access["has_access"]:
            raise HTTPException(403, "No manage_members access")
        return await self.repo.remove_member(project_id, user_id)

    async def check_access(self, project_id: int, user_id: int, permission: str, token=None) -> CheckAccessResponse:
        result = await self.access.check_access(project_id, user_id, permission, token)
        p = await self.repo.get_by_id(project_id)
        return CheckAccessResponse(
            has_access=result["has_access"],
            reason=result.get("reason"),
            role=result.get("role"),
            project_status=p.status if p else None,
        )

    async def list_with_access(self, user_id: int, permission: str, token=None) -> List[ProjectResponse]:
        items = await self.access.list_projects_with_access(user_id, permission, token)
        result = []
        for item in items:
            p = await self.repo.get_by_id(item["project_id"])
            if not p:
                continue
            resp = self._to_response(p)
            resp.access_reason = item["reason"]
            result.append(resp)
        return result
