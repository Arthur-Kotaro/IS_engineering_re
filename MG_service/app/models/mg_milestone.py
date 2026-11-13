import uuid
from sqlalchemy import Column, String, Integer, Date, Text, DateTime, BigInteger, Boolean, ForeignKey
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy.sql import func
from app.database import Base


class MgMilestoneStatus:
    PLANNED = 0
    COMPLETED = 1
    RESCHEDULED = 2


class MgMilestone(Base):
    __tablename__ = "mg_milestones"

    milestone_id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    mg_id = Column(UUID(as_uuid=True), ForeignKey("mastergraphs.mg_id", ondelete="CASCADE"), nullable=False, index=True)
    abbreviation = Column(String(20), nullable=False)
    full_name = Column(String(200), nullable=False)
    tooltip = Column(Text, nullable=True)
    week_offset = Column(Integer, nullable=False, default=0)
    planned_date = Column(Date, nullable=False)
    actual_date = Column(Date, nullable=True)
    status = Column(Integer, nullable=False, default=MgMilestoneStatus.PLANNED)
    sort_order = Column(Integer, nullable=False, default=0)
    version = Column(Integer, nullable=False, default=1)
    created_at = Column(DateTime(timezone=True), nullable=False, server_default=func.now())
    updated_at = Column(DateTime(timezone=True), nullable=False, server_default=func.now(), onupdate=func.now())

    is_deleted = Column(Boolean, nullable=False, default=False)
    deleted_at = Column(DateTime(timezone=True), nullable=True)
    deleted_by = Column(BigInteger, nullable=True)

    def __repr__(self):
        return f"<MgMilestone({self.abbreviation}, {self.planned_date}, status={self.status})>"
