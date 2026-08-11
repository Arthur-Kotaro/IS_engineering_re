from sqlalchemy import Column, Integer, String, DateTime, JSON, Enum
from sqlalchemy.sql import func
from app.database import Base
import enum

class PJPStatus(str, enum.Enum):
    UPLOADED = "uploaded"
    PROCTECH_REVIEW = "proctech_review"
    TECH_REVIEW = "tech_review"
    APPROVED = "approved"
    RELEASED = "released"

class PJP(Base):
    __tablename__ = "pjps"
    id = Column(Integer, primary_key=True, autoincrement=True)
    lot_id = Column(Integer, nullable=False)
    version = Column(String(20), default="1.0")
    status = Column(Enum(PJPStatus), default=PJPStatus.UPLOADED)
    created_by = Column(Integer, nullable=False)
    created_at = Column(DateTime(timezone=True), server_default=func.now())
    updated_at = Column(DateTime(timezone=True), onupdate=func.now())
    data = Column(JSON)
