# app/models/rule.py
from sqlalchemy import Column, Integer, String, Boolean, Index
from app.database import Base


class DelegationRule(Base):
    __tablename__ = "delegation_rules"

    rule_id = Column(Integer, primary_key=True, autoincrement=True)
    role = Column(String(50), nullable=False, unique=True)
    can_delegate = Column(Boolean, default=False)
    max_delegations = Column(Integer, default=1)
    max_duration_days = Column(Integer, default=30)
    requires_approval = Column(Boolean, default=False)
    direct_allowed = Column(Boolean, default=True)
    temporary_allowed = Column(Boolean, default=True)

    __table_args__ = ()

    def __repr__(self):
        return f"<DelegationRule(role={self.role}, can_delegate={self.can_delegate})>"
