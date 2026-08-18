# app/models/notification.py
from sqlalchemy import Column, Integer, String, Text, DateTime, Boolean, Enum, JSON, Index
from sqlalchemy.sql import func
from app.database import Base
import enum


class NotificationType(str, enum.Enum):
    # Безопасность
    LOGIN_NEW_DEVICE = "login_new_device"
    PASSWORD_CHANGED = "password_changed"
    PASSWORD_EXPIRY_WARNING = "password_expiry_warning"
    PASSWORD_EXPIRED = "password_expired"
    ACCOUNT_BLOCKED = "account_blocked"
    ACCOUNT_UNBLOCKED = "account_unblocked"
    LOGIN_FAILED = "login_failed"
    
    # Делегирование
    DELEGATION_CREATED = "delegation_created"
    DELEGATION_REVOKED = "delegation_revoked"
    DELEGATION_EXPIRED = "delegation_expired"
    DELEGATION_TEMPORARY = "delegation_temporary"
    
    # Проекты
    PROJECT_ASSIGNED = "project_assigned"
    PROJECT_UNASSIGNED = "project_unassigned"


class NotificationStatus(str, enum.Enum):
    UNREAD = "unread"
    READ = "read"


class Notification(Base):
    __tablename__ = "notifications"
    
    notification_id = Column(Integer, primary_key=True, autoincrement=True)
    user_id = Column(Integer, nullable=False)
    user_email = Column(String(255), nullable=True)
    user_name = Column(String(100), nullable=True)
    
    notification_type = Column(Enum(NotificationType), nullable=False)
    status = Column(Enum(NotificationStatus), default=NotificationStatus.UNREAD)
    
    title = Column(String(255), nullable=False)
    message = Column(Text, nullable=False)
    
    # Ссылка на связанную сущность
    reference_id = Column(Integer, nullable=True)
    reference_type = Column(String(50), nullable=True)
    
    # Дополнительные данные (JSON)
    data = Column(JSON, nullable=True)
    
    # Email статус
    send_email = Column(Boolean, default=False)
    email_sent = Column(Boolean, default=False)
    email_sent_at = Column(DateTime(timezone=True), nullable=True)
    
    # Аудит
    created_at = Column(DateTime(timezone=True), server_default=func.now())
    read_at = Column(DateTime(timezone=True), nullable=True)
    
    __table_args__ = (
        Index("idx_notifications_user_id", "user_id"),
        Index("idx_notifications_status", "status"),
        Index("idx_notifications_type", "notification_type"),  # ← исправлено
        Index("idx_notifications_created_at", "created_at"),
        Index("idx_notifications_user_status", "user_id", "status"),
    )
    
    def __repr__(self):
        return f"<Notification(id={self.notification_id}, user={self.user_id}, type={self.notification_type})>"
