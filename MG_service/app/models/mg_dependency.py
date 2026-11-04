# app/models/mg_dependency.py
import uuid
from sqlalchemy import Column, Integer, String, DateTime, ForeignKey
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy.sql import func
from app.database import Base


class MgDependency(Base):
    __tablename__ = "mg_dependencies"

    dep_id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    mg_id = Column(UUID(as_uuid=True), ForeignKey("mastergraphs.mg_id", ondelete="CASCADE"), nullable=False, index=True)
    predecessor_id = Column(UUID(as_uuid=True), ForeignKey("mg_tasks.task_id", ondelete="CASCADE"), nullable=False)
    successor_id = Column(UUID(as_uuid=True), ForeignKey("mg_tasks.task_id", ondelete="CASCADE"), nullable=False)
    dep_type = Column(String(20), nullable=False, default="finish_to_start")
    version = Column(Integer, nullable=False, default=1)
    created_at = Column(DateTime(timezone=True), nullable=False, server_default=func.now())

    def __repr__(self):
        return f"<MgDependency({self.predecessor_id}->{self.successor_id})>"
