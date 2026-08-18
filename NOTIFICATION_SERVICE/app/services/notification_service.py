# app/services/notification_service.py
from typing import List, Optional
from datetime import datetime, timezone
from app.repositories.notification_repo import NotificationRepository
from app.services.email_service import EmailService
from app.schemas.notification import NotificationCreate, NotificationResponse, NotificationListResponse
from app.models.notification import NotificationStatus


class NotificationService:
    def __init__(
        self,
        notification_repo: NotificationRepository,
        email_service: EmailService
    ):
        self.notification_repo = notification_repo
        self.email_service = email_service
    
    async def create_notification(
        self,
        data: NotificationCreate
    ) -> NotificationResponse:
        """Создать уведомление"""
        notification = await self.notification_repo.create(**data.model_dump())
        
        # Отправка email если нужно
        if data.send_email and data.user_email:
            sent = await self.email_service.send_email(
                to_email=data.user_email,
                subject=data.title,
                body=data.message
            )
            if sent:
                notification.email_sent = True
                notification.email_sent_at = datetime.now(timezone.utc)
                await self.notification_repo.db.commit()
                await self.notification_repo.db.refresh(notification)
        
        return NotificationResponse.model_validate(notification)
    
    async def get_user_notifications(
        self,
        user_id: int,
        skip: int = 0,
        limit: int = 100,
        only_unread: bool = False
    ) -> NotificationListResponse:
        """Получить уведомления пользователя"""
        notifications = await self.notification_repo.get_by_user(
            user_id, skip, limit, only_unread
        )
        total = len(notifications)
        unread_count = await self.notification_repo.get_unread_count(user_id)
        
        return NotificationListResponse(
            total=total,
            unread_count=unread_count,
            notifications=[NotificationResponse.model_validate(n) for n in notifications]
        )
    
    async def get_unread_count(self, user_id: int) -> int:
        """Получить количество непрочитанных"""
        return await self.notification_repo.get_unread_count(user_id)
    
    async def get_unread(self, user_id: int, limit: int = 50) -> List[NotificationResponse]:
        """Получить непрочитанные уведомления"""
        notifications = await self.notification_repo.get_unread(user_id, limit)
        return [NotificationResponse.model_validate(n) for n in notifications]
    
    async def mark_as_read(self, notification_ids: List[int], user_id: int) -> int:
        """Отметить уведомления как прочитанные"""
        return await self.notification_repo.mark_as_read(notification_ids, user_id)
    
    async def mark_all_as_read(self, user_id: int) -> int:
        """Отметить все уведомления как прочитанные"""
        return await self.notification_repo.mark_all_as_read(user_id)
