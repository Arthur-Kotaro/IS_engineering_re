# app/services/external_service.py
import httpx
import logging
from typing import Optional, List, Dict, Any
from app.config import settings

logger = logging.getLogger(__name__)


class ExternalService:
    @staticmethod
    async def get_user_info(user_id: int, token: Optional[str] = None) -> Optional[Dict[str, Any]]:
        """Получить информацию о пользователе из User Service через Gateway"""
        try:
            headers = {}
            if token:
                headers["Authorization"] = f"Bearer {token}"
            
            url = f"{settings.USER_SERVICE_URL}/api/v1/admin/users/{user_id}"
            logger.info(f"🔍 Fetching user info from: {url}")
            
            async with httpx.AsyncClient(timeout=5.0) as client:
                resp = await client.get(url, headers=headers)
                if resp.status_code == 200:
                    return resp.json()
                logger.warning(f"Failed to get user info for {user_id}: {resp.status_code} - {resp.text[:100]}")
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
    async def send_notification(
        user_id: int,
        message: str,
        title: str = "Уведомление",
        notification_type: str = "system",
        reference_id: Optional[int] = None,
        reference_type: Optional[str] = None,
        data: Optional[Dict] = None,
        send_email: bool = True,
        token: Optional[str] = None
    ):
        """Отправить уведомление пользователю через Notification Service"""
        try:
            # Получаем информацию о пользователе с токеном
            user_info = await ExternalService.get_user_info(user_id, token)
            if not user_info:
                logger.warning(f"Cannot send notification to {user_id}: user not found")
                return
            
            logger.info(f"📧 Sending notification to user {user_id} ({user_info.get('user_name')})")
            
            # Используем внутренний эндпоинт (без проверки прав)
            url = f"{settings.NOTIFICATION_SERVICE_URL}/api/v1/notifications/internal"
            
            # Заголовки с токеном для авторизации в Notification Service
            headers = {}
            if token:
                headers["Authorization"] = f"Bearer {token}"
            
            async with httpx.AsyncClient(timeout=5.0, follow_redirects=True) as client:
                resp = await client.post(
                    url,
                    headers=headers,
                    json={
                        "user_id": user_id,
                        "user_email": user_info.get("email"),
                        "user_name": user_info.get("user_name"),
                        "notification_type": notification_type,
                        "title": title,
                        "message": message,
                        "reference_id": reference_id,
                        "reference_type": reference_type,
                        "data": data or {},
                        "send_email": send_email
                    }
                )
                if resp.status_code == 200:
                    logger.info(f"✅ Notification sent to user {user_id}: {title}")
                else:
                    logger.warning(f"Failed to send notification: {resp.status_code} - {resp.text[:100]}")
        except httpx.ConnectError as e:
            logger.warning(f"❌ Notification Service not available: {e}")
        except Exception as e:
            logger.error(f"Error sending notification to {user_id}: {e}")
