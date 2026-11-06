# app/schemas/local_graph.py
from pydantic import BaseModel
from typing import Optional, List
from datetime import date, datetime
from uuid import UUID


class LocalGraphInitRequest(BaseModel):
    project_id: int
    task_id: UUID
    responsible_id: Optional[int] = None
    title: str
    target_start: Optional[date] = None
    target_end: Optional[date] = None
    predecessor_end: Optional[date] = None


class LocalGraphResponse(BaseModel):
    lg_id: UUID
    project_id: int
    task_id: UUID
    responsible_id: Optional[int]
    title: str
    target_start: Optional[date]
    target_end: Optional[date]
    predecessor_end: Optional[date]
    version: int

    class Config:
        from_attributes = True


class LgTaskResponse(BaseModel):
    lg_task_id: UUID
    lg_id: UUID
    title: str
    responsible_id: Optional[int]
    status: int
    start_date: Optional[date]
    end_date: Optional[date]
    sort_order: int
    version: int

    class Config:
        from_attributes = True


class LgTaskCreate(BaseModel):
    title: str
    responsible_id: Optional[int] = None
    start_date: Optional[date] = None
    end_date: Optional[date] = None
    sort_order: int = 0


class LgTaskUpdate(BaseModel):
    title: Optional[str] = None
    responsible_id: Optional[int] = None
    status: Optional[int] = None
    start_date: Optional[date] = None
    end_date: Optional[date] = None
    sort_order: Optional[int] = None


class ForecastRecalcRequest(BaseModel):
    project_id: int
    task_id: UUID
    predecessor_end: Optional[date] = None


class ForecastRecalcResponse(BaseModel):
    forecast_start: Optional[date]
    forecast_end: Optional[date]
    progress_current: int = -1
    progress_total: int = -1
    status: int = 0
