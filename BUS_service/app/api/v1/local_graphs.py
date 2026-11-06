# app/api/v1/local_graphs.py
import logging
import aiohttp
from uuid import UUID
from datetime import date, timedelta
from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, func

from app.database import get_db
from app.models.local_graph import LocalGraph, LgTask
from app.schemas.local_graph import (
    LocalGraphInitRequest, LocalGraphResponse,
    LgTaskResponse, LgTaskCreate, LgTaskUpdate,
    ForecastRecalcRequest, ForecastRecalcResponse,
)

logger = logging.getLogger(__name__)
router = APIRouter(prefix="/internal/local-graphs", tags=["LocalGraphs"])

MG_SERVICE_URL = "http://localhost:8003"


# ---------- LocalGraph ----------
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


# ---------- LgTask CRUD ----------
@router.get("/{lg_id}/tasks")
async def get_lg_tasks(lg_id: UUID, db: AsyncSession = Depends(get_db)):
    result = await db.execute(select(LgTask).where(LgTask.lg_id == lg_id).order_by(LgTask.sort_order))
    tasks = result.scalars().all()
    return {
        "lg_id": str(lg_id),
        "tasks": [
            {
                "lg_task_id": str(t.lg_task_id),
                "lg_id": str(t.lg_id),
                "title": t.title,
                "responsible_id": t.responsible_id,
                "status": t.status,
                "start_date": t.start_date.isoformat() if t.start_date else None,
                "end_date": t.end_date.isoformat() if t.end_date else None,
                "sort_order": t.sort_order,
                "version": t.version,
            } for t in tasks
        ],
    }


@router.post("/{lg_id}/tasks", response_model=LgTaskResponse, status_code=201)
async def create_lg_task(lg_id: UUID, data: LgTaskCreate, db: AsyncSession = Depends(get_db)):
    lg = await db.execute(select(LocalGraph).where(LocalGraph.lg_id == lg_id))
    if not lg.scalar_one_or_none():
        raise HTTPException(404, "LocalGraph not found")

    task = LgTask(
        lg_id=lg_id,
        title=data.title,
        responsible_id=data.responsible_id,
        start_date=data.start_date,
        end_date=data.end_date,
        sort_order=data.sort_order,
    )
    db.add(task)
    await db.commit()
    await db.refresh(task)
    logger.info(f"create_lg_task: lg={lg_id}, task={task.lg_task_id}")
    return task


@router.put("/{lg_id}/tasks/{task_id}", response_model=LgTaskResponse)
async def update_lg_task(lg_id: UUID, task_id: UUID, data: LgTaskUpdate, db: AsyncSession = Depends(get_db)):
    result = await db.execute(select(LgTask).where(LgTask.lg_task_id == task_id, LgTask.lg_id == lg_id))
    task = result.scalar_one_or_none()
    if not task:
        raise HTTPException(404, "LgTask not found")

    if data.title is not None: task.title = data.title
    if data.responsible_id is not None: task.responsible_id = data.responsible_id
    if data.status is not None: task.status = data.status
    if data.start_date is not None: task.start_date = data.start_date
    if data.end_date is not None: task.end_date = data.end_date
    if data.sort_order is not None: task.sort_order = data.sort_order
    task.version += 1

    await db.commit()
    await db.refresh(task)
    logger.info(f"update_lg_task: task={task_id}, status={task.status}")

    # При изменении статуса — пересчитать прогноз
    await _recalc_and_push(lg_id, db)
    return task


@router.delete("/{lg_id}/tasks/{task_id}", status_code=204)
async def delete_lg_task(lg_id: UUID, task_id: UUID, db: AsyncSession = Depends(get_db)):
    result = await db.execute(select(LgTask).where(LgTask.lg_task_id == task_id, LgTask.lg_id == lg_id))
    task = result.scalar_one_or_none()
    if not task:
        raise HTTPException(404, "LgTask not found")
    await db.delete(task)
    await db.commit()
    await _recalc_and_push(lg_id, db)
    return None


