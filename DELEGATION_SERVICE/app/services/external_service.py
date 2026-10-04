# app/services/external_service.py
import httpx
import logging
from typing import Optional, List, Dict, Any
from app.config import settings

logger = logging.getLogger(__name__)


class ExternalService:
    @staticmethod
    async def get_user_info(user_id: int, token: Optional[str] = None) -> Optional[Dict[str, Any]]:
        try:
            headers = {}
            if token:
                headers["Authorization"] = f"Bearer {token}"
            url = f"{settings.USER_SERVICE_URL}/api/v1/users/{user_id}"
            async with httpx.AsyncClient(timeout=5.0) as client:
                resp = await client.get(url, headers=headers)
                if resp.status_code == 200:
                    return resp.json()
                logger.warning(f"get_user_info({user_id}) -> {resp.status_code}")
                return None
        except Exception as e:
            logger.error(f"get_user_info({user_id}): {e}")
            return None

    @staticmethod
    async def get_user_roles(user_id: int, token: Optional[str] = None) -> List[str]:
        user = await ExternalService.get_user_info(user_id, token)
        if user:
            return user.get("roles", [])
        return []

    @staticmethod
    async def get_manager(user_id: int, token: Optional[str] = None) -> Optional[int]:
        try:
            headers = {}
            if token:
                headers["Authorization"] = f"Bearer {token}"
            url = f"{settings.USER_SERVICE_URL}/api/v1/users/{user_id}/manager"
            async with httpx.AsyncClient(timeout=5.0) as client:
                resp = await client.get(url, headers=headers)
                if resp.status_code == 200:
                    return resp.json().get("manager_id")
                return None
        except Exception as e:
            logger.error(f"get_manager({user_id}): {e}")
            return None

    @staticmethod
    async def get_subordinates(user_id: int, token: Optional[str] = None) -> List[int]:
        try:
            headers = {}
            if token:
                headers["Authorization"] = f"Bearer {token}"
            url = f"{settings.USER_SERVICE_URL}/api/v1/users/{user_id}/subordinates"
            async with httpx.AsyncClient(timeout=5.0) as client:
                resp = await client.get(url, headers=headers)
                if resp.status_code == 200:
                    return [u["user_id"] for u in resp.json()]
                return []
        except Exception as e:
            logger.error(f"get_subordinates({user_id}): {e}")
            return []

    @staticmethod
    async def is_manager_of(manager_id: int, subordinate_id: int, token: Optional[str] = None) -> bool:
        subs = await ExternalService.get_subordinates(manager_id, token)
        return subordinate_id in subs

    @staticmethod
    async def send_notification(
        user_id: int,
        message: str,
        title: str = "Уведомление",
        notification_type: str = "system",
        reference_id: Optional[int] = None,
        reference_type: Optional[str] = None,
        data: Optional[Dict] = None,
        token: Optional[str] = None,
        internal_key: Optional[str] = None,
        user_email: Optional[str] = None,
        user_name: Optional[str] = None,
    ):
        try:
            url = f"{settings.NOTIFICATION_SERVICE_URL}/api/v1/notifications/internal"
            headers = {}
            if token:
                headers["Authorization"] = f"Bearer {token}"
            if internal_key:
                headers["X-Internal-Key"] = internal_key
            payload = {
                "user_id": user_id,
                "user_email": user_email,
                "user_name": user_name,
                "notification_type": notification_type,
                "title": title,
                "message": message,
                "reference_id": reference_id,
                "reference_type": reference_type,
                "data": data or {},
                "send_email": False,
            }
            async with httpx.AsyncClient(timeout=5.0) as client:
                resp = await client.post(url, headers=headers, json=payload)
                if resp.status_code >= 400:
                    logger.warning(f"send_notification({user_id}) -> {resp.status_code}")
        except Exception as e:
            logger.warning(f"send_notification({user_id}): {e}")
