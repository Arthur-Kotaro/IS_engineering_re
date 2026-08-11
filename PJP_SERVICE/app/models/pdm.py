from sqlalchemy import Column, Integer, String, DateTime, ForeignKey
from sqlalchemy.sql import func
from app.database import Base

class PDMDocument(Base):
    __tablename__ = "pdm_documents"
    
    id = Column(Integer, primary_key=True, autoincrement=True)
    reference_number = Column(String(10), unique=True, nullable=False)
    name = Column(String(255), nullable=False)
    description = Column(String(500))
    file_url = Column(String(500))
    version = Column(String(10), default="1.0")
    status = Column(String(20), default="active")  # active, archived, obsolete
    uploaded_by = Column(Integer, nullable=False)
    uploaded_at = Column(DateTime(timezone=True), server_default=func.now())
    updated_at = Column(DateTime(timezone=True), onupdate=func.now())
