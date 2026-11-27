# app/api/v1/local_graphs.py
import json
import logging
from uuid import UUID
from datetime import date, timedelta
from fastapi import APIRouter, Depends, HTTPException, Request
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, func
import aiohttp

from app.database import get_db
from app.models.local_graph import LocalGraph, LgTask, LgDependency, LocalGraphState
from app.models.lg_event import LgEvent, LgEventType, LgEntityType
from app.schemas.local_graph import (
    LocalGraphInitRequest, LocalGraphResponse,
    LgTaskResponse, LgTaskCreate, LgTaskUpdate,
    ForecastRecalcRequest, ForecastRecalcResponse,
    LgDependencyCreate, LgDependencyResponse, LgDependencyListResponse,
    LgCompleteRequest,
)
from app.ws.manager import manager

logger = logging.getLogger(__name__)
router = APIRouter(prefix="/internal/local-graphs", tags=["LocalGraphs"])

MG_SERVICE_URL = "http://localhost:8003"


def _user_id(request: Request) -> int:
    uid = request.headers.get("X-User-ID")
    if not uid:
        raise HTTPException(401, "Missing X-User-ID")
    return int(uid)


def _try_user_id(request: Request) -> int:
    uid = request.headers.get("X-User-ID")
    if not uid:
        return 0
    try:
        return int(uid)
    except ValueError:
        return 0


async def _check_edit_access(lg: LocalGraph, user_id: int):
    if lg.responsible_id is None:
        raise HTTPException(403, "No responsible assigned")
    if lg.responsible_id != user_id:
        raise HTTPException(403, "Only responsible can edit local graph")


async def _log_event(db, lg_id, entity_type, entity_id, event_type, field=None, old_value=None, new_value=None, reason=None, actor_id=None):
    ev = LgEvent(
        lg_id=lg_id,
        entity_type=entity_type,
        entity_id=entity_id,
        event_type=event_type,
        field=field,
        old_value=json.dumps(old_value) if old_value is not None else None,
        new_value=json.dumps(new_value) if new_value is not None else None,
        reason=reason,
        actor_id=actor_id,
    )
    db.add(ev)
    await db.flush()
    return ev


async def _recalc_state(lg: LocalGraph, db: AsyncSession):
    if lg.responsible_id is None:
        lg.local_graph_state = LocalGraphState.NOT_REQUIRED
        return
    result = await db.execute(
        select(func.count(LgTask.lg_task_id)).where(
            LgTask.lg_id == lg.lg_id,
            LgTask.is_deleted == 0,
            LgTask.start_date.isnot(None),
            LgTask.end_date.isnot(None),
        )
    )
    count = result.scalar() or 0
    if count > 0:
        lg.local_graph_state = LocalGraphState.ATTACHED
    else:
        lg.local_graph_state = LocalGraphState.REQUIRED


async def _build_snapshot(lg_id: UUID, db: AsyncSession) -> dict:
    result = await db.execute(select(LocalGraph).where(LocalGraph.lg_id == lg_id, LocalGraph.is_deleted == 0))
    lg = result.scalar_one_or_none()
    if not lg:
        return {}
    tasks_result = await db.execute(
        select(LgTask).where(LgTask.lg_id == lg_id, LgTask.is_deleted == 0).order_by(LgTask.sort_order)
    )
    tasks = tasks_result.scalars().all()
    tasks_count = await db.execute(
        select(func.count(LgTask.lg_task_id)).where(LgTask.lg_id == lg_id, LgTask.is_deleted == 0)
    )
    is_pure = (tasks_count.scalar() or 0) == 0

    deps_result = await db.execute(
        select(LgDependency).where(LgDependency.lg_id == lg_id, LgDependency.is_deleted == 0)
    )
    deps = deps_result.scalars().all()

    return {
        "type": "snapshot",
        "lg_id": str(lg.lg_id),
        "project_id": lg.project_id,
        "task_id": str(lg.task_id),
        "responsible_id": lg.responsible_id,
        "title": lg.title,
        "target_start": lg.target_start.isoformat() if lg.target_start else None,
        "target_end": lg.target_end.isoformat() if lg.target_end else None,
        "predecessor_end": lg.predecessor_end.isoformat() if lg.predecessor_end else None,
        "version": lg.version,
        "local_graph_state": lg.local_graph_state,
        "is_pure": is_pure,
        "tasks": [
            {"lg_task_id": str(t.lg_task_id), "lg_id": str(t.lg_id),
             "title": t.title, "responsible_id": t.responsible_id, "status": t.status,
             "start_date": t.start_date.isoformat() if t.start_date else None,
             "end_date": t.end_date.isoformat() if t.end_date else None,
             "sort_order": t.sort_order, "version": t.version}
            for t in tasks
        ],
        "dependencies": [
            {"dep_id": str(d.dep_id), "lg_id": str(d.lg_id),
             "predecessor_id": str(d.predecessor_id), "successor_id": str(d.successor_id),
             "dep_type": d.dep_type, "version": d.version}
            for d in deps
        ],
    }


