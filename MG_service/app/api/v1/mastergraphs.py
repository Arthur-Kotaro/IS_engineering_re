# app/api/v1/mastergraphs.py
import logging
from fastapi import APIRouter, Depends, HTTPException, Request
from typing import List, Optional
from uuid import UUID
from pydantic import BaseModel
from sqlalchemy.ext.asyncio import AsyncSession

from app.database import get_db
from app.repositories.mastergraph_repo import MastergraphRepository
from app.schemas.mastergraph import (
    MastergraphResponse, MastergraphDetailResponse,
    MastergraphCreate, CanEditResponse,
)
from app.services.access_client import check_project_access, list_accessible_projects
from app.services.bus_client import init_local_graph, get_bus_url_for_group, get_local_graph_state
from app.services.user_client import get_user_name

logger = logging.getLogger(__name__)
router = APIRouter(prefix="/mastergraphs", tags=["Mastergraphs"])


def _user_id(request: Request) -> int:
    uid = request.headers.get("X-User-ID")
    if not uid:
        raise HTTPException(401, "Missing X-User-ID")
    return int(uid)


def _token(request: Request) -> Optional[str]:
    auth = request.headers.get("Authorization")
    return auth.removeprefix("Bearer ").strip() if auth else None


class SetResponsibleRequest(BaseModel):
    responsible_id: int


async def _build_detail_response(mg, tasks, deps, groups) -> dict:
    enriched_tasks = []
    for t in tasks:
        resp_name = None
        if t.responsible_id:
            resp_name = await get_user_name(t.responsible_id)

        bus_url = get_bus_url_for_group(t.group_id)
        lg_state = await get_local_graph_state(bus_url, str(t.task_id)) if bus_url else 0

        enriched_tasks.append({
            "task_id": t.task_id,
            "mg_id": t.mg_id,
            "group_id": t.group_id,
            "parent_task_id": t.parent_task_id,
            "title": t.title,
            "responsible_id": t.responsible_id,
            "responsible_name": resp_name,
            "local_graph_state": lg_state,
            "status": t.status,
            "start_date": t.start_date,
            "end_date": t.end_date,
            "forecast_start": t.forecast_start,
            "forecast_end": t.forecast_end,
            "progress_current": t.progress_current,
            "progress_total": t.progress_total,
            "sort_order": t.sort_order,
            "version": t.version,
        })

    return {
        "mg_id": mg.mg_id,
        "project_id": mg.project_id,
        "name": mg.name,
        "version": mg.version,
        "created_by": mg.created_by,
        "created_at": mg.created_at,
        "updated_at": mg.updated_at,
        "tasks": enriched_tasks,
        "dependencies": deps,
        "groups": groups,
    }


@router.get("", response_model=List[MastergraphResponse])
async def list_mastergraphs(
    request: Request,
    permission: str = "view_project",
    db: AsyncSession = Depends(get_db),
):
    user_id = _user_id(request)
    token = _token(request)

    accessible = await list_accessible_projects(user_id, permission, token)
    project_ids = [p["project_id"] for p in accessible]
    if not project_ids:
        return []

    repo = MastergraphRepository(db)
    return await repo.list_by_project_ids(project_ids)


@router.get("/{mg_id}", response_model=MastergraphDetailResponse)
async def get_mastergraph(
    mg_id: UUID,
    request: Request,
    db: AsyncSession = Depends(get_db),
):
    user_id = _user_id(request)
    token = _token(request)

    repo = MastergraphRepository(db)
    mg = await repo.get_by_id(mg_id)
    if not mg:
        raise HTTPException(404, "Mastergraph not found")

    access = await check_project_access(mg.project_id, user_id, "view_project", token)
    if not access.get("has_access"):
        raise HTTPException(403, "Access denied")

    tasks = await repo.get_tasks(mg_id)
    deps = await repo.get_dependencies(mg_id)
    groups = await repo.get_groups_for_mg(mg_id)

    return await _build_detail_response(mg, tasks, deps, groups)


@router.get("/{mg_id}/access", response_model=CanEditResponse)
async def check_mg_access(
    mg_id: UUID,
    request: Request,
    db: AsyncSession = Depends(get_db),
):
    user_id = _user_id(request)
    token = _token(request)

    repo = MastergraphRepository(db)
    mg = await repo.get_by_id(mg_id)
    if not mg:
        raise HTTPException(404, "Mastergraph not found")

    edit = await check_project_access(mg.project_id, user_id, "edit_mastergraphic", token)
    view = await check_project_access(mg.project_id, user_id, "view_project", token)

    return {
        "can_edit": bool(edit.get("has_access")),
        "can_view": bool(view.get("has_access")),
        "reason": edit.get("reason"),
    }


@router.post("", response_model=MastergraphResponse, status_code=201)
async def create_mastergraph(
    data: MastergraphCreate,
    request: Request,
    db: AsyncSession = Depends(get_db),
):
    user_id = _user_id(request)
    token = _token(request)

    access = await check_project_access(data.project_id, user_id, "edit_mastergraphic", token)
    if not access.get("has_access"):
        raise HTTPException(403, "Access denied")

    repo = MastergraphRepository(db)
    existing = await repo.get_by_project_id(data.project_id)
    if existing:
        raise HTTPException(409, "Mastergraph for this project already exists")

    return await repo.create(data.project_id, data.name, user_id)


@router.put("/{mg_id}/tasks/{task_id}/responsible", response_model=MastergraphDetailResponse)
async def set_responsible(
    mg_id: UUID,
    task_id: UUID,
    data: SetResponsibleRequest,
    request: Request,
    db: AsyncSession = Depends(get_db),
):
    user_id = _user_id(request)
    token = _token(request)

    repo = MastergraphRepository(db)
    mg = await repo.get_by_id(mg_id)
    if not mg:
        raise HTTPException(404, "Mastergraph not found")

    access = await check_project_access(mg.project_id, user_id, "edit_mastergraphic", token)
    if not access.get("has_access"):
        raise HTTPException(403, "Access denied")

    tasks = await repo.get_tasks(mg_id)
    task = next((t for t in tasks if str(t.task_id) == str(task_id)), None)
    if not task:
        raise HTTPException(404, "Task not found")

    task.responsible_id = data.responsible_id
    task.version += 1
    await db.commit()
    await db.refresh(task)

    bus_url = get_bus_url_for_group(task.group_id)
    if bus_url:
        deps = await repo.get_dependencies(mg_id)
        predecessors = [d for d in deps if str(d.successor_id) == str(task_id)]
        predecessor_end = None
        if predecessors:
            pred_id = predecessors[0].predecessor_id
            pred_task = next((t for t in tasks if str(t.task_id) == str(pred_id)), None)
            if pred_task and pred_task.end_date:
                predecessor_end = pred_task.end_date.isoformat()

        payload = {
            "project_id": mg.project_id,
            "task_id": str(task.task_id),
            "responsible_id": data.responsible_id,
            "title": task.title,
            "target_start": task.start_date.isoformat() if task.start_date else None,
            "target_end": task.end_date.isoformat() if task.end_date else None,
            "predecessor_end": predecessor_end,
        }
        result = await init_local_graph(bus_url, payload)
        if result:
            logger.info(f"init_local_graph OK: lg_id={result.get('lg_id')}")
        else:
            logger.warning(f"init_local_graph FAILED for task={task_id}")

    return await _build_detail_response(
        mg,
        await repo.get_tasks(mg_id),
        await repo.get_dependencies(mg_id),
        await repo.get_groups_for_mg(mg_id),
    )
