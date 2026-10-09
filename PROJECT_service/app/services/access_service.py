# app/services/access_service.py
import httpx
import logging
from typing import Dict, List
from app.config import settings
from app.repositories.project_repo import ProjectRepository

logger = logging.getLogger(__name__)

PERM_VIEW_PROJECT = "view_project"
PERM_EDIT_MASTERGRAPHIC = "edit_mastergraphic"
PERM_MANAGE_MEMBERS = "manage_members"
PERM_APPROVE_MEMBERS = "approve_members"
PERM_EDIT_PROJECT = "edit_project"
PERM_DELETE_PROJECT = "delete_project"


class AccessService:
    def __init__(self, repo: ProjectRepository):
        self.repo = repo

    async def _get_user_roles(self, user_id: int, token: str = None) -> List[str]:
        headers = {}
        if token:
            headers["Authorization"] = f"Bearer {token}"
        try:
            async with httpx.AsyncClient(timeout=5.0) as client:
                resp = await client.get(
                    f"{settings.USER_SERVICE_URL}/api/v1/users/{user_id}",
                    headers=headers,
                )
                if resp.status_code == 200:
                    data = resp.json()
                    return data.get("role_codes", []) or data.get("roles", [])
        except Exception as e:
            logger.warning(f"AccessService get user roles failed: {e}")
        return []

    async def check_access(
        self,
        project_id: int,
        user_id: int,
        permission: str,
        token: str = None,
    ) -> Dict:
        project = await self.repo.get_by_id(project_id)
        if not project:
            return {"has_access": False, "reason": "project_not_found", "role": None}

        roles = await self._get_user_roles(user_id, token)
        if "admin" in roles:
            return {"has_access": True, "reason": "admin", "role": "admin"}

        is_ce = project.chief_engineer_id == user_id
        is_pe = project.planning_engineer_id == user_id
        member = await self.repo.get_member(project_id, user_id)
        is_member = member is not None
        member_role = member.role_code if member else None

        if permission == PERM_VIEW_PROJECT:
            if is_member or is_ce or is_pe:
                return {"has_access": True, "reason": "member", "role": member_role}

        if permission == PERM_EDIT_MASTERGRAPHIC:
            if is_ce:
                return {"has_access": True, "reason": "chief_engineer", "role": "chief_engineer"}
            if is_pe:
                return {"has_access": True, "reason": "planning_engineer", "role": "planning_engineer"}

        if permission == PERM_MANAGE_MEMBERS:
            if is_ce or is_pe:
                return {"has_access": True, "reason": "manager", "role": member_role}

        if permission == PERM_APPROVE_MEMBERS:
            if is_ce:
                return {"has_access": True, "reason": "chief_engineer", "role": "chief_engineer"}

        if permission == PERM_EDIT_PROJECT:
            if is_ce:
                return {"has_access": True, "reason": "chief_engineer", "role": "chief_engineer"}

        return {"has_access": False, "reason": "not_authorized", "role": member_role}

    async def list_projects_with_access(
        self,
        user_id: int,
        permission: str,
        token: str = None,
    ) -> List[Dict]:
        roles = await self._get_user_roles(user_id, token)
        is_admin = "admin" in roles

        all_projects = await self.repo.get_all(0, 10000)
        result = []

        for p in all_projects:
            if is_admin:
                result.append({"project_id": p.project_id, "reason": "admin", "role": "admin"})
                continue

            is_ce = p.chief_engineer_id == user_id
            is_pe = p.planning_engineer_id == user_id
            member = await self.repo.get_member(p.project_id, user_id)
            is_member = member is not None
            member_role = member.role_code if member else None

            allowed = False
            reason = None

            if permission == PERM_VIEW_PROJECT and (is_member or is_ce or is_pe):
                allowed = True
                reason = "member"
            elif permission == PERM_EDIT_MASTERGRAPHIC and (is_ce or is_pe):
                allowed = True
                reason = "chief_engineer" if is_ce else "planning_engineer"
            elif permission == PERM_MANAGE_MEMBERS and (is_ce or is_pe):
                allowed = True
                reason = "manager"
            elif permission == PERM_APPROVE_MEMBERS and is_ce:
                allowed = True
                reason = "chief_engineer"
            elif permission == PERM_EDIT_PROJECT and is_ce:
                allowed = True
                reason = "chief_engineer"

            if allowed:
                result.append({"project_id": p.project_id, "reason": reason, "role": member_role})

        return result
