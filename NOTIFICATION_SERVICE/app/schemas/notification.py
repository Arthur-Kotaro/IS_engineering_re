# app/schemas/notification.py
from pydantic import BaseModel, Field
from typing import Optional, List, Dict, Any
from datetime import datetime
from app.models.notification import NotificationType, NotificationStatus


class NotificationCreate(BaseModel):
    user_id: int
    user_email: Optional[str] = None
    user_name: Optional[str] = None
    notification_type: NotificationType
    title: str = Field(..., max_length=255)
    message: str
    reference_id: Optional[int] = None
    reference_type: Optional[str] = None
    data: Optional[Dict[str, Any]] = None
    send_email: bool = False


class NotificationResponse(BaseModel):
    notification_id: int
    user_id: int
    user_email: Optional[str]
    user_name: Optional[str]
    notification_type: NotificationType
    status: NotificationStatus
    title: str
    message: str
    reference_id: Optional[int]
    reference_type: Optional[str]
    data: Optional[Dict[str, Any]]
    created_at: datetime
    read_at: Optional[datetime]
    
    class Config:
        from_attributes = True


class NotificationListResponse(BaseModel):
    total: int
    unread_count: int
    notifications: List[NotificationResponse]


class NotificationUnreadResponse(BaseModel):
    unread_count: int


class MarkReadRequest(BaseModel):
    notification_ids: List[int]
