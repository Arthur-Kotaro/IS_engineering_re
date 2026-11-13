from sqlalchemy import Column, String, Text, Boolean, DateTime, BigInteger
from sqlalchemy.sql import func
from app.database import Base


class TaskGroup(Base):
    __tablename__ = "task_groups"

    group_id = Column(String(50), primary_key=True)
    name = Column(String(200), nullable=False)
    description = Column(Text, nullable=True)
    bus_instance_id = Column(String(50), nullable=False)

    is_deleted = Column(Boolean, nullable=False, default=False)
    deleted_at = Column(DateTime(timezone=True), nullable=True)
    deleted_by = Column(BigInteger, nullable=True)

    def __repr__(self):
        return f"<TaskGroup({self.group_id})>"
