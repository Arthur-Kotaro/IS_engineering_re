# app/schemas/delegation.py
from pydantic import BaseModel, Field, field_validator, model_validator
from typing import Optional, List
from datetime import datetime, timezone


class DelegationCreate(BaseModel):
    initiator_id: Optional[int] = None
    delegator_id: int
    delegate_id: int
    main_delegate_id: Optional[int] = None
    delegation_type: str = Field(..., pattern="^(direct|temporary)$")
    starts_at: datetime
    expires_at: datetime
    reason: Optional[str] = Field(None, max_length=500)

    @field_validator("expires_at")
    def check_dates(cls, v, info):
        starts_at = info.data.get("starts_at")
        if starts_at and v <= starts_at:
            raise ValueError("expires_at must be after starts_at")
        if v <= datetime.now(timezone.utc):
            raise ValueError("expires_at must be in the future")
        return v

    @field_validator("starts_at")
    def check_start(cls, v):
        return v

    @model_validator(mode="after")
    def check_self_and_temporary(self):
        if self.delegator_id == self.delegate_id:
            raise ValueError("delegator_id and delegate_id must differ")
        if self.delegation_type == "temporary":
            if self.main_delegate_id is None:
                raise ValueError("main_delegate_id required for temporary delegation")
            if self.main_delegate_id in (self.delegator_id, self.delegate_id):
                raise ValueError("main_delegate_id must differ from delegator and delegate")
        return self


class DelegationRevoke(BaseModel):
    reason: Optional[str] = Field(None, max_length=500)


class DelegationResponse(BaseModel):
    delegation_id: int
    initiator_id: int
    delegator_id: int
    delegator_name: Optional[str]
    delegate_id: int
    delegate_name: Optional[str]
    main_delegate_id: Optional[int]
    delegation_type: str
    starts_at: datetime
    expires_at: datetime
    status: str
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
    delegator_id: Optional[int] = None
    delegation_type: Optional[str] = None
    expires_at: Optional[datetime] = None
