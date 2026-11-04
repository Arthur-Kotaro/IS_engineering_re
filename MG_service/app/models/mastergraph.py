# app/models/mastergraph.py
import uuid
from sqlalchemy import Column, String, Integer, BigInteger, DateTime
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy.sql import func
from app.database import Base


class Mastergraph(Base):
    __tablename__ = "mastergraphs"

    mg_id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    project_id = Column(BigInteger, nullable=False, unique=True, index=True)
    name = Column(String(200), nullable=False)
    version = Column(Integer, nullable=False, default=1)
    created_by = Column(BigInteger, nullable=True)
    created_at = Column(DateTime(timezone=True), nullable=False, server_default=func.now())
    updated_at = Column(DateTime(timezone=True), nullable=False, server_default=func.now(), onupdate=func.now())

    def __repr__(self):
        return f"<Mastergraph({self.mg_id}, project={self.project_id})>"
