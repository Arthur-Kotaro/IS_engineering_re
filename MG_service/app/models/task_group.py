# app/models/task_group.py
from sqlalchemy import Column, String, Text
from app.database import Base


class TaskGroup(Base):
    __tablename__ = "task_groups"

    group_id = Column(String(50), primary_key=True)
    name = Column(String(200), nullable=False)
    description = Column(Text, nullable=True)
    bus_instance_id = Column(String(50), nullable=False)

    def __repr__(self):
        return f"<TaskGroup({self.group_id})>"
