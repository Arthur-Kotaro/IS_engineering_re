import uuid
from sqlalchemy import Column, String, Text, DateTime, BigInteger, ForeignKey
from sqlalchemy.dialects.postgresql import UUID, JSONB
from sqlalchemy.sql import func
from app.database import Base


class MgEventType:
    CREATE = "create"
    UPDATE = "update"
    DELETE = "delete"
    RESCHEDULE = "reschedule"
    STATUS_CHANGE = "status_change"
    COMPLETE = "complete"
    APPROVE = "approve"


class MgEntityType:
    TASK = "task"
    MILESTONE = "milestone"
    MASTERGRAPH = "mastergraph"
    DEPENDENCY = "dependency"


class MgEvent(Base):
    __tablename__ = "mg_events"

    event_id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    mg_id = Column(UUID(as_uuid=True), ForeignKey("mastergraphs.mg_id", ondelete="CASCADE"), nullable=False, index=True)
    entity_type = Column(String(20), nullable=False)
    entity_id = Column(UUID(as_uuid=True), nullable=False)
    event_type = Column(String(30), nullable=False)
    field = Column(String(50), nullable=True)
    old_value = Column(JSONB, nullable=True)
    new_value = Column(JSONB, nullable=True)
    reason = Column(Text, nullable=True)
    actor_id = Column(BigInteger, nullable=True)
    created_at = Column(DateTime(timezone=True), nullable=False, server_default=func.now())

    def __repr__(self):
        return f"<MgEvent({self.event_type}, {self.entity_type}/{self.entity_id})>"
