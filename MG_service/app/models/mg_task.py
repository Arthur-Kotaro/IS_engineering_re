# app/models/mg_task.py
import uuid
from sqlalchemy import Column, String, Integer, Date, Text, DateTime, BigInteger, ForeignKey
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy.sql import func
from app.database import Base


class MgTask(Base):
    __tablename__ = "mg_tasks"

    task_id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    mg_id = Column(UUID(as_uuid=True), ForeignKey("mastergraphs.mg_id", ondelete="CASCADE"), nullable=False, index=True)
    group_id = Column(String(50), ForeignKey("task_groups.group_id"), nullable=False, index=True)
    parent_task_id = Column(UUID(as_uuid=True), ForeignKey("mg_tasks.task_id", ondelete="CASCADE"), nullable=True)
    title = Column(String(500), nullable=False)
    responsible_id = Column(BigInteger, nullable=True)
    status = Column(Integer, nullable=False, default=0)  # 0=planned,1=completed,2=risks,3=blocked
    start_date = Column(Date, nullable=True)
    end_date = Column(Date, nullable=True)
    forecast_start = Column(Date, nullable=True)
    forecast_end = Column(Date, nullable=True)
    progress_current = Column(Integer, nullable=False, default=-1)
    progress_total = Column(Integer, nullable=False, default=-1)
    sort_order = Column(Integer, nullable=False, default=0)
    version = Column(Integer, nullable=False, default=1)
    created_at = Column(DateTime(timezone=True), nullable=False, server_default=func.now())
    updated_at = Column(DateTime(timezone=True), nullable=False, server_default=func.now(), onupdate=func.now())

    def __repr__(self):
        return f"<MgTask({self.task_id}, {self.title[:30]})>"
