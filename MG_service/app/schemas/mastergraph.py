# app/schemas/mastergraph.py
from pydantic import BaseModel
from typing import Optional, List
from datetime import date, datetime
from uuid import UUID


class MgTaskResponse(BaseModel):
    task_id: UUID
    mg_id: UUID
    group_id: str
    parent_task_id: Optional[UUID]
    title: str
    responsible_id: Optional[int]
    responsible_name: Optional[str] = None
    local_graph_state: int = 0
    status: int
    start_date: Optional[date]
    end_date: Optional[date]
    forecast_start: Optional[date]
    forecast_end: Optional[date]
    progress_current: int
    progress_total: int
    sort_order: int
    comment: Optional[str] = None
    version: int
    date_history: List[dict] = []

    class Config:
        from_attributes = True


class MgDependencyResponse(BaseModel):
    dep_id: UUID
    mg_id: UUID
    predecessor_id: UUID
    successor_id: UUID
    dep_type: str
    version: int

    class Config:
        from_attributes = True


class MgMilestoneResponse(BaseModel):
    milestone_id: UUID
    mg_id: UUID
    abbreviation: str
    full_name: str
    tooltip: Optional[str] = None
    week_offset: int
    planned_date: date
    actual_date: Optional[date]
    status: int
    sort_order: int
    version: int

    class Config:
        from_attributes = True


class MgGroupResponse(BaseModel):
    group_id: str
    name: str
    description: Optional[str] = None
    bus_instance_id: str
    bus_url: Optional[str] = None

    class Config:
        from_attributes = True


class MastergraphResponse(BaseModel):
    mg_id: UUID
    project_id: int
    name: str
    version: int
    is_approved: bool = False
    is_pure: bool = False
    typology_name: Optional[str] = None
    created_by: Optional[int]
    created_at: datetime
    updated_at: datetime

    class Config:
        from_attributes = True


class MastergraphDetailResponse(MastergraphResponse):
    tasks: List[MgTaskResponse] = []
    dependencies: List[MgDependencyResponse] = []
    milestones: List[MgMilestoneResponse] = []
    groups: List[MgGroupResponse] = []


class MastergraphCreate(BaseModel):
    project_id: int
    name: str
    created_by: Optional[int] = None


class CanEditResponse(BaseModel):
    can_edit: bool
    can_view: bool
    reason: Optional[str] = None
    is_approved: bool = False


class BatchTaskItem(BaseModel):
    batch_id: str
    group_id: str
    title: str
    responsible_id: Optional[int] = None
    start_date: Optional[str] = None
    end_date: Optional[str] = None
    sort_order: int = 0
    parent_batch_id: Optional[str] = None
    comment: Optional[str] = None


class BatchDependencyItem(BaseModel):
    predecessor_batch_id: str
    successor_batch_id: str


class BatchCreateRequest(BaseModel):
    tasks: List[BatchTaskItem]
    dependencies: List[BatchDependencyItem] = []


class BatchTaskCreated(BaseModel):
    batch_id: str
    task_id: UUID


class BatchCreateResponse(BaseModel):
    tasks: List[BatchTaskCreated]
    dependencies: List[UUID]


class ConflictItem(BaseModel):
    title: str
    group_id: str
    group_name: Optional[str] = None


class ConflictResponse(BaseModel):
    detail: str = "task_title_conflict"
    conflicts: List[ConflictItem]


# ---------------- Groups wizard ----------------

class CreateGroupsRequest(BaseModel):
    group_ids: List[str]
    position: str = "bottom"   # "top" | "bottom"
    after_group_id: Optional[str] = None


class CreateCustomGroupRequest(BaseModel):
    name: str
    description: Optional[str] = None
    position: str = "bottom"
    after_group_id: Optional[str] = None


class RenameGroupRequest(BaseModel):
    name: str
