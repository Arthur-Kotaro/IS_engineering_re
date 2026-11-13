import uuid
from sqlalchemy import Column, String, Integer, Boolean, DateTime, BigInteger, ForeignKey
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy.sql import func
from app.database import Base


class MgGroup(Base):
    __tablename__ = "mg_groups"

    mg_group_id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    mg_id = Column(UUID(as_uuid=True), ForeignKey("mastergraphs.mg_id", ondelete="CASCADE"), nullable=False, index=True)
    group_id = Column(String(50), ForeignKey("task_groups.group_id"), nullable=False, index=True)
    sort_order = Column(Integer, nullable=False, default=0)
    created_at = Column(DateTime(timezone=True), nullable=False, server_default=func.now())
    updated_at = Column(DateTime(timezone=True), nullable=False, server_default=func.now(), onupdate=func.now())

    is_deleted = Column(Boolean, nullable=False, default=False)
    deleted_at = Column(DateTime(timezone=True), nullable=True)
    deleted_by = Column(BigInteger, nullable=True)

    def __repr__(self):
        return f"<MgGroup(mg={self.mg_id}, group={self.group_id})>"
