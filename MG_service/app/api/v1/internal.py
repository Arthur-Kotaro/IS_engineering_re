# app/api/v1/internal.py
import logging
from datetime import date
from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from pydantic import BaseModel

from app.database import get_db
from app.models.mg_task import MgTask
from app.ws.manager import manager

logger = logging.getLogger(__name__)
router = APIRouter(tags=["Internal"])


class ForecastUpdateRequest(BaseModel):
    project_id: int
    task_id: str
    forecast_start: str | None = None
    forecast_end: str | None = None
    progress_current: int = -1
    progress_total: int = -1
    status: int = 0


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
            },
            "version": task.version,
        }]
    }
    await manager.broadcast(str(task.mg_id), diff)
    return {"status": "ok"}
