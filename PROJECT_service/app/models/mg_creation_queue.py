from sqlalchemy import Column, BigInteger, String, Text, Integer, DateTime, ForeignKey
from sqlalchemy.sql import func
from app.database import Base


class MgCreationQueueStatus:
    PENDING = "pending"
    PROCESSING = "processing"
    DONE = "done"
    FAILED = "failed"


class MgCreationQueue(Base):
    __tablename__ = "mg_creation_queue"

    id = Column(BigInteger, primary_key=True, autoincrement=True)
    project_id = Column(BigInteger, ForeignKey("projects.project_id", ondelete="CASCADE"), nullable=False, index=True)
    project_type = Column(String(50), nullable=False)
    typology_name = Column(String(50), nullable=False)
    cve_id = Column(BigInteger, nullable=False)
    ipp_id = Column(BigInteger, nullable=False)
    attempt = Column(Integer, nullable=False, default=0)
    max_attempts = Column(Integer, nullable=False, default=5)
    next_retry_at = Column(DateTime(timezone=True), nullable=False, server_default=func.now())
    status = Column(String(20), nullable=False, default=MgCreationQueueStatus.PENDING)
    last_error = Column(Text, nullable=True)
    created_at = Column(DateTime(timezone=True), nullable=False, server_default=func.now())
    updated_at = Column(DateTime(timezone=True), nullable=False, server_default=func.now(), onupdate=func.now())

    def __repr__(self):
        return f"<MgCreationQueue(project={self.project_id}, status={self.status}, attempt={self.attempt})>"
