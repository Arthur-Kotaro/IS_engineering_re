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

active_connections: dict[int, list[asyncio.Queue]] = {}


async def get_notification_service(db: AsyncSession = Depends(get_db)) -> NotificationService:
    notification_repo = NotificationRepository(db)
    email_service = EmailService()
    return NotificationService(notification_repo, email_service)


async def get_user_id_from_token(request: Request) -> int:
    auth_header = request.headers.get("Authorization")
    if not auth_header:
        raise HTTPException(401, "Authorization header required")
    
    token = auth_header.replace("Bearer ", "").strip()
    
    try:
        payload = jwt.decode(
            token,
            settings.SECRET_KEY,
            algorithms=[settings.ALGORITHM]
        )
        user_id = payload.get("user_id")
        if user_id is None:
            raise HTTPException(401, "Invalid token: missing user_id")
        return user_id
    except jwt.ExpiredSignatureError:
        raise HTTPException(401, "Token expired")
    except jwt.InvalidTokenError:
        raise HTTPException(401, "Invalid token")


async def get_user_id_internal(request: Request) -> int:
    user_id = request.headers.get("X-User-ID")
    if not user_id:
        raise HTTPException(401, "Missing X-User-ID header")
    try:
        return int(user_id)
    except ValueError:
        raise HTTPException(401, "Invalid X-User-ID format")


@router.get("", response_model=NotificationListResponse)
async def get_notifications(
    request: Request,
    limit: int = Query(20, ge=1, le=100),
    offset: int = Query(0, ge=0),
    status_filter: Optional[str] = None,
    user_id: int = Depends(get_user_id_from_token),
    service: NotificationService = Depends(get_notification_service)
):
    """Получить список уведомлений пользователя"""
    return await service.get_user_notifications(user_id, offset, limit, only_unread=(status_filter == "unread"))


@router.get("/unread", response_model=List[NotificationResponse])
async def get_unread_notifications(
    request: Request,
    user_id: int = Depends(get_user_id_from_token),
    service: NotificationService = Depends(get_notification_service)
):
    """Получить все непрочитанные уведомления пользователя"""
    return await service.get_unread(user_id)


@router.get("/unread/count", response_model=NotificationUnreadResponse)
async def get_unread_count(
    request: Request,
    user_id: int = Depends(get_user_id_from_token),
    service: NotificationService = Depends(get_notification_service)
):
    """Получить количество непрочитанных уведомлений"""
    count = await service.get_unread_count(user_id)
    return NotificationUnreadResponse(unread_count=count)


@router.post("/read")
async def mark_as_read(
    request: MarkReadRequest,
    user_id: int = Depends(get_user_id_from_token),
    service: NotificationService = Depends(get_notification_service)
):
    """Отметить уведомления как прочитанные"""
    await service.mark_as_read(request.notification_ids, user_id)
    return {"message": "Notifications marked as read"}


@router.post("/read/all")
async def mark_all_as_read(
    user_id: int = Depends(get_user_id_from_token),
    service: NotificationService = Depends(get_notification_service)
):
    """Отметить все уведомления как прочитанные"""
    await service.mark_all_as_read(user_id)
    return {"message": "All notifications marked as read"}


@router.post("/internal")
async def create_notification_internal(
    notification: NotificationCreate,
    user_id: int = Depends(get_user_id_internal),
    service: NotificationService = Depends(get_notification_service)
):
    """Внутренний эндпоинт для создания уведомлений (используется другими сервисами)"""
    created = await service.create_notification(notification)
    return created


@router.get("/stream")
async def stream_notifications(
    request: Request,
    user_id: int = Depends(get_user_id_from_token)
):
    """SSE поток уведомлений для пользователя"""
    
    if user_id not in active_connections:
        active_connections[user_id] = []
    
    queue = asyncio.Queue()
    active_connections[user_id].append(queue)
    
    async def event_generator():
        try:
            while True:
                if await request.is_disconnected():
                    break
                
                try:
                    notification = await asyncio.wait_for(queue.get(), timeout=30.0)
                    yield {
                        "event": "notification",
                        "data": json.dumps(notification)
                    }
                except asyncio.TimeoutError:
                    yield {
                        "event": "ping",
                        "data": ""
                    }
        finally:
            if user_id in active_connections:
                active_connections[user_id].remove(queue)
                if not active_connections[user_id]:
                    del active_connections[user_id]
    
    return EventSourceResponse(event_generator())


async def send_notification_to_user(user_id: int, notification_data: dict):
    """Отправить уведомление в SSE поток пользователя"""
    if user_id in active_connections:
        for queue in active_connections[user_id]:
            await queue.put(notification_data)