async def _broadcast_snapshot(lg_id: UUID, db: AsyncSession):
    snapshot = await _build_snapshot(lg_id, db)
    if snapshot:
        await manager.broadcast(str(lg_id), snapshot)


# ---------------- LocalGraph ----------------

@router.post("/init", response_model=LocalGraphResponse)
async def init_local_graph(data: LocalGraphInitRequest, db: AsyncSession = Depends(get_db)):
    existing = await db.execute(
        select(LocalGraph).where(LocalGraph.task_id == data.task_id, LocalGraph.is_deleted == 0)
    )
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
    await db.flush()
    await _recalc_state(lg, db)
    await db.commit()
    await db.refresh(lg)

    tasks_count = await db.execute(
        select(func.count(LgTask.lg_task_id)).where(LgTask.lg_id == lg.lg_id, LgTask.is_deleted == 0)
    )
    is_pure = (tasks_count.scalar() or 0) == 0

    await _broadcast_snapshot(lg.lg_id, db)

    return {
        "lg_id": lg.lg_id, "project_id": lg.project_id, "task_id": lg.task_id,
        "responsible_id": lg.responsible_id, "title": lg.title,
        "target_start": lg.target_start, "target_end": lg.target_end,
        "predecessor_end": lg.predecessor_end, "version": lg.version,
        "local_graph_state": lg.local_graph_state, "is_pure": is_pure,
        "can_edit": False,
    }


