# app/schemas/project.py
from pydantic import BaseModel, Field
from typing import Optional, List
from datetime import datetime


class ProjectCreate(BaseModel):
    title: str = Field(..., min_length=1, max_length=200)
    description: Optional[str] = None
    priority: int = 0
    chief_engineer_id: int
    planning_engineer_id: int
    status: str = "draft"


class ProjectUpdate(BaseModel):
    title: Optional[str] = Field(None, min_length=1, max_length=200)
    description: Optional[str] = None
    priority: Optional[int] = None
    status: Optional[str] = None
    chief_engineer_id: Optional[int] = None
    planning_engineer_id: Optional[int] = None


class ProjectResponse(BaseModel):
    project_id: int
    title: str
    description: Optional[str]
    priority: int
    status: str
    chief_engineer_id: Optional[int]
    planning_engineer_id: Optional[int]
    created_by: int
    created_at: datetime
    updated_at: datetime
    members_count: int = 0

    class Config:
        from_attributes = True


class ProjectMemberResponse(BaseModel):
    member_id: int
    user_id: int
    role_code: str
    joined_at: datetime

    class Config:
        from_attributes = True


class ProjectDetailResponse(ProjectResponse):
    members: List[ProjectMemberResponse] = []


class ProjectMemberCreate(BaseModel):
    user_id: int
    role_code: str = Field(..., min_length=1, max_length=50)


class ProjectRoleResponse(BaseModel):
    role_code: str
    role_name_en: str
    role_name_ru: str
    role_description_ru: Optional[str]

    class Config:
        from_attributes = True


class CheckAccessResponse(BaseModel):
    has_access: bool
    reason: Optional[str] = None
    role: Optional[str] = None
    project_status: Optional[str] = None
