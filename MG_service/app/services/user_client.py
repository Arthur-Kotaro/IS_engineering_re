# app/services/user_client.py
import aiohttp
import logging
from typing import Optional
from app.config import settings

logger = logging.getLogger(__name__)


async def get_user_name(user_id: int) -> Optional[str]:
    """Получить ФИО пользователя из User Service (internal endpoint)."""
    headers = {"X-Internal-Key": settings.INTERNAL_API_KEY}
    try:
        async with aiohttp.ClientSession(timeout=aiohttp.ClientTimeout(total=3.0)) as s:
            async with s.get(
                f"{settings.USER_SERVICE_URL}/internal/users/{user_id}",
                headers=headers,
            ) as resp:
                if resp.status == 200:
                    data = await resp.json()
                    return data.get("full_name") or data.get("user_name")
                logger.warning(f"get_user_name({user_id}): HTTP {resp.status}")
    except Exception as e:
        logger.warning(f"get_user_name({user_id}): {e}")
    return None
