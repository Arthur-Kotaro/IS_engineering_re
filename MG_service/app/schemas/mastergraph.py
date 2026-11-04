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
    status: int
    start_date: Optional[date]
    end_date: Optional[date]
    forecast_start: Optional[date]
    forecast_end: Optional[date]
    progress_current: int
    progress_total: int
    sort_order: int
    version: int

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


class TaskGroupResponse(BaseModel):
    group_id: str
    name: str
    description: Optional[str]
    bus_instance_id: str

    class Config:
        from_attributes = True


class MastergraphResponse(BaseModel):
    mg_id: UUID
    project_id: int
    name: str
    version: int
    created_by: Optional[int]
    created_at: datetime
    updated_at: datetime

    class Config:
        from_attributes = True


class MastergraphDetailResponse(MastergraphResponse):
    tasks: List[MgTaskResponse] = []
    dependencies: List[MgDependencyResponse] = []
    groups: List[TaskGroupResponse] = []


class MastergraphCreate(BaseModel):
    project_id: int
    name: str
    created_by: Optional[int] = None


class CanEditResponse(BaseModel):
    can_edit: bool
    can_view: bool
    reason: Optional[str] = None
