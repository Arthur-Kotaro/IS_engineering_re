# app/models/lg_event.py
import uuid
from sqlalchemy import Column, String, Text, DateTime, BigInteger, ForeignKey
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy.sql import func
from app.database import Base


class LgEventType:
    CREATE = "create"
    UPDATE = "update"
    DELETE = "delete"
    STATUS_CHANGE = "status_change"
    COMPLETE = "complete"
    FORECAST_UPDATE = "forecast_update"


class LgEntityType:
    TASK = "task"
    LOCAL_GRAPH = "local_graph"
    DEPENDENCY = "dependency"


class LgEvent(Base):
    __tablename__ = "lg_events"

    event_id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    lg_id = Column(UUID(as_uuid=True), ForeignKey("local_graphs.lg_id", ondelete="CASCADE"), nullable=False, index=True)
    entity_type = Column(String(20), nullable=False)
    entity_id = Column(UUID(as_uuid=True), nullable=False)
    event_type = Column(String(30), nullable=False)
    field = Column(String(50), nullable=True)
    old_value = Column(Text, nullable=True)   # JSON-строка (SQLite не имеет JSONB)
    new_value = Column(Text, nullable=True)   # JSON-строка
    reason = Column(Text, nullable=True)
    actor_id = Column(BigInteger, nullable=True)
    created_at = Column(DateTime(timezone=True), nullable=False, server_default=func.now())
