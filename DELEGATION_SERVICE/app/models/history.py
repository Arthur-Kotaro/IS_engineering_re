# app/models/history.py
from sqlalchemy import Column, Integer, String, DateTime, ForeignKey, JSON, Index
from sqlalchemy.sql import func
from sqlalchemy.orm import relationship
from app.database import Base


class DelegationHistory(Base):
    __tablename__ = "delegation_history"
    
    history_id = Column(Integer, primary_key=True, autoincrement=True)
    delegation_id = Column(Integer, ForeignKey("delegations.delegation_id", ondelete="CASCADE"))
    action = Column(String(50), nullable=False)  # CREATED, ACTIVATED, REVOKED, EXPIRED, UPDATED
    user_id = Column(Integer, nullable=False)
    user_name = Column(String(100), nullable=True)
    timestamp = Column(DateTime(timezone=True), server_default=func.now())
    details = Column(JSON, nullable=True)
    ip_address = Column(String(45), nullable=True)
    user_agent = Column(String(255), nullable=True)
    
    delegation = relationship("Delegation", back_populates="history")
    
    __table_args__ = (
        Index("idx_history_delegation_id", "delegation_id"),
        Index("idx_history_user_id", "user_id"),
        Index("idx_history_timestamp", "timestamp"),
    )
    
    def __repr__(self):
        return f"<DelegationHistory(id={self.history_id}, delegation={self.delegation_id}, action={self.action})>"
