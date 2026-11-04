import uuid
from sqlalchemy import Column, String, Boolean, Date, DateTime
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy.sql import func
from app.database import Base


class Holiday(Base):
    __tablename__ = "holidays"

    holiday_id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    name = Column(String(200), nullable=False)
    start_date = Column(Date, nullable=False)   # для is_yearly: год = 2000
    end_date = Column(Date, nullable=True)
    is_yearly = Column(Boolean, nullable=False, default=True)
    created_at = Column(DateTime(timezone=True), nullable=False, server_default=func.now())
    updated_at = Column(DateTime(timezone=True), nullable=False, server_default=func.now(), onupdate=func.now())

    def __repr__(self):
        return f"<Holiday({self.name}, {self.start_date})>"
