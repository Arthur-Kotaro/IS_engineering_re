# app/models/delegation.py
from sqlalchemy import Column, Integer, String, DateTime, Boolean, Text, Index, CheckConstraint
from sqlalchemy.sql import func
from sqlalchemy.orm import relationship
from app.database import Base


class DelegationType(str):
    DIRECT = "direct"
    TEMPORARY = "temporary"


class DelegationStatus(str):
    ACTIVE = "active"
    EXPIRED = "expired"
    REVOKED = "revoked"
    PENDING = "pending"


class Delegation(Base):
    __tablename__ = "delegations"

    delegation_id = Column(Integer, primary_key=True, autoincrement=True)

    initiator_id = Column(Integer, nullable=False, index=True)

    delegator_id = Column(Integer, nullable=False, index=True)
    delegator_name = Column(String(100), nullable=True)

    delegate_id = Column(Integer, nullable=False, index=True)
    delegate_name = Column(String(100), nullable=True)

    main_delegate_id = Column(Integer, nullable=True)

    delegation_type = Column(String(20), nullable=False)

    starts_at = Column(DateTime(timezone=True), nullable=False)
    expires_at = Column(DateTime(timezone=True), nullable=False)

    status = Column(String(20), default=DelegationStatus.ACTIVE, nullable=False)

    reason = Column(String(500), nullable=True)

    created_by = Column(Integer, nullable=False)
    created_at = Column(DateTime(timezone=True), server_default=func.now())
    updated_at = Column(DateTime(timezone=True), onupdate=func.now())

    revoked_at = Column(DateTime(timezone=True), nullable=True)
    revoked_by = Column(Integer, nullable=True)
    revoke_reason = Column(String(500), nullable=True)

    history = relationship("DelegationHistory", back_populates="delegation", cascade="all, delete-orphan")

    __table_args__ = (
        CheckConstraint("delegation_type IN ('direct', 'temporary')", name="ck_delegation_type"),
        CheckConstraint("status IN ('active', 'expired', 'revoked', 'pending')", name="ck_delegation_status"),
        Index("idx_delegations_initiator", "initiator_id"),
        Index("idx_delegations_delegator", "delegator_id"),
        Index("idx_delegations_delegate", "delegate_id"),
        Index("idx_delegations_status", "status"),
        Index("idx_delegations_expires", "expires_at"),
    )

    def __repr__(self):
        return f"<Delegation(id={self.delegation_id}, type={self.delegation_type}, delegator={self.delegator_id}, delegate={self.delegate_id})>"
