# app/dependencies.py
from fastapi import HTTPException, status, Request, Depends
import logging

logger = logging.getLogger(__name__)

async def get_current_user_id(request: Request) -> int:
    """Получить ID текущего пользователя из заголовка X-User-ID"""
    user_id = request.headers.get("X-User-ID")
    logger.info(f"📌 X-User-ID header: {user_id}")
    logger.info(f"📌 All headers: {dict(request.headers)}")
    
    if not user_id:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Missing X-User-ID header"
        )
    try:
        return int(user_id)
    except ValueError:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid X-User-ID format"
        )

async def get_current_user_role(request: Request) -> str:
    """Получить роль текущего пользователя из заголовка X-User-Role"""
    role = request.headers.get("X-User-Role", "user")
    return role

def require_role(allowed_roles: list):
    """Фабрика для проверки ролей"""
    async def role_checker(role: str = Depends(get_current_user_role)):
        if role not in allowed_roles:
            raise HTTPException(status_code=403, detail="Insufficient permissions")
        return role
    return role_checker
