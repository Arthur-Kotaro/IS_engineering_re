# app/schemas/rule.py
from pydantic import BaseModel, Field


class DelegationRuleCreate(BaseModel):
    role: str = Field(..., min_length=1, max_length=50)
    can_delegate: bool = False
    max_delegations: int = Field(1, ge=0)
    max_duration_days: int = Field(30, ge=0)
    requires_approval: bool = False
    direct_allowed: bool = True
    temporary_allowed: bool = True


class DelegationRuleResponse(BaseModel):
    rule_id: int
    role: str
    can_delegate: bool
    max_delegations: int
    max_duration_days: int
    requires_approval: bool
    direct_allowed: bool
    temporary_allowed: bool

    class Config:
        from_attributes = True
