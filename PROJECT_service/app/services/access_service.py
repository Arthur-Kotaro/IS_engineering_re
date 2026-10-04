# app/services/access_service.py
import httpx
from typing import Optional, List, Dict, Set
from app.config import settings
from app.repositories.project_repo import ProjectRepository
from app.models.project_member import ProjectMember, ProjectRole


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
        except Exception:
            return None
        return None

    async def get_subordinates(self, user_id: int, token: Optional[str]) -> List[int]:
        data = await self._http_get(
            f"{settings.USER_SERVICE_URL}/api/v1/users/{user_id}/subordinates",
            token=token,
            user_id=user_id,
        )
        if not data:
            return []
        return [u["user_id"] for u in data]

    async def get_active_delegations(self, delegate_id: int) -> List[Dict]:
        data = await self._http_get(
            f"{settings.DELEGATION_SERVICE_URL}/api/v1/delegations/active/{delegate_id}"
        )
        if not data:
            return []
        return data

    async def check_access(
        self,
        project_id: int,
        user_id: int,
        permission: str,
        token: Optional[str] = None,
    ) -> Dict:
        # 1. Direct
        m = await self.repo.get_member(project_id, user_id)
        if m and permission in ProjectRole.permissions_for(m.role):
            return {
                "has_access": True,
                "reason": "direct",
                "role": m.role,
                "via_user_id": None,
            }

        # 2. Subordinate (1 уровень)
        subs = await self.get_subordinates(user_id, token)
        if subs:
            for sub_id in subs:
                sm = await self.repo.get_member(project_id, sub_id)
                if sm and permission in ProjectRole.permissions_for(sm.role):
                    return {
                        "has_access": True,
                        "reason": "subordinate_access",
                        "role": sm.role,
                        "via_user_id": sub_id,
                    }

        # 3. Delegation
        delegations = await self.get_active_delegations(user_id)
        for d in delegations:
            delegator_id = d.get("delegator_id")
            if not delegator_id:
                continue
            dm = await self.repo.get_member(project_id, delegator_id)
            if dm and permission in ProjectRole.permissions_for(dm.role):
                return {
                    "has_access": True,
                    "reason": "delegation",
                    "role": dm.role,
                    "via_user_id": delegator_id,
                    "delegation_id": d.get("delegation_id"),
                }

        return {"has_access": False, "reason": "none", "role": None, "via_user_id": None}

    async def list_projects_with_access(
        self,
        user_id: int,
        permission: str,
        token: Optional[str] = None,
    ) -> List[Dict]:
        result: Dict[int, Dict] = {}

        # 1. Direct
        direct_memberships = await self.repo.list_memberships_by_user_ids([user_id])
        for m in direct_memberships:
            if permission in ProjectRole.permissions_for(m.role):
                result[m.project_id] = {
                    "project_id": m.project_id,
                    "access_via": "direct",
                    "role": m.role,
                    "via_user_id": None,
                }

        # 2. Subordinate
        subs = await self.get_subordinates(user_id, token)
        if subs:
            sub_memberships = await self.repo.list_memberships_by_user_ids(subs)
            for m in sub_memberships:
                if m.project_id in result:
                    continue
                if permission in ProjectRole.permissions_for(m.role):
                    result[m.project_id] = {
                        "project_id": m.project_id,
                        "access_via": "subordinate_access",
                        "role": m.role,
                        "via_user_id": m.user_id,
                    }

        # 3. Delegation
        delegations = await self.get_active_delegations(user_id)
        delegator_ids = [d.get("delegator_id") for d in delegations if d.get("delegator_id")]
        if delegator_ids:
            del_memberships = await self.repo.list_memberships_by_user_ids(delegator_ids)
            for m in del_memberships:
                if m.project_id in result:
                    continue
                if permission in ProjectRole.permissions_for(m.role):
                    result[m.project_id] = {
                        "project_id": m.project_id,
                        "access_via": "delegation",
                        "role": m.role,
                        "via_user_id": m.user_id,
                    }

        return list(result.values())
