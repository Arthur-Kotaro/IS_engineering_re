# app/services/project_service.py
import logging
from typing import Optional, List
from fastapi import HTTPException
from app.repositories.project_repo import ProjectRepository
from app.services.access_service import (
    AccessService,
    PERM_VIEW_PROJECT,
    PERM_EDIT_PROJECT,
    PERM_MANAGE_MEMBERS,
    PERM_DELETE_PROJECT,
)
from app.schemas.project import (
    ProjectCreate, ProjectUpdate, ProjectResponse,
    ProjectDetailResponse, ProjectMemberCreate, ProjectMemberResponse,
    CheckAccessResponse, ProjectRoleResponse,
)
from app.models.project import Project, ProjectStatus
from app.models.mg_creation_queue import MgCreationQueue, MgCreationQueueStatus
from app.services.user_client import get_user_name

logger = logging.getLogger(__name__)


class ProjectService:
    def __init__(self, db):
        self.db = db
        self.repo = ProjectRepository(db)
        self.access = AccessService(self.repo)

    def _to_response(self, p: Project, members_count: int = 0) -> ProjectResponse:
        return ProjectResponse(
            project_id=p.project_id,
            title=p.title,
            description=p.description,
            priority=p.priority,
            status=p.status,
            project_type=p.project_type,
            typology=p.typology,
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
            project_type=data.project_type,
            typology=data.typology,
            chief_engineer_id=data.chief_engineer_id,
            planning_engineer_id=data.planning_engineer_id,
            created_by=created_by,
            status=data.status or ProjectStatus.DRAFT,
        )
        await self.repo.add_member(p.project_id, "chief_engineer", data.chief_engineer_id)
        if data.planning_engineer_id != data.chief_engineer_id:
            await self.repo.add_member(p.project_id, "planning_engineer", data.planning_engineer_id)

        queue_item = MgCreationQueue(
            project_id=p.project_id,
            project_type=p.project_type,
            typology_name=p.typology,
            cve_id=data.chief_engineer_id,
            ipp_id=data.planning_engineer_id,
            status=MgCreationQueueStatus.PENDING,
        )
        self.db.add(queue_item)
        await self.db.commit()
        logger.info(f"Project {p.project_id} created; queued MG creation (item {queue_item.id})")

        return self._to_response(p, members_count=2 if data.planning_engineer_id != data.chief_engineer_id else 1)

    async def get_project(self, project_id: int) -> Optional[ProjectDetailResponse]:
        p = await self.repo.get_by_id(project_id, with_members=True)
        if not p:
            return None

        members = []
        for m in p.members:
            full_name = await get_user_name(m.user_id)
            members.append(ProjectMemberResponse(
                member_id=m.member_id,
                user_id=m.user_id,
                role_code=m.role_code,
                joined_at=m.joined_at,
                full_name=full_name,
            ))

        return ProjectDetailResponse(
            project_id=p.project_id,
            title=p.title,
            description=p.description,
            priority=p.priority,
            status=p.status,
            project_type=p.project_type,
            typology=p.typology,
            chief_engineer_id=p.chief_engineer_id,
            planning_engineer_id=p.planning_engineer_id,
            created_by=p.created_by,
            created_at=p.created_at,
            updated_at=p.updated_at,
            members_count=len(members),
            members=members,
        )

    async def list_projects(self, skip: int, limit: int, status=None) -> List[ProjectResponse]:
        projects = await self.repo.get_all(skip, limit, status)
        return [self._to_response(p) for p in projects]

    async def update_project(self, project_id, data: ProjectUpdate, user_id, token=None) -> Optional[ProjectResponse]:
        access = await self.access.check_access(project_id, user_id, PERM_EDIT_PROJECT, token)
        if not access["has_access"]:
            raise HTTPException(403, "No edit_project access")
        
        # exclude_unset=True позволяет отличить "поле не передано" от "поле = null"
        update_dict = data.model_dump(exclude_unset=True)
        p = await self.repo.update(project_id, **update_dict)
        if not p:
            return None
        return self._to_response(p)

    async def delete_project(self, project_id, user_id, token=None):
        access = await self.access.check_access(project_id, user_id, PERM_DELETE_PROJECT, token)
        if not access["has_access"]:
            raise HTTPException(403, "No delete_project access")
        
        deleted = await self.repo.delete(project_id)
        if not deleted:
            raise HTTPException(404, "Project not found")
        logger.info(f"Project {project_id} deleted by user {user_id}")

    async def add_member(self, project_id, data: ProjectMemberCreate, current_user_id, token=None) -> ProjectMemberResponse:
        access = await self.access.check_access(project_id, current_user_id, PERM_MANAGE_MEMBERS, token)
        if not access["has_access"]:
            raise HTTPException(403, "No manage_members access")
        m = await self.repo.add_member(project_id, data.role_code, data.user_id)
        if not m:
            raise HTTPException(400, "User already in project or role invalid")
        full_name = await get_user_name(m.user_id)
        return ProjectMemberResponse(
            member_id=m.member_id,
            user_id=m.user_id,
            role_code=m.role_code,
            joined_at=m.joined_at,
            full_name=full_name,
        )

    async def remove_member(self, project_id, user_id, current_user_id, token=None) -> bool:
        access = await self.access.check_access(project_id, current_user_id, PERM_MANAGE_MEMBERS, token)
        if not access["has_access"]:
            raise HTTPException(403, "No manage_members access")
        return await self.repo.remove_member(project_id, user_id)

    async def check_access(self, project_id, user_id, permission, token=None) -> CheckAccessResponse:
        result = await self.access.check_access(project_id, user_id, permission, token)
        p = await self.repo.get_by_id(project_id)
        return CheckAccessResponse(
            has_access=result["has_access"],
            reason=result.get("reason"),
            role=result.get("role"),
            project_status=p.status if p else None,
        )

    async def list_with_access(self, user_id, permission, token=None) -> List[ProjectResponse]:
        items = await self.access.list_projects_with_access(user_id, permission, token)
        result = []
        for item in items:
            p = await self.repo.get_by_id(item["project_id"])
            if not p:
                continue
            result.append(self._to_response(p))
        return result