# ---------- Пересчёт прогноза ЛГ ----------
@router.post("/{lg_id}/recalc", response_model=ForecastRecalcResponse)
async def recalc_lg(lg_id: UUID, db: AsyncSession = Depends(get_db)):
    result = await db.execute(select(LocalGraph).where(LocalGraph.lg_id == lg_id))
    lg = result.scalar_one_or_none()
    if not lg:
        raise HTTPException(404, "LocalGraph not found")
    return await _recalc_and_push(lg_id, db)


@router.post("/forecast-recalc", response_model=ForecastRecalcResponse)
async def forecast_recalc(data: ForecastRecalcRequest, db: AsyncSession = Depends(get_db)):
    result = await db.execute(select(LocalGraph).where(LocalGraph.task_id == data.task_id))
    lg = result.scalar_one_or_none()
    if not lg:
        raise HTTPException(404, "LocalGraph not found")

    if data.predecessor_end:
        lg.predecessor_end = data.predecessor_end

    forecast = await _compute_forecast(lg, db)
    await db.commit()
    logger.info(f"forecast_recalc: task={data.task_id}")
    return forecast


async def _compute_forecast(lg: LocalGraph, db: AsyncSession) -> ForecastRecalcResponse:
    """Прогноз = min(start_date) / max(end_date) по задачам ЛГ. Fallback — target."""
    result = await db.execute(select(LgTask).where(LgTask.lg_id == lg.lg_id))
    tasks = result.scalars().all()

    if tasks:
        starts = [t.start_date for t in tasks if t.start_date]
        ends = [t.end_date for t in tasks if t.end_date]
        completed = sum(1 for t in tasks if t.status == 1)
        forecast_start = min(starts) if starts else lg.target_start
        forecast_end = max(ends) if ends else lg.target_end
        progress_current = completed
        progress_total = len(tasks)
        status = 1 if completed == len(tasks) else 0
    else:
        forecast_start = lg.target_start
        forecast_end = lg.target_end
        progress_current = -1
        progress_total = -1
        status = 0

    if lg.predecessor_end and forecast_start and forecast_start <= lg.predecessor_end:
        duration = (forecast_end - forecast_start).days if forecast_end else 0
        forecast_start = lg.predecessor_end + timedelta(days=1)
        if duration > 0:
            forecast_end = forecast_start + timedelta(days=duration)

    return ForecastRecalcResponse(
        forecast_start=forecast_start,
        forecast_end=forecast_end,
        progress_current=progress_current,
        progress_total=progress_total,
        status=status,
    )


async def _recalc_and_push(lg_id: UUID, db: AsyncSession):
    """Пересчитать прогноз ЛГ и запушить в MG."""
    result = await db.execute(select(LocalGraph).where(LocalGraph.lg_id == lg_id))
    lg = result.scalar_one_or_none()
    if not lg:
        return
    forecast = await _compute_forecast(lg, db)
    await db.commit()

    payload = {
        "project_id": lg.project_id,
        "task_id": str(lg.task_id),
        "forecast_start": forecast.forecast_start.isoformat() if forecast.forecast_start else None,
        "forecast_end": forecast.forecast_end.isoformat() if forecast.forecast_end else None,
        "progress_current": forecast.progress_current,
        "progress_total": forecast.progress_total,
        "status": forecast.status,
    }
    try:
        async with aiohttp.ClientSession(timeout=aiohttp.ClientTimeout(total=3.0)) as s:
            async with s.post(f"{MG_SERVICE_URL}/internal/forecast-update", json=payload) as resp:
                if resp.status == 200:
                    logger.info(f"pushed forecast to MG: task={lg.task_id}")
                else:
                    logger.warning(f"push forecast to MG: HTTP {resp.status}")
    except Exception as e:
        logger.warning(f"push forecast to MG: {e}")
