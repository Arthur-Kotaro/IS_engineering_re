from pydantic import BaseModel, Field, validator
from datetime import datetime
from uuid import UUID
from typing import Optional, List, Any
from app.models.pjp import PJPStatus, SourceType, PartLineStatus


class PJPPartBase(BaseModel):
    part_number: str = Field(..., min_length=1, max_length=50)
    part_name: str = Field(..., min_length=1, max_length=500)
    part_name_en: Optional[str] = None
    gfe_code: Optional[str] = None
    quantity: str = Field(..., description="Количество с единицей измерения, например '1', '1,6 л.'")
    unit: Optional[str] = "шт"
    source_type: SourceType
    supplier: Optional[str] = None
    supplier_code: Optional[str] = None
    parent_model: Optional[str] = None
    cd_status: Optional[str] = None
    cd_ready_week: Optional[int] = None
    comment: Optional[str] = None
    architecture_zone: Optional[str] = None
    function: Optional[str] = None
    pg_number: Optional[str] = None
    responsible: Optional[str] = None
    applies_to_prototypes: Optional[List[str]] = []
    line_status: PartLineStatus = PartLineStatus.ACTIVE

    @validator('applies_to_prototypes')
    def validate_applies_to(cls, v):
        if v is None:
            return []
        return v


class PJPPartResponse(PJPPartBase):
    id: UUID
    pjp_id: UUID
    created_at: datetime
    updated_at: Optional[datetime]

    class Config:
        from_attributes = True


class PJPBase(BaseModel):
    lot_id: int
    version: str = "1.0"
    comment: Optional[str] = None
    activity_id: Optional[str] = None


class PJPResponse(PJPBase):
    id: UUID
    status: PJPStatus
    created_by: int
    created_at: datetime
    updated_at: Optional[datetime]
    parts: List[PJPPartResponse] = []

    class Config:
        from_attributes = True


class PJPStatusUpdate(BaseModel):
    status: PJPStatus
    comment: Optional[str] = None
