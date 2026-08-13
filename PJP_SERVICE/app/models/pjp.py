from sqlalchemy import Column, String, Integer, DateTime, Text, Boolean, DECIMAL, ForeignKey, JSON, Enum
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy.sql import func
from sqlalchemy.orm import relationship, backref
from app.database import Base
import uuid
import enum


class PJPStatus(str, enum.Enum):
    UPLOADED = "UPLOADED"
    PROCTECH_REVIEW = "PROCTECH_REVIEW"
    TECH_REVIEW = "TECH_REVIEW"
    APPROVED = "APPROVED"
    RELEASED = "RELEASED"


class PartLineStatus(str, enum.Enum):
    ACTIVE = "active"
    EXCLUDED = "excluded"


class SourceType(str, enum.Enum):
    PROTOTYPE = "PROTOTYPE"
    PURCHASE = "PURCHASE"
    SERIAL = "SERIAL"
    REFINING = "REFINING"


class PJP(Base):
    __tablename__ = "pjps"

    id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    lot_id = Column(Integer, nullable=False)
    version = Column(String(20), nullable=False, default="1.0")
    status = Column(Enum(PJPStatus), nullable=False, default=PJPStatus.UPLOADED)
    created_by = Column(Integer, nullable=False)
    created_at = Column(DateTime(timezone=True), server_default=func.now())
    updated_at = Column(DateTime(timezone=True), onupdate=func.now())
    activity_id = Column(String(50), nullable=True)
    comment = Column(Text, nullable=True)

    parts = relationship("PJPPart", back_populates="pjp", cascade="all, delete-orphan")
    change_log = relationship("PJPChangeLog", back_populates="pjp", cascade="all, delete-orphan")

    def __repr__(self):
        return f"<PJP(id={self.id}, lot_id={self.lot_id}, status={self.status})>"


class PJPPart(Base):
    __tablename__ = "pjp_parts"

    id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    pjp_id = Column(UUID(as_uuid=True), ForeignKey("pjps.id", ondelete="CASCADE"), nullable=False)
    parent_id = Column(UUID(as_uuid=True), ForeignKey("pjp_parts.id"), nullable=True)

    line_status = Column(Enum(PartLineStatus), nullable=False, default=PartLineStatus.ACTIVE)
    gfe_code = Column(String(20), nullable=True)
    part_number = Column(String(50), nullable=False, index=True)
    part_name = Column(String(500), nullable=False)
    part_name_en = Column(String(500), nullable=True)
    cd_status = Column(String(50), nullable=True)
    cd_ready_week = Column(Integer, nullable=True)
    quantity = Column(DECIMAL(10, 2), nullable=False)
    unit = Column(String(10), nullable=True, default="шт")
    source_type = Column(Enum(SourceType), nullable=False)
    supplier = Column(String(200), nullable=True)
    supplier_code = Column(String(50), nullable=True)
    parent_model = Column(String(50), nullable=True)
    requires_cd = Column(Boolean, default=False)
    comment = Column(Text, nullable=True)
    architecture_zone = Column(String(50), nullable=True)
    function = Column(String(50), nullable=True)
    pg_number = Column(String(50), nullable=True)
    responsible = Column(String(200), nullable=True)
    applies_to_prototypes = Column(JSON, nullable=True)

    created_at = Column(DateTime(timezone=True), server_default=func.now())
    updated_at = Column(DateTime(timezone=True), onupdate=func.now())

    pjp = relationship("PJP", back_populates="parts")
    children = relationship("PJPPart", backref=backref("parent", remote_side=[id]))


class PJPChangeLog(Base):
    __tablename__ = "pjp_change_log"

    id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    pjp_id = Column(UUID(as_uuid=True), ForeignKey("pjps.id", ondelete="CASCADE"), nullable=False)
    user_id = Column(Integer, nullable=False)
    action = Column(String(50), nullable=False)
    details = Column(JSON, nullable=True)
    created_at = Column(DateTime(timezone=True), server_default=func.now())

    pjp = relationship("PJP", back_populates="change_log")
