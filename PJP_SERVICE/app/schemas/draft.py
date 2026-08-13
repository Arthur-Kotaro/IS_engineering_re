from pydantic import BaseModel, Field
from datetime import datetime
from uuid import UUID
from typing import Optional, List
from app.schemas.pjp import PJPPartBase


class PJPDraftCreate(BaseModel):
    lot_id: int
    comment: Optional[str] = None


class PJPDraftResponse(BaseModel):
    id: UUID
    lot_id: int
    created_by: int
    created_at: datetime
    updated_at: Optional[datetime]
    comment: Optional[str]
    is_published: bool
    parts: List[PJPPartBase] = []

    class Config:
        from_attributes = True


class PJPDraftPartCreate(BaseModel):
    part: PJPPartBase


class PJPDraftPartUpdate(BaseModel):
    part: PJPPartBase


class PJPDraftPublishResponse(BaseModel):
    message: str
    pjp_id: UUID
    status: str
    warnings: List[dict] = []