@router.get("/by-task/{task_id}", response_model=LocalGraphResponse)
async def get_by_task(task_id: UUID, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _try_user_id(request)
    result = await db.execute(
        select(LocalGraph).where(LocalGraph.task_id == task_id, LocalGraph.is_deleted == 0)
    )
    lg = result.scalar_one_or_none()
    if not lg:
        raise HTTPException(404, "LocalGraph not found")

    tasks_count = await db.execute(
        select(func.count(LgTask.lg_task_id)).where(LgTask.lg_id == lg.lg_id, LgTask.is_deleted == 0)
    )
    is_pure = (tasks_count.scalar() or 0) == 0
    can_edit = (lg.responsible_id is not None and lg.responsible_id == user_id)

    return {
        "lg_id": lg.lg_id, "project_id": lg.project_id, "task_id": lg.task_id,
        "responsible_id": lg.responsible_id, "title": lg.title,
        "target_start": lg.target_start, "target_end": lg.target_end,
        "predecessor_end": lg.predecessor_end, "version": lg.version,
        "local_graph_state": lg.local_graph_state, "is_pure": is_pure,
        "can_edit": can_edit,
    }


@router.get("/{lg_id}/access")
async def get_access(lg_id: UUID, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    result = await db.execute(
        select(LocalGraph).where(LocalGraph.lg_id == lg_id, LocalGraph.is_deleted == 0)
    )
    lg = result.scalar_one_or_none()
    if not lg:
        raise HTTPException(404, "LocalGraph not found")
    can_edit = (lg.responsible_id is not None and lg.responsible_id == user_id)
    return {
        "lg_id": str(lg.lg_id), "can_edit": can_edit, "can_view": True,
        "responsible_id": lg.responsible_id,
        "local_graph_state": lg.local_graph_state, "is_approved": None,
    }


# ---------------- LgTask CRUD ----------------

@router.get("/{lg_id}/tasks")
async def get_lg_tasks(lg_id: UUID, db: AsyncSession = Depends(get_db)):
    result = await db.execute(
        select(LgTask).where(LgTask.lg_id == lg_id, LgTask.is_deleted == 0).order_by(LgTask.sort_order)
    )
    tasks = result.scalars().all()
    return {
        "lg_id": str(lg_id),
        "tasks": [
            {
                "lg_task_id": str(t.lg_task_id), "lg_id": str(t.lg_id),
                "title": t.title, "responsible_id": t.responsible_id, "status": t.status,
                "start_date": t.start_date.isoformat() if t.start_date else None,
                "end_date": t.end_date.isoformat() if t.end_date else None,
                "sort_order": t.sort_order, "version": t.version,
            } for t in tasks
        ],
    }


@router.post("/{lg_id}/tasks", response_model=LgTaskResponse, status_code=201)
async def create_lg_task(lg_id: UUID, data: LgTaskCreate, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    lg_res = await db.execute(select(LocalGraph).where(LocalGraph.lg_id == lg_id, LocalGraph.is_deleted == 0))
    lg = lg_res.scalar_one_or_none()
    if not lg:
        raise HTTPException(404, "LocalGraph not found")
    await _check_edit_access(lg, user_id)

    task = LgTask(
        lg_id=lg_id, title=data.title, responsible_id=data.responsible_id,
        start_date=data.start_date, end_date=data.end_date, sort_order=data.sort_order,
    )
    db.add(task)
    await db.flush()

    await _log_event(db, lg_id, LgEntityType.TASK, task.lg_task_id, LgEventType.CREATE,
                     new_value={"title": task.title, "start_date": str(task.start_date), "end_date": str(task.end_date)},
                     actor_id=user_id)

    await _recalc_state(lg, db)
    await db.commit()
    await db.refresh(task)

    await _broadcast_snapshot(lg_id, db)
    await _recalc_and_push(lg_id, db)
    return task


@router.put("/{lg_id}/tasks/{task_id}", response_model=LgTaskResponse)
async def update_lg_task(lg_id: UUID, task_id: UUID, data: LgTaskUpdate, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    lg_res = await db.execute(select(LocalGraph).where(LocalGraph.lg_id == lg_id, LocalGraph.is_deleted == 0))
    lg = lg_res.scalar_one_or_none()
    if not lg:
        raise HTTPException(404, "LocalGraph not found")
    await _check_edit_access(lg, user_id)

    result = await db.execute(select(LgTask).where(LgTask.lg_task_id == task_id, LgTask.lg_id == lg_id, LgTask.is_deleted == 0))
    task = result.scalar_one_or_none()
    if not task:
        raise HTTPException(404, "LgTask not found")

    changes = []
    if data.title is not None and task.title != data.title:
        changes.append(("title", task.title, data.title)); task.title = data.title
    if data.responsible_id is not None and task.responsible_id != data.responsible_id:
        changes.append(("responsible_id", task.responsible_id, data.responsible_id)); task.responsible_id = data.responsible_id
    if data.status is not None and task.status != data.status:
        changes.append(("status", task.status, data.status)); task.status = data.status
    if data.start_date is not None and task.start_date != data.start_date:
        changes.append(("start_date", str(task.start_date), str(data.start_date))); task.start_date = data.start_date
    if data.end_date is not None and task.end_date != data.end_date:
        changes.append(("end_date", str(task.end_date), str(data.end_date))); task.end_date = data.end_date
    if data.sort_order is not None and task.sort_order != data.sort_order:
        changes.append(("sort_order", task.sort_order, data.sort_order)); task.sort_order = data.sort_order
    task.version += 1

    for field, old, new in changes:
        await _log_event(db, lg_id, LgEntityType.TASK, task.lg_task_id, LgEventType.UPDATE,
                         field=field, old_value=old, new_value=new, actor_id=user_id)

    await _recalc_state(lg, db)
    await db.commit()
    await db.refresh(task)

    await _broadcast_snapshot(lg_id, db)
    if changes:
        await _recalc_and_push(lg_id, db)
    return task


@router.delete("/{lg_id}/tasks/{task_id}", status_code=204)
async def delete_lg_task(lg_id: UUID, task_id: UUID, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    lg_res = await db.execute(select(LocalGraph).where(LocalGraph.lg_id == lg_id, LocalGraph.is_deleted == 0))
    lg = lg_res.scalar_one_or_none()
    if not lg:
        raise HTTPException(404, "LocalGraph not found")
    await _check_edit_access(lg, user_id)

    result = await db.execute(select(LgTask).where(LgTask.lg_task_id == task_id, LgTask.lg_id == lg_id, LgTask.is_deleted == 0))
    task = result.scalar_one_or_none()
    if not task:
        raise HTTPException(404, "LgTask not found")

    task.is_deleted = 1
    task.deleted_at = func.now()
    task.deleted_by = user_id
    task.version += 1

    await _log_event(db, lg_id, LgEntityType.TASK, task.lg_task_id, LgEventType.DELETE,
                     old_value={"title": task.title}, actor_id=user_id)

    await _recalc_state(lg, db)
    await db.commit()

    await _broadcast_snapshot(lg_id, db)
    await _recalc_and_push(lg_id, db)
    return None


# ---------------- Зависимости ЛГ ----------------

@router.get("/{lg_id}/dependencies", response_model=LgDependencyListResponse)
async def get_lg_dependencies(lg_id: UUID, db: AsyncSession = Depends(get_db)):
    result = await db.execute(
        select(LgDependency).where(LgDependency.lg_id == lg_id, LgDependency.is_deleted == 0)
    )
    deps = result.scalars().all()
    return {
        "lg_id": lg_id,
        "dependencies": [
            {"dep_id": d.dep_id, "lg_id": d.lg_id,
             "predecessor_id": d.predecessor_id, "successor_id": d.successor_id,
             "dep_type": d.dep_type, "version": d.version}
            for d in deps
        ],
    }


@router.post("/{lg_id}/dependencies", response_model=LgDependencyResponse, status_code=201)
async def create_lg_dependency(lg_id: UUID, data: LgDependencyCreate, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    lg_res = await db.execute(select(LocalGraph).where(LocalGraph.lg_id == lg_id, LocalGraph.is_deleted == 0))
    lg = lg_res.scalar_one_or_none()
    if not lg:
        raise HTTPException(404, "LocalGraph not found")
    await _check_edit_access(lg, user_id)

    if data.predecessor_id == data.successor_id:
        raise HTTPException(400, "predecessor_id == successor_id")

    tasks_res = await db.execute(
        select(LgTask.lg_task_id).where(
            LgTask.lg_id == lg_id,
            LgTask.lg_task_id.in_([data.predecessor_id, data.successor_id]),
            LgTask.is_deleted == 0,
        )
    )
    found = set(t for t, in tasks_res.all())
    if len(found) != 2:
        raise HTTPException(400, "Task not found in this local graph")

    existing = await db.execute(
        select(LgDependency).where(
            LgDependency.lg_id == lg_id,
            LgDependency.predecessor_id == data.predecessor_id,
            LgDependency.successor_id == data.successor_id,
            LgDependency.is_deleted == 0,
        )
    )
    if existing.scalar_one_or_none():
        raise HTTPException(409, "Dependency already exists")

    dep = LgDependency(
        lg_id=lg_id,
        predecessor_id=data.predecessor_id,
        successor_id=data.successor_id,
        dep_type=data.dep_type,
    )
    db.add(dep)
    await db.flush()

    await _log_event(db, lg_id, LgEntityType.DEPENDENCY, dep.dep_id, LgEventType.CREATE,
                     new_value={"predecessor_id": str(data.predecessor_id),
                                "successor_id": str(data.successor_id)},
                     actor_id=user_id)

    await db.commit()
    await db.refresh(dep)

    await _broadcast_snapshot(lg_id, db)
    await _recalc_and_push(lg_id, db)
    return dep


@router.delete("/{lg_id}/dependencies/{dep_id}", status_code=204)
async def delete_lg_dependency(lg_id: UUID, dep_id: UUID, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    lg_res = await db.execute(select(LocalGraph).where(LocalGraph.lg_id == lg_id, LocalGraph.is_deleted == 0))
    lg = lg_res.scalar_one_or_none()
    if not lg:
        raise HTTPException(404, "LocalGraph not found")
    await _check_edit_access(lg, user_id)

    result = await db.execute(
        select(LgDependency).where(
            LgDependency.dep_id == dep_id,
            LgDependency.lg_id == lg_id,
            LgDependency.is_deleted == 0,
        )
    )
    dep = result.scalar_one_or_none()
    if not dep:
        raise HTTPException(404, "Dependency not found")

    dep.is_deleted = 1
    dep.deleted_at = func.now()
    dep.deleted_by = user_id

    await _log_event(db, lg_id, LgEntityType.DEPENDENCY, dep.dep_id, LgEventType.DELETE,
                     old_value={"predecessor_id": str(dep.predecessor_id),
                                "successor_id": str(dep.successor_id)},
                     actor_id=user_id)

    await db.commit()
    await _broadcast_snapshot(lg_id, db)
    return None

# ---------------- Завершение ЛГ (явное действие пользователя) ----------------

@router.post("/{lg_id}/complete")
async def complete_local_graph(lg_id: UUID, data: LgCompleteRequest, request: Request, db: AsyncSession = Depends(get_db)):
    """
    Явное завершение МГ-задачи через ЛГ.
    Условия:
      - пользователь — ответственный (право редактирования ЛГ);
      - в ЛГ есть хотя бы одна задача со сроками;
      - нет задач без сроков (start_date/end_date null);
      - все задачи со сроками имеют status = 1 (Completed);
      - МГ-задача ещё не завершена (в BUS нет флага, проверяем через _recalc_and_push).
    Действие:
      - _compute_forecast (max из ЛГ) + status = 1;
      - POST /internal/forecast-update в MG (status = 1, local_graph_state = ATTACHED);
      - broadcast WS snapshot.
    """
    user_id = _user_id(request)
    lg_res = await db.execute(select(LocalGraph).where(LocalGraph.lg_id == lg_id, LocalGraph.is_deleted == 0))
    lg = lg_res.scalar_one_or_none()
    if not lg:
        raise HTTPException(404, "LocalGraph not found")
    await _check_edit_access(lg, user_id)

    # Все активные задачи ЛГ
    tasks_res = await db.execute(
        select(LgTask).where(LgTask.lg_id == lg_id, LgTask.is_deleted == 0)
    )
    tasks = tasks_res.scalars().all()

    if not tasks:
        raise HTTPException(400, "В ЛГ нет задач")

    # Проверка: нет задач без сроков
    tasks_without_dates = [t for t in tasks if not t.start_date or not t.end_date]
    if tasks_without_dates:
        raise HTTPException(
            400,
            f"В ЛГ есть задачи без сроков ({len(tasks_without_dates)}). "
            f"Завершение невозможно."
        )

    # Проверка: все задачи Completed
    not_completed = [t for t in tasks if t.status != 1]
    if not_completed:
        raise HTTPException(
            400,
            f"Не все задачи ЛГ завершены ({len(not_completed)} из {len(tasks)}). "
            f"Завершение невозможно."
        )

    # Вычисляем forecast (max из ЛГ)
    forecast = await _compute_forecast(lg, db)
    # Принудительно status = 1 (явное завершение)
    forecast.status = 1

    # Логируем событие завершения
    await _log_event(
        db, lg_id, LgEntityType.LOCAL_GRAPH, lg.lg_id, LgEventType.STATUS_CHANGE,
        new_value={"status": "completed", "reason": data.reason},
        reason=data.reason,
        actor_id=user_id,
    )

    await db.commit()

    # Push в MG с status = 1
    payload = {
        "project_id": lg.project_id,
        "task_id": str(lg.task_id),
        "forecast_start": forecast.forecast_start.isoformat() if forecast.forecast_start else None,
        "forecast_end": forecast.forecast_end.isoformat() if forecast.forecast_end else None,
        "progress_current": forecast.progress_current,
        "progress_total": forecast.progress_total,
        "status": 1,  # ← явное завершение
        "local_graph_state": lg.local_graph_state,
    }
    try:
        async with aiohttp.ClientSession(timeout=aiohttp.ClientTimeout(total=3.0)) as s:
            async with s.post(f"{MG_SERVICE_URL}/internal/forecast-update", json=payload) as resp:
                if resp.status == 200:
                    logger.info(f"complete_local_graph: pushed to MG, task={lg.task_id}")
                else:
                    logger.warning(f"complete_local_graph: push to MG HTTP {resp.status}")
    except Exception as e:
        logger.warning(f"complete_local_graph: push to MG: {e}")

    await _broadcast_snapshot(lg_id, db)

    return {
        "lg_id": str(lg_id),
        "status": "completed",
        "forecast_start": forecast.forecast_start.isoformat() if forecast.forecast_start else None,
        "forecast_end": forecast.forecast_end.isoformat() if forecast.forecast_end else None,
        "progress_current": forecast.progress_current,
        "progress_total": forecast.progress_total,
    }

# ---------------- Пересчёт прогноза ----------------

@router.post("/{lg_id}/recalc", response_model=ForecastRecalcResponse)
async def recalc_lg(lg_id: UUID, db: AsyncSession = Depends(get_db)):
    result = await db.execute(select(LocalGraph).where(LocalGraph.lg_id == lg_id, LocalGraph.is_deleted == 0))
    lg = result.scalar_one_or_none()
    if not lg:
        raise HTTPException(404, "LocalGraph not found")
    return await _recalc_and_push(lg_id, db)


@router.post("/forecast-recalc", response_model=ForecastRecalcResponse)
async def forecast_recalc(data: ForecastRecalcRequest, db: AsyncSession = Depends(get_db)):
    result = await db.execute(select(LocalGraph).where(LocalGraph.task_id == data.task_id, LocalGraph.is_deleted == 0))
    lg = result.scalar_one_or_none()
    if not lg:
        raise HTTPException(404, "LocalGraph not found")
    if data.predecessor_end:
        lg.predecessor_end = data.predecessor_end
    forecast = await _compute_forecast(lg, db)
    await db.commit()
    await _broadcast_snapshot(lg.lg_id, db)
    return forecast


async def _compute_forecast(lg: LocalGraph, db: AsyncSession) -> ForecastRecalcResponse:
    result = await db.execute(select(LgTask).where(LgTask.lg_id == lg.lg_id, LgTask.is_deleted == 0))
    tasks = result.scalars().all()
    if tasks:
        starts = [t.start_date for t in tasks if t.start_date]
        ends = [t.end_date for t in tasks if t.end_date]
        completed = sum(1 for t in tasks if t.status == 1)
        forecast_start = min(starts) if starts else lg.target_start
        forecast_end = max(ends) if ends else lg.target_end
        progress_current = completed
        progress_total = len(tasks)
        # Завершение МГ-задачи — только явное действие пользователя
        # (кнопка «Завершить» в ЛГ → отдельный endpoint POST /{lg_id}/complete).
        status = 0
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
        forecast_start=forecast_start, forecast_end=forecast_end,
        progress_current=progress_current, progress_total=progress_total, status=status,
    )


async def _recalc_and_push(lg_id: UUID, db: AsyncSession):
    result = await db.execute(select(LocalGraph).where(LocalGraph.lg_id == lg_id, LocalGraph.is_deleted == 0))
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
        "local_graph_state": lg.local_graph_state,
    }
    try:
        async with aiohttp.ClientSession(timeout=aiohttp.ClientTimeout(total=3.0)) as s:
            async with s.post(f"{MG_SERVICE_URL}/internal/forecast-update", json=payload) as resp:
                if resp.status == 200:
                    logger.info(f"pushed forecast to MG: task={lg.task_id}, state={lg.local_graph_state}")
                else:
                    logger.warning(f"push forecast to MG: HTTP {resp.status}")
    except Exception as e:
        logger.warning(f"push forecast to MG: {e}")


# ---------------- Diff ----------------

@router.put("/{lg_id}/diff")
async def apply_diff(lg_id: UUID, payload: dict, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    lg_res = await db.execute(select(LocalGraph).where(LocalGraph.lg_id == lg_id, LocalGraph.is_deleted == 0))
    lg = lg_res.scalar_one_or_none()
    if not lg:
        raise HTTPException(404, "LocalGraph not found")
    await _check_edit_access(lg, user_id)

    changes = payload.get("changes", [])
    applied = 0
    errors = []

    for ch in changes:
        try:
            entity = ch.get("entity")
            op = ch.get("op")
            eid = ch.get("id")
            fields = ch.get("fields", {})
            reason = ch.get("reason")
            if entity == "task" and op == "update":
                result = await db.execute(select(LgTask).where(LgTask.lg_task_id == eid, LgTask.lg_id == lg_id, LgTask.is_deleted == 0))
                task = result.scalar_one_or_none()
                if not task:
                    errors.append({"id": eid, "error": "not_found"}); continue
                for k, v in fields.items():
                    if k in ("start_date", "end_date") and v:
                        setattr(task, k, date.fromisoformat(v))
                    elif k in ("title", "status", "sort_order", "responsible_id"):
                        setattr(task, k, v)
                task.version += 1
                await _log_event(db, lg_id, LgEntityType.TASK, task.lg_task_id, LgEventType.UPDATE,
                                 field=", ".join(fields.keys()), new_value=fields, reason=reason, actor_id=user_id)
                applied += 1
        except Exception as e:
            errors.append({"id": ch.get("id"), "error": str(e)})

    await _recalc_state(lg, db)
    await db.commit()

    await _broadcast_snapshot(lg_id, db)
    if applied > 0:
        await _recalc_and_push(lg_id, db)
    return {"status": "ok", "applied": applied, "errors": errors}
