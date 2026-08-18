# app/repositories/notification_repo.py
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, update, func
from typing import List, Optional
from datetime import datetime, timezone
from app.models.notification import Notification, NotificationStatus


class NotificationRepository:
    def __init__(self, db: AsyncSession):
        self.db = db
    
    async def create(self, **kwargs) -> Notification:
        notification = Notification(**kwargs)
        self.db.add(notification)
        await self.db.commit()
        await self.db.refresh(notification)
        return notification
    
    async def get_by_id(self, notification_id: int) -> Optional[Notification]:
        result = await self.db.execute(
            select(Notification).where(Notification.notification_id == notification_id)
        )
        return result.scalar_one_or_none()
    
    async def get_by_user(
        self, 
        user_id: int, 
        skip: int = 0, 
        limit: int = 100,
        only_unread: bool = False
    ) -> List[Notification]:
        query = select(Notification).where(Notification.user_id == user_id)
        if only_unread:
            query = query.where(Notification.status == NotificationStatus.UNREAD)
        query = query.order_by(Notification.created_at.desc()).offset(skip).limit(limit)
        result = await self.db.execute(query)
        return result.scalars().all()
    
    async def get_unread_count(self, user_id: int) -> int:
        result = await self.db.execute(
            select(func.count()).select_from(Notification)
            .where(
                Notification.user_id == user_id,
                Notification.status == NotificationStatus.UNREAD
            )
        )
        return result.scalar() or 0
    
    async def mark_as_read(self, notification_ids: List[int], user_id: int) -> int:
        if not notification_ids:
            return 0
        result = await self.db.execute(
            update(Notification)
            .where(
                Notification.notification_id.in_(notification_ids),
                Notification.user_id == user_id
            )
            .values(
                status=NotificationStatus.READ,
                read_at=datetime.now(timezone.utc)
            )
        )
        await self.db.commit()
        return result.rowcount
    
    async def mark_all_as_read(self, user_id: int) -> int:
        result = await self.db.execute(
            update(Notification)
            .where(
                Notification.user_id == user_id,
                Notification.status == NotificationStatus.UNREAD
            )
            .values(
                status=NotificationStatus.READ,
                read_at=datetime.now(timezone.utc)
            )
        )
        await self.db.commit()
        return result.rowcount
    
    async def get_unread(self, user_id: int, limit: int = 50) -> List[Notification]:
        return await self.get_by_user(user_id, only_unread=True, limit=limit)
