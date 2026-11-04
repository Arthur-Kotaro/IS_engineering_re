# app/services/access_client.py
import aiohttp
import logging
from typing import Optional
from app.config import settings

logger = logging.getLogger(__name__)


async def check_project_access(
    project_id: int,
    user_id: int,
    permission: str,
    token: Optional[str] = None,
) -> dict:
    """Запрашивает project_service: может ли user X делать Y над project Z."""
    headers = {}
    if token:
        headers["Authorization"] = f"Bearer {token}"
    url = f"{settings.PROJECT_SERVICE_URL}/api/v1/projects/{project_id}/check-access"
    params = {"user_id": user_id, "permission": permission}
    try:
        async with aiohttp.ClientSession(timeout=aiohttp.ClientTimeout(total=5.0)) as session:
            async with session.get(url, params=params, headers=headers) as resp:
                if resp.status == 200:
                    return await resp.json()
                logger.warning(f"check_access {project_id}: HTTP {resp.status}")
    except Exception as e:
        logger.warning(f"check_access {project_id}: {e}")
    return {"has_access": False, "reason": "service_unavailable", "role": None}


async def list_accessible_projects(
    user_id: int,
    permission: str,
    token: Optional[str] = None,
) -> list:
    """Список проектов, где у пользователя есть permission."""
    headers = {"X-User-ID": str(user_id)}
    if token:
        headers["Authorization"] = f"Bearer {token}"
    url = f"{settings.PROJECT_SERVICE_URL}/api/v1/projects/list-with-access"
    params = {"permission": permission}
    try:
        async with aiohttp.ClientSession(timeout=aiohttp.ClientTimeout(total=5.0)) as session:
            async with session.get(url, params=params, headers=headers) as resp:
                if resp.status == 200:
                    return await resp.json()
                logger.warning(f"list_with_access: HTTP {resp.status}")
    except Exception as e:
        logger.warning(f"list_with_access: {e}")
    return []
