# app/schemas/delegation.py
from pydantic import BaseModel, Field, field_validator
from typing import Optional, List
from datetime import datetime
from app.models.delegation import DelegationType, DelegationStatus


class DelegationCreate(BaseModel):
    delegator_id: int
    delegate_id: int
    delegation_type: DelegationType
    starts_at: datetime
    expires_at: datetime
    reason: Optional[str] = Field(None, max_length=500)
    main_delegate_id: Optional[int] = None
    
    @field_validator('expires_at')
    def validate_dates(cls, v, info):
        starts_at = info.data.get('starts_at')
        if starts_at and v <= starts_at:
            raise ValueError('expires_at must be after starts_at')
        return v
    
    @field_validator('main_delegate_id')
    def validate_main_delegate(cls, v, info):
        delegation_type = info.data.get('delegation_type')
        if delegation_type == DelegationType.TEMPORARY and v is None:
            raise ValueError('main_delegate_id required for TEMPORARY delegation')
        return v


class DelegationUpdate(BaseModel):
    expires_at: Optional[datetime] = None
    reason: Optional[str] = Field(None, max_length=500)
    status: Optional[DelegationStatus] = None


class DelegationRevoke(BaseModel):
    reason: Optional[str] = Field(None, max_length=500)


class DelegationResponse(BaseModel):
    delegation_id: int
    delegator_id: int
    delegator_name: Optional[str]
    delegate_id: int
    delegate_name: Optional[str]
    main_delegate_id: Optional[int]
    delegation_type: DelegationType
    starts_at: datetime
    expires_at: datetime
    status: DelegationStatus
    reason: Optional[str]
    created_by: int
    created_at: datetime
    updated_at: Optional[datetime]
    revoked_at: Optional[datetime]
    revoked_by: Optional[int]
    revoke_reason: Optional[str]
    
    class Config:
        from_attributes = True


class DelegationListResponse(BaseModel):
    total: int
    delegations: List[DelegationResponse]


class DelegationCheckResponse(BaseModel):
    has_delegation: bool
    delegation_id: Optional[int] = None
    delegated_from: Optional[int] = None
    delegated_to: Optional[int] = None
    delegation_type: Optional[DelegationType] = None
    expires_at: Optional[datetime] = None
