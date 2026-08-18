# app/models/delegation.py
from sqlalchemy import Column, Integer, String, DateTime, Boolean, Enum, Text, ForeignKey, Index
from sqlalchemy.sql import func
from sqlalchemy.orm import relationship
from app.database import Base
import enum


class DelegationType(str, enum.Enum):
    DIRECT = "direct"          # Руководитель → подчиненный
    REVERSE = "reverse"        # Руководитель с правами подчиненного
    TEMPORARY = "temporary"    # Руководитель → другой подчиненный


class DelegationStatus(str, enum.Enum):
    ACTIVE = "active"
    EXPIRED = "expired"
    REVOKED = "revoked"
    PENDING = "pending"


class Delegation(Base):
    __tablename__ = "delegations"
    
    delegation_id = Column(Integer, primary_key=True, autoincrement=True)
    
    # Кто делегирует (руководитель)
    delegator_id = Column(Integer, nullable=False)
    delegator_name = Column(String(100), nullable=True)
    
    # Кому делегирует (подчиненный)
    delegate_id = Column(Integer, nullable=False)
    delegate_name = Column(String(100), nullable=True)
    
    # Для temporary делегирования (основной подчиненный, которого заменяют)
    main_delegate_id = Column(Integer, nullable=True)
    
    # Тип делегирования
    delegation_type = Column(Enum(DelegationType), nullable=False)
    
    # Временные рамки
    starts_at = Column(DateTime(timezone=True), nullable=False)
    expires_at = Column(DateTime(timezone=True), nullable=False)
    
    # Статус
    status = Column(Enum(DelegationStatus), default=DelegationStatus.ACTIVE)
    
    # Причина
    reason = Column(String(500), nullable=True)
    
    # Кто создал
    created_by = Column(Integer, nullable=False)
    created_at = Column(DateTime(timezone=True), server_default=func.now())
    updated_at = Column(DateTime(timezone=True), onupdate=func.now())
    
    # Отзыв
    revoked_at = Column(DateTime(timezone=True), nullable=True)
    revoked_by = Column(Integer, nullable=True)
    revoke_reason = Column(String(500), nullable=True)
    
    # Связи
    history = relationship("DelegationHistory", back_populates="delegation", cascade="all, delete-orphan")
    
    __table_args__ = (
        Index("idx_delegations_delegator_id", "delegator_id"),
        Index("idx_delegations_delegate_id", "delegate_id"),
        Index("idx_delegations_status", "status"),
        Index("idx_delegations_expires_at", "expires_at"),
        Index("idx_delegations_type", "delegation_type"),
    )
    
    def __repr__(self):
        return f"<Delegation(id={self.delegation_id}, delegator={self.delegator_id}, delegate={self.delegate_id}, type={self.delegation_type})>"
