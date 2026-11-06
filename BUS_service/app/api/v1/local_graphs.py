# app/api/v1/local_graphs.py
import logging
from uuid import UUID
from datetime import date
from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select

from app.database import get_db
from app.models.local_graph import LocalGraph, LgTask
from app.schemas.local_graph import (
    LocalGraphInitRequest, LocalGraphResponse,
    ForecastRecalcRequest, ForecastRecalcResponse,
)

logger = logging.getLogger(__name__)
router = APIRouter(prefix="/internal/local-graphs", tags=["LocalGraphs"])


@router.post("/init", response_model=LocalGraphResponse)
async def init_local_graph(data: LocalGraphInitRequest, db: AsyncSession = Depends(get_db)):
    existing = await db.execute(select(LocalGraph).where(LocalGraph.task_id == data.task_id))
    lg = existing.scalar_one_or_none()
    if lg:
        lg.responsible_id = data.responsible_id
        lg.title = data.title
        lg.target_start = data.target_start
        lg.target_end = data.target_end
        lg.predecessor_end = data.predecessor_end
        lg.version += 1
    else:
        lg = LocalGraph(
            project_id=data.project_id,
            task_id=data.task_id,
            responsible_id=data.responsible_id,
            title=data.title,
            target_start=data.target_start,
            target_end=data.target_end,
            predecessor_end=data.predecessor_end,
        )
        db.add(lg)
    await db.commit()
    await db.refresh(lg)
    logger.info(f"init_local_graph: task={data.task_id}, lg={lg.lg_id}")
    return lg


@router.get("/by-task/{task_id}", response_model=LocalGraphResponse)
async def get_by_task(task_id: UUID, db: AsyncSession = Depends(get_db)):
    result = await db.execute(select(LocalGraph).where(LocalGraph.task_id == task_id))
    lg = result.scalar_one_or_none()
    if not lg:
        raise HTTPException(404, "LocalGraph not found")
    return lg


@router.get("/{lg_id}/tasks")
async def get_lg_tasks(lg_id: UUID, db: AsyncSession = Depends(get_db)):
    result = await db.execute(select(LgTask).where(LgTask.lg_id == lg_id).order_by(LgTask.sort_order))
    tasks = result.scalars().all()
    return {
        "lg_id": str(lg_id),
        "tasks": [
            {
                "lg_task_id": str(t.lg_task_id),
                "title": t.title,
                "responsible_id": t.responsible_id,
                "status": t.status,
                "start_date": t.start_date.isoformat() if t.start_date else None,
                "end_date": t.end_date.isoformat() if t.end_date else None,
                "sort_order": t.sort_order,
            } for t in tasks
        ],
    }


@router.post("/forecast-recalc", response_model=ForecastRecalcResponse)
async def forecast_recalc(data: ForecastRecalcRequest, db: AsyncSession = Depends(get_db)):
    result = await db.execute(select(LocalGraph).where(LocalGraph.task_id == data.task_id))
    lg = result.scalar_one_or_none()
    if not lg:
        raise HTTPException(404, "LocalGraph not found")

    if data.predecessor_end:
        lg.predecessor_end = data.predecessor_end

    forecast_start = lg.target_start
    forecast_end = lg.target_end

    if lg.predecessor_end and forecast_start and forecast_start <= lg.predecessor_end:
        duration = (forecast_end - forecast_start).days if forecast_end else 0
        from datetime import timedelta
        forecast_start = lg.predecessor_end + timedelta(days=1)
        if duration > 0:
            forecast_end = forecast_start + timedelta(days=duration)

    progress_current = -1
    progress_total = -1
    status = 0

    await db.commit()
    logger.info(f"forecast_recalc: task={data.task_id}")
    return ForecastRecalcResponse(
        forecast_start=forecast_start,
        forecast_end=forecast_end,
        progress_current=progress_current,
        progress_total=progress_total,
        status=status,
    )
