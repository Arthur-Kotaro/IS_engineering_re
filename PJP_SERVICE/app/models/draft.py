from sqlalchemy import Column, String, Integer, DateTime, Text, JSON, ForeignKey, Boolean
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy.sql import func
from sqlalchemy.orm import relationship, Mapped, mapped_column
from app.database import Base
import uuid
from datetime import datetime
from typing import Optional, List


class PJPDraft(Base):
    __tablename__ = "pjp_drafts"
    
    # Используем современный синтаксис SQLAlchemy 2.0
    id: Mapped[uuid.UUID] = mapped_column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    lot_id: Mapped[int] = mapped_column(Integer, nullable=False)
    created_by: Mapped[int] = mapped_column(Integer, nullable=False)
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())
    updated_at: Mapped[Optional[datetime]] = mapped_column(DateTime(timezone=True), onupdate=func.now())
    comment: Mapped[Optional[str]] = mapped_column(Text, nullable=True)
    is_published: Mapped[bool] = mapped_column(Boolean, default=False)
    
    # Отношения
    parts: Mapped[List["PJPDraftPart"]] = relationship(
        "PJPDraftPart", 
        back_populates="draft", 
        cascade="all, delete-orphan"
    )


class PJPDraftPart(Base):
    __tablename__ = "pjp_draft_parts"
    
    id: Mapped[uuid.UUID] = mapped_column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    draft_id: Mapped[uuid.UUID] = mapped_column(
        UUID(as_uuid=True), 
        ForeignKey("pjp_drafts.id", ondelete="CASCADE"), 
        nullable=False
    )
    
    line_status: Mapped[str] = mapped_column(String(20), default="active")
    gfe_code: Mapped[Optional[str]] = mapped_column(String(20), nullable=True)
    part_number: Mapped[str] = mapped_column(String(50), nullable=False, index=True)
    part_name: Mapped[str] = mapped_column(String(500), nullable=False)
    part_name_en: Mapped[Optional[str]] = mapped_column(String(500), nullable=True)
    cd_status: Mapped[Optional[str]] = mapped_column(String(50), nullable=True)
    cd_ready_week: Mapped[Optional[int]] = mapped_column(Integer, nullable=True)
    quantity: Mapped[str] = mapped_column(String(20), nullable=False)
    unit: Mapped[Optional[str]] = mapped_column(String(10), nullable=True, default="шт")
    source_type: Mapped[str] = mapped_column(String(20), nullable=False)
    supplier: Mapped[Optional[str]] = mapped_column(String(200), nullable=True)
    supplier_code: Mapped[Optional[str]] = mapped_column(String(50), nullable=True)
    parent_model: Mapped[Optional[str]] = mapped_column(String(50), nullable=True)
    requires_cd: Mapped[bool] = mapped_column(Boolean, default=False)
    comment: Mapped[Optional[str]] = mapped_column(Text, nullable=True)
    architecture_zone: Mapped[Optional[str]] = mapped_column(String(50), nullable=True)
    function: Mapped[Optional[str]] = mapped_column(String(50), nullable=True)
    pg_number: Mapped[Optional[str]] = mapped_column(String(50), nullable=True)
    responsible: Mapped[Optional[str]] = mapped_column(String(200), nullable=True)
    applies_to_prototypes: Mapped[Optional[dict]] = mapped_column(JSON, nullable=True)
    
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())
    updated_at: Mapped[Optional[datetime]] = mapped_column(DateTime(timezone=True), onupdate=func.now())
    
    # Отношения
    draft: Mapped["PJPDraft"] = relationship("PJPDraft", back_populates="parts")
