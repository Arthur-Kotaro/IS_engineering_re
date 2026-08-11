# app/schemas/project.py (МИНИМАЛЬНАЯ ВЕРСИЯ)
from pydantic import BaseModel
from typing import Optional, List
from datetime import datetime
from app.models.project import ProjectStatus

class ProjectCreate(BaseModel):
    title: str
    description: Optional[str] = None
    status: ProjectStatus = ProjectStatus.DRAFT

class ProjectUpdate(BaseModel):
    title: Optional[str] = None
    description: Optional[str] = None
    status: Optional[ProjectStatus] = None

class ProjectStatusUpdate(BaseModel):
    status: ProjectStatus

class ProjectResponse(BaseModel):
    project_id: int
    title: str
    description: Optional[str]
    status: ProjectStatus
    created_by: int
    created_at: datetime
    updated_at: datetime
    members_count: int = 0

class ProjectDetailResponse(ProjectResponse):
    members: List[dict] = []

class ProjectMemberCreate(BaseModel):
    user_id: int
    role: str = "member"

class ProjectMemberResponse(BaseModel):
    user_id: int
    role: str
    joined_at: datetime

class CheckAccessResponse(BaseModel):
    has_access: bool
    role: Optional[str] = None
    project_status: Optional[ProjectStatus] = None
