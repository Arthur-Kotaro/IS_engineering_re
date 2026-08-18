# app/services/external_service.py
import httpx
import logging
from typing import Optional, List, Dict, Any
from app.config import settings

logger = logging.getLogger(__name__)


class ExternalService:
    @staticmethod
    async def get_user_info(user_id: int, token: Optional[str] = None) -> Optional[Dict[str, Any]]:
        """Получить информацию о пользователе из User Service"""
        try:
            headers = {}
            if token:
                headers["Authorization"] = f"Bearer {token}"
            
            async with httpx.AsyncClient(timeout=5.0) as client:
                resp = await client.get(
                    f"{settings.USER_SERVICE_URL}/api/v1/admin/users/{user_id}",
                    headers=headers
                )
                if resp.status_code == 200:
                    return resp.json()
                logger.warning(f"Failed to get user info for {user_id}: {resp.status_code}")
                return None
        except Exception as e:
            logger.error(f"Error calling User Service: {e}")
            return None
    
    @staticmethod
    async def get_user_roles(user_id: int, token: Optional[str] = None) -> List[str]:
        """Получить роли пользователя"""
        user = await ExternalService.get_user_info(user_id, token)
        if user:
            return user.get("roles", [])
        return []
    
    @staticmethod
    async def get_manager(user_id: int, token: Optional[str] = None) -> Optional[int]:
        """Получить ID руководителя пользователя"""
        try:
            headers = {}
            if token:
                headers["Authorization"] = f"Bearer {token}"
            
            async with httpx.AsyncClient(timeout=5.0) as client:
                resp = await client.get(
                    f"{settings.USER_SERVICE_URL}/api/v1/users/{user_id}/manager",
                    headers=headers
                )
                if resp.status_code == 200:
                    data = resp.json()
                    return data.get("manager_id")
                return None
        except Exception as e:
            logger.error(f"Error getting manager for {user_id}: {e}")
            return None
    
    @staticmethod
    async def get_subordinates(user_id: int, token: Optional[str] = None) -> List[int]:
        """Получить список подчиненных пользователя"""
        try:
            headers = {}
            if token:
                headers["Authorization"] = f"Bearer {token}"
            
            async with httpx.AsyncClient(timeout=5.0) as client:
                resp = await client.get(
                    f"{settings.USER_SERVICE_URL}/api/v1/users/{user_id}/subordinates",
                    headers=headers
                )
                if resp.status_code == 200:
                    data = resp.json()
                    return [u["user_id"] for u in data]
                return []
        except Exception as e:
            logger.error(f"Error getting subordinates for {user_id}: {e}")
            return []
    
    @staticmethod
    async def is_manager_of(manager_id: int, subordinate_id: int, token: Optional[str] = None) -> bool:
        """Проверить, является ли один пользователь руководителем другого"""
        subordinates = await ExternalService.get_subordinates(manager_id, token)
        return subordinate_id in subordinates
    
    @staticmethod
    async def send_notification(user_id: int, message: str, notification_type: str = "delegation"):
        """Отправить уведомление пользователю"""
        try:
            async with httpx.AsyncClient(timeout=5.0) as client:
                await client.post(
                    f"{settings.NOTIFICATION_SERVICE_URL}/api/v1/notifications",
                    json={
                        "user_id": user_id,
                        "message": message,
                        "type": notification_type
                    }
                )
        except Exception as e:
            logger.error(f"Error sending notification to {user_id}: {e}")
