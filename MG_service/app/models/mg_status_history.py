from sqlalchemy import Column, BigInteger, String, Text, DateTime, ForeignKey
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy.sql import func
from app.database import Base


class MgStatusHistory(Base):
    __tablename__ = "mg_status_history"

    id = Column(BigInteger, primary_key=True, autoincrement=True)
    mg_id = Column(UUID(as_uuid=True), ForeignKey("mastergraphs.mg_id", ondelete="CASCADE"), nullable=False, index=True)
    old_status = Column(String(20), nullable=False)
    new_status = Column(String(20), nullable=False)
    reason = Column(Text, nullable=True)
    actor_id = Column(BigInteger, nullable=False)
    created_at = Column(DateTime(timezone=True), nullable=False, server_default=func.now())

    def __repr__(self):
        return f"<MgStatusHistory({self.old_status}->{self.new_status}, mg={self.mg_id})>"
