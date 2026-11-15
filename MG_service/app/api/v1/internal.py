# app/api/v1/internal.py
import logging
from datetime import date
from fastapi import APIRouter, Depends, HTTPException, Header
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from pydantic import BaseModel
from typing import Optional

from app.config import settings
from app.database import get_db
from app.models.mg_task import MgTask
from app.services.mastergraph_creator import (
    create_mastergraph_from_typology,
    MastergraphCreationError,
)
from app.ws.manager import manager

logger = logging.getLogger(__name__)
router = APIRouter(tags=["Internal"])


def _check_internal_key(x_internal_key: Optional[str] = Header(None)):
    if settings.INTERNAL_API_KEY and x_internal_key != settings.INTERNAL_API_KEY:
        raise HTTPException(403, "Invalid X-Internal-Key")


class ForecastUpdateRequest(BaseModel):
    project_id: int
    task_id: str
    forecast_start: str | None = None
    forecast_end: str | None = None
    progress_current: int = -1
    progress_total: int = -1
    status: int = 0
    local_graph_state: int | None = None


@router.post("/forecast-update")
async def forecast_update(data: ForecastUpdateRequest, db: AsyncSession = Depends(get_db)):
    print(f"[FORECAST-UPDATE] task={data.task_id}, forecast_end={data.forecast_end}", flush=True)
    result = await db.execute(select(MgTask).where(MgTask.task_id == data.task_id))
    task = result.scalar_one_or_none()
    if not task:
        raise HTTPException(404, "Task not found")

    if data.forecast_start:
        task.forecast_start = date.fromisoformat(data.forecast_start)
    if data.forecast_end:
        task.forecast_end = date.fromisoformat(data.forecast_end)
    task.progress_current = data.progress_current
    task.progress_total = data.progress_total
    task.status = data.status

    await db.commit()
    print(f"[FORECAST-UPDATE] task saved to DB, mg_id={task.mg_id}", flush=True)

    diff = {
        "type": "diff",
        "changes": [{
            "op": "update",
            "entity": "task",
            "id": data.task_id,
            "fields": {
                "forecast_start": data.forecast_start,
                "forecast_end": data.forecast_end,
                "progress_current": data.progress_current,
                "progress_total": data.progress_total,
                "status": data.status,
                "local_graph_state": data.local_graph_state,
            },
            "version": task.version,
        }]
    }
    await manager.broadcast(str(task.mg_id), diff)
    return {"status": "ok"}


# ---------------- Mastergraph creation (from Project Service) ----------------

class CreateMastergraphRequest(BaseModel):
    project_id: int
    project_type: str
    typology_name: str
    cve_id: int
    ipp_id: int
    name: Optional[str] = None
    start_date: Optional[str] = None


class CreateMastergraphResponse(BaseModel):
    mg_id: str
    project_id: int
    typology_name: str
    is_approved: bool
    milestones_count: int
    created: bool  # True — создан, False — уже существовал (идемпотентность)


@router.post("/mastergraphs", response_model=CreateMastergraphResponse, dependencies=[Depends(_check_internal_key)])
async def create_mastergraph(
    data: CreateMastergraphRequest,
    db: AsyncSession = Depends(get_db),
):
    # Быстрая проверка "был ли МГ раньше" (для поля created; гонка защищена IntegrityError в creator)
    from app.models.mastergraph import Mastergraph
    existing = await db.execute(
        select(Mastergraph).where(
            Mastergraph.project_id == data.project_id,
            Mastergraph.is_deleted == False,
        )
    )
    was_existing = existing.scalar_one_or_none() is not None

    start_date = date.fromisoformat(data.start_date) if data.start_date else None

    try:
        mg = await create_mastergraph_from_typology(
            db=db,
            project_id=data.project_id,
            project_type=data.project_type,
            typology_name=data.typology_name,
            cve_id=data.cve_id,
            ipp_id=data.ipp_id,
            name=data.name,
            start_date=start_date,
        )
    except MastergraphCreationError as e:
        raise HTTPException(400, str(e))

    # посчитать вехи
    from app.models.mg_milestone import MgMilestone
    ms_result = await db.execute(
        select(MgMilestone).where(MgMilestone.mg_id == mg.mg_id, MgMilestone.is_deleted == False)
    )
    milestones_count = len(ms_result.scalars().all())

    return CreateMastergraphResponse(
        mg_id=str(mg.mg_id),
        project_id=mg.project_id,
        typology_name=mg.typology_name,
        is_approved=mg.is_approved,
        milestones_count=milestones_count,
        created=not was_existing,
    )
