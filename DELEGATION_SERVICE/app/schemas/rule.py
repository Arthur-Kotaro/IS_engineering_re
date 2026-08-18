# app/schemas/rule.py
from pydantic import BaseModel
from typing import Optional


class DelegationRuleCreate(BaseModel):
    role: str
    can_delegate: bool = False
    max_delegations: int = 1
    max_duration_days: int = 30
    requires_approval: bool = False
    direct_allowed: bool = True
    reverse_allowed: bool = False
    temporary_allowed: bool = True


class DelegationRuleResponse(BaseModel):
    rule_id: int
    role: str
    can_delegate: bool
    max_delegations: int
    max_duration_days: int
    requires_approval: bool
    direct_allowed: bool
    reverse_allowed: bool
    temporary_allowed: bool
    
    class Config:
        from_attributes = True
