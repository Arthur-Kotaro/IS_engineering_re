# app/schemas/project.py
from pydantic import BaseModel, Field
from typing import Optional, List
from datetime import datetime


class ProjectCreate(BaseModel):
    title: str = Field(..., min_length=1, max_length=200)
    description: Optional[str] = None
    status: str = "draft"


class ProjectUpdate(BaseModel):
    title: Optional[str] = Field(None, min_length=1, max_length=200)
    description: Optional[str] = None
    status: Optional[str] = None


class ProjectResponse(BaseModel):
    project_id: int
    title: str
    description: Optional[str]
    status: str
    created_by: int
    created_at: datetime
    updated_at: datetime
    members_count: int = 0
    access_via: Optional[str] = None
    role: Optional[str] = None
    via_user_id: Optional[int] = None

    class Config:
        from_attributes = True


class ProjectDetailResponse(ProjectResponse):
    members: List[dict] = []


class ProjectMemberCreate(BaseModel):
    user_id: int
    role: str = "viewer"


class ProjectMemberResponse(BaseModel):
    user_id: int
    role: str
    joined_at: datetime

    class Config:
        from_attributes = True


class CheckAccessResponse(BaseModel):
    has_access: bool
    reason: Optional[str] = None
    role: Optional[str] = None
    via_user_id: Optional[int] = None
    project_status: Optional[str] = None
