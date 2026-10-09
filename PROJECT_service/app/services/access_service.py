# app/services/access_service.py
import httpx
import logging
from typing import Optional, List, Dict
from app.config import settings
from app.repositories.project_repo import ProjectRepository

logger = logging.getLogger(__name__)


PERM_VIEW_PROJECT = "view_project"
PERM_EDIT_PROJECT = "edit_project"
PERM_EDIT_MASTERGRAPHIC = "edit_mastergraphic"
PERM_MANAGE_MEMBERS = "manage_members"
PERM_DELETE_PROJECT = "delete_project"
PERM_APPROVE_MEMBERS = "approve_members"


class AccessService:
    def __init__(self, repo: ProjectRepository):
        self.repo = repo

    async def _http_get(self, url: str, token: Optional[str] = None, user_id: Optional[int] = None):
        headers = {}
        if token:
            headers["Authorization"] = f"Bearer {token}"
        if user_id is not None:
            headers["X-User-ID"] = str(user_id)
        try:
            async with httpx.AsyncClient(timeout=5.0) as client:
                resp = await client.get(url, headers=headers)
                if resp.status_code == 200:
                    return resp.json()
        except Exception as e:
            logger.warning(f"AccessService GET {url}: {e}")
        return None

    async def get_user_roles(self, user_id: int, token: Optional[str]) -> List[str]:
        data = await self._http_get(
            f"{settings.USER_SERVICE_URL}/api/v1/users/{user_id}",
            token=token,
            user_id=user_id,
        )
        if not data:
            return []
        return data.get("roles", [])

    async def is_member(self, project_id: int, user_id: int) -> bool:
        m = await self.repo.get_member(project_id, user_id)
        return m is not None

    async def check_access(
        self,
        project_id: int,
        user_id: int,
        permission: str,
        token: Optional[str] = None,
    ) -> Dict:
        project = await self.repo.get_by_id(project_id)
        if not project:
            return {"has_access": False, "reason": "project_not_found", "role": None}

        roles = await self.get_user_roles(user_id, token)
        is_admin = "admin" in roles

        if is_admin:
            return {"has_access": True, "reason": "admin", "role": "admin"}

        is_member = await self.is_member(project_id, user_id)
        is_ce = project.chief_engineer_id == user_id
        is_pe = project.planning_engineer_id == user_id

        if permission == PERM_VIEW_PROJECT:
            if is_member or is_ce or is_pe:
                return {"has_access": True, "reason": "member", "role": None}
            return {"has_access": False, "reason": "not_member", "role": None}

        if permission == PERM_EDIT_MASTERGRAPHIC:
            if is_ce:
                return {"has_access": True, "reason": "chief_engineer", "role": "chief_engineer"}
            if is_pe:
                return {"has_access": True, "reason": "planning_engineer", "role": "planning_engineer"}
            return {"has_access": False, "reason": "not_authorized", "role": None}

        if permission == PERM_MANAGE_MEMBERS:
            if is_ce or is_pe:
                return {"has_access": True, "reason": "manager", "role": None}
            return {"has_access": False, "reason": "not_authorized", "role": None}

        if permission == PERM_APPROVE_MEMBERS:
            if is_ce:
                return {"has_access": True, "reason": "chief_engineer", "role": "chief_engineer"}
            return {"has_access": False, "reason": "not_authorized", "role": None}

        if permission == PERM_EDIT_PROJECT:
            if is_ce:
                return {"has_access": True, "reason": "chief_engineer", "role": "chief_engineer"}
            return {"has_access": False, "reason": "not_authorized", "role": None}

        if permission == PERM_DELETE_PROJECT:
            return {"has_access": False, "reason": "not_implemented", "role": None}

        return {"has_access": False, "reason": "unknown_permission", "role": None}

    async def list_projects_with_access(
        self,
        user_id: int,
        permission: str,
        token: Optional[str] = None,
    ) -> List[Dict]:
        roles = await self.get_user_roles(user_id, token)
        is_admin = "admin" in roles

        all_projects = await self.repo.get_all(0, 10000)

        result = []
        for p in all_projects:
            if is_admin:
                result.append({"project_id": p.project_id, "reason": "admin", "role": "admin"})
                continue

            is_ce = p.chief_engineer_id == user_id
            is_pe = p.planning_engineer_id == user_id
            is_member = await self.is_member(p.project_id, user_id)

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
                result.append({"project_id": p.project_id, "reason": reason, "role": None})

        return result
