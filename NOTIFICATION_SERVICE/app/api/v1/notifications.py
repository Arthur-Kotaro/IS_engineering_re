# app/api/v1/notifications.py
from fastapi import APIRouter, Depends, HTTPException, status, Query, Request
from typing import List, Optional
from app.database import get_db
from app.schemas.notification import (
    NotificationCreate,
    NotificationResponse,
    NotificationListResponse,
    NotificationUnreadResponse,
    MarkReadRequest
)
from app.services.notification_service import NotificationService
from app.services.email_service import EmailService
from app.repositories.notification_repo import NotificationRepository
from sqlalchemy.ext.asyncio import AsyncSession
import jwt
from app.config import settings
import asyncio
import json
from sse_starlette.sse import EventSourceResponse

router = APIRouter(prefix="/api/v1/notifications", tags=["Notifications"])

# Хранилище активных SSE соединений
active_connections: dict[int, list[asyncio.Queue]] = {}


async def get_notification_service(db: AsyncSession = Depends(get_db)) -> NotificationService:
    notification_repo = NotificationRepository(db)
    email_service = EmailService()
    return NotificationService(notification_repo, email_service)


async def get_current_user_id(request: Request) -> int:
    """Получить ID текущего пользователя из заголовка X-User-ID"""
    user_id = request.headers.get("X-User-ID")
    if not user_id:
        raise HTTPException(401, "Missing X-User-ID header")
    try:
        return int(user_id)
    except ValueError:
        raise HTTPException(401, "Invalid X-User-ID format")


async def get_token(request: Request) -> Optional[str]:
    """Получить токен из заголовка Authorization"""
    auth_header = request.headers.get("Authorization")
    if not auth_header:
        return None
    return auth_header.replace("Bearer ", "").strip()


async def get_is_super_admin(request: Request) -> bool:
    """Проверить, является ли пользователь супер-админом"""
    auth_header = request.headers.get("Authorization")
    if not auth_header:
        return False
    token = auth_header.replace("Bearer ", "").strip()
    try:
        payload = jwt.decode(token, options={"verify_signature": False})
        return payload.get("is_super_admin", False)
    except Exception:
        return False


# ========== SSE (Server-Sent Events) ==========

@router.get("/stream")
async def notifications_stream(
    request: Request,
    current_user_id: int = Depends(get_current_user_id)
):
    """
    Server-Sent Events поток для получения уведомлений в реальном времени.
    Клиент подписывается на события и получает их мгновенно.
    
    Для клиента:
    1. Открыть SSE соединение: GET /api/v1/notifications/stream
    2. При получении события обновить бэйдж
    3. При отключении — автоматически переподключиться
    """
    async def event_generator():
        queue = asyncio.Queue()
        
        if current_user_id not in active_connections:
            active_connections[current_user_id] = []
        active_connections[current_user_id].append(queue)
        
        try:
            # Отправляем приветственное событие
            yield {
                "event": "connected",
                "data": json.dumps({
                    "status": "connected",
                    "user_id": current_user_id
                })
            }
            
            while True:
                try:
                    # Ждем новое уведомление с таймаутом (heartbeat)
                    notification = await asyncio.wait_for(queue.get(), timeout=30.0)
                    yield {
                        "event": "notification",
                        "data": notification
                    }
                except asyncio.TimeoutError:
                    # Heartbeat для поддержания соединения
                    yield {
                        "event": "ping",
                        "data": json.dumps({"type": "ping"})
                    }
        except asyncio.CancelledError:
            # Клиент отключился
            pass
        finally:
            if current_user_id in active_connections:
                active_connections[current_user_id].remove(queue)
                if not active_connections[current_user_id]:
                    del active_connections[current_user_id]
    
    return EventSourceResponse(event_generator())


async def broadcast_notification(user_id: int, notification_data: dict):
    """Отправить уведомление всем активным SSE клиентам пользователя"""
    if user_id in active_connections:
        message = json.dumps(notification_data)
        for queue in active_connections[user_id]:
            try:
                await queue.put(message)
            except Exception:
                pass


# ========== API для клиента ==========

@router.get("/unread/count", response_model=NotificationUnreadResponse)
async def get_unread_count(
    current_user_id: int = Depends(get_current_user_id),
    service: NotificationService = Depends(get_notification_service)
):
    """
    Получить количество непрочитанных уведомлений.
    Используется для отображения бэйджа в TopBar.
    """
    count = await service.get_unread_count(current_user_id)
    return NotificationUnreadResponse(unread_count=count)


@router.get("/unread", response_model=NotificationListResponse)
async def get_unread_notifications(
    limit: int = Query(50, ge=1, le=500),
    current_user_id: int = Depends(get_current_user_id),
    service: NotificationService = Depends(get_notification_service)
):
    """
    Получить непрочитанные уведомления (для окна уведомлений).
    """
    return await service.get_user_notifications(
        current_user_id, 
        skip=0, 
        limit=limit, 
        only_unread=True
    )


@router.get("/", response_model=NotificationListResponse)
async def get_notifications(
    skip: int = Query(0, ge=0),
    limit: int = Query(50, ge=1, le=500),
    only_unread: bool = Query(False),
    current_user_id: int = Depends(get_current_user_id),
    service: NotificationService = Depends(get_notification_service)
):
    """
    Получить список уведомлений с пагинацией.
    Используется для окна уведомлений с историей.
    """
    return await service.get_user_notifications(
        current_user_id, 
        skip, 
        limit, 
        only_unread
    )


@router.post("/read")
async def mark_notifications_read(
    request: MarkReadRequest,
    current_user_id: int = Depends(get_current_user_id),
    service: NotificationService = Depends(get_notification_service)
):
    """Отметить выбранные уведомления как прочитанные"""
    count = await service.mark_as_read(request.notification_ids, current_user_id)
    return {"marked_count": count, "message": f"{count} notifications marked as read"}


@router.post("/read/all")
async def mark_all_read(
    current_user_id: int = Depends(get_current_user_id),
    service: NotificationService = Depends(get_notification_service)
):
    """Отметить все уведомления как прочитанные"""
    count = await service.mark_all_as_read(current_user_id)
    return {"marked_count": count, "message": f"{count} notifications marked as read"}


# ========== Административные эндпоинты ==========

@router.post("/")
async def create_notification(
    data: NotificationCreate,
    is_super_admin: bool = Depends(get_is_super_admin),
    service: NotificationService = Depends(get_notification_service)
):
    """
    Создать уведомление (только для супер-админа или внутреннего использования).
    """
    if not is_super_admin:
        raise HTTPException(403, "Only super admin can create notifications")
    
    notification = await service.create_notification(data)
    
    # Отправляем SSE
    await broadcast_notification(
        data.user_id,
        {
            "notification_id": notification.notification_id,
            "type": data.notification_type,
            "title": data.title,
            "message": data.message,
            "created_at": notification.created_at.isoformat()
        }
    )
    
    return notification
