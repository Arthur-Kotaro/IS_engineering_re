# app/models/local_graph.py
import uuid
from sqlalchemy import Column, String, Integer, Date, Text, DateTime, BigInteger, ForeignKey
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy.sql import func
from app.database import Base


class LocalGraph(Base):
    __tablename__ = "local_graphs"

    lg_id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    project_id = Column(BigInteger, nullable=False, index=True)
    task_id = Column(UUID(as_uuid=True), nullable=False, unique=True, index=True)
    responsible_id = Column(BigInteger, nullable=True)
    title = Column(String(500), nullable=False)
    target_start = Column(Date, nullable=True)
    target_end = Column(Date, nullable=True)
    predecessor_end = Column(Date, nullable=True)
    version = Column(Integer, nullable=False, default=1)
    created_at = Column(DateTime(timezone=True), nullable=False, server_default=func.now())
    updated_at = Column(DateTime(timezone=True), nullable=False, server_default=func.now(), onupdate=func.now())


class LgTask(Base):
    __tablename__ = "lg_tasks"

    lg_task_id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    lg_id = Column(UUID(as_uuid=True), ForeignKey("local_graphs.lg_id", ondelete="CASCADE"), nullable=False, index=True)
    title = Column(String(500), nullable=False)
    responsible_id = Column(BigInteger, nullable=True)
    status = Column(Integer, nullable=False, default=0)  # 0=planned, 1=completed
    start_date = Column(Date, nullable=True)
    end_date = Column(Date, nullable=True)
    sort_order = Column(Integer, nullable=False, default=0)
    version = Column(Integer, nullable=False, default=1)


class LgDependency(Base):
    __tablename__ = "lg_dependencies"

    dep_id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    lg_id = Column(UUID(as_uuid=True), ForeignKey("local_graphs.lg_id", ondelete="CASCADE"), nullable=False)
    predecessor_id = Column(UUID(as_uuid=True), ForeignKey("lg_tasks.lg_task_id"), nullable=False)
    successor_id = Column(UUID(as_uuid=True), ForeignKey("lg_tasks.lg_task_id"), nullable=False)
    dep_type = Column(String(20), nullable=False, default="finish_to_start")
