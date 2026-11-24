# app/api/v1/mastergraphs.py
import logging
from datetime import date
from fastapi import APIRouter, Depends, HTTPException, Request, Query
from typing import List, Optional
from uuid import UUID
from pydantic import BaseModel
from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession

from app.database import get_db
from app.repositories.mastergraph_repo import MastergraphRepository
from app.repositories.milestone_repo import MilestoneRepository
from app.repositories.mg_group_repo import MgGroupRepository
from app.repositories.event_repo import EventRepository
from app.schemas.mastergraph import (
    MastergraphResponse, MastergraphDetailResponse,
    MastergraphCreate, CanEditResponse,
    BatchCreateRequest, BatchCreateResponse, BatchTaskCreated,
    ConflictResponse, ConflictItem,
    CreateGroupsRequest, CreateCustomGroupRequest, RenameGroupRequest,
)
from app.services.access_client import check_project_access, list_accessible_projects
from app.services.bus_client import init_local_graph, get_bus_url_for_group, get_local_graph_state
from app.services.user_client import get_user_name
from app.services import mastergraph_service as mgsvc

logger = logging.getLogger(__name__)
# Кэш local_graph_state в памяти MG: task_id -> state
_lg_state_cache: dict[str, int] = {}
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


class ApproveRequest(BaseModel):
    reason: Optional[str] = None


class UpdateMilestoneRequest(BaseModel):
    planned_date: Optional[str] = None
    reason: Optional[str] = None


class CompleteMilestoneRequest(BaseModel):
    actual_date: Optional[str] = None


class UpdateTaskRequest(BaseModel):
    title: Optional[str] = None
    responsible_id: Optional[int] = None
    status: Optional[int] = None
    start_date: Optional[str] = None
    end_date: Optional[str] = None
    progress_current: Optional[int] = None
    progress_total: Optional[int] = None
    sort_order: Optional[int] = None
    comment: Optional[str] = None
    reason: Optional[str] = None


class CreateTaskRequest(BaseModel):
    group_id: str
    title: str
    responsible_id: Optional[int] = None
    start_date: Optional[str] = None
    end_date: Optional[str] = None
    sort_order: int = 0
    parent_task_id: Optional[str] = None
    comment: Optional[str] = None


class CreateDependencyRequest(BaseModel):
    predecessor_id: str
    successor_id: str


async def _build_detail_response(db, mg, tasks, deps, groups) -> dict:
    repo = MastergraphRepository(db)
    date_history = await repo.get_date_history(mg.mg_id)

    enriched_tasks = []
    for t in tasks:
        resp_name = None
        if t.responsible_id:
            resp_name = await get_user_name(t.responsible_id)
        bus_url = get_bus_url_for_group(t.group_id)
        task_key = str(t.task_id)
        lg_state_raw = await get_local_graph_state(bus_url, task_key) if bus_url else 0
        if lg_state_raw is None:
            # BUS недоступен — берём из кэша, если есть; иначе 0
            lg_state = _lg_state_cache.get(task_key, 0)
        else:
            lg_state = lg_state_raw
            _lg_state_cache[task_key] = lg_state
        enriched_tasks.append({
            "task_id": t.task_id, "mg_id": t.mg_id, "group_id": t.group_id,
            "parent_task_id": t.parent_task_id, "title": t.title,
            "responsible_id": t.responsible_id, "responsible_name": resp_name,
            "local_graph_state": lg_state, "status": t.status,
            "start_date": t.start_date, "end_date": t.end_date,
            "forecast_start": t.forecast_start, "forecast_end": t.forecast_end,
            "progress_current": t.progress_current, "progress_total": t.progress_total,
            "sort_order": t.sort_order, "comment": t.comment, "version": t.version,
            "date_history": date_history.get(str(t.task_id), []),
        })

    ms_repo = MilestoneRepository(db)
    milestones = await ms_repo.list_by_mg(mg.mg_id)
    is_pure = await repo.is_pure(mg.mg_id)

    return {
        "mg_id": mg.mg_id, "project_id": mg.project_id, "name": mg.name,
        "version": mg.version, "is_approved": mg.is_approved, "is_pure": is_pure,
        "typology_name": mg.typology_name,
        "created_by": mg.created_by, "created_at": mg.created_at, "updated_at": mg.updated_at,
        "tasks": enriched_tasks, "dependencies": deps,
        "milestones": [
            {"milestone_id": m.milestone_id, "mg_id": m.mg_id,
             "abbreviation": m.abbreviation, "full_name": m.full_name,
             "tooltip": m.tooltip, "week_offset": m.week_offset,
             "planned_date": m.planned_date, "actual_date": m.actual_date,
             "status": m.status, "sort_order": m.sort_order, "version": m.version}
            for m in milestones
        ],
        "groups": groups,
    }


@router.get("", response_model=List[MastergraphResponse])
async def list_mastergraphs(request: Request, permission: str = "view_project", db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    token = _token(request)
    accessible = await list_accessible_projects(user_id, permission, token)
    project_ids = [p["project_id"] for p in accessible]
    if not project_ids:
        return []
    repo = MastergraphRepository(db)
    mss = await repo.list_by_project_ids(project_ids)
    result = []
    for mg in mss:
        is_pure = await repo.is_pure(mg.mg_id)
        result.append({
            "mg_id": mg.mg_id, "project_id": mg.project_id, "name": mg.name,
            "version": mg.version, "is_approved": mg.is_approved, "is_pure": is_pure,
            "typology_name": mg.typology_name, "created_by": mg.created_by,
            "created_at": mg.created_at, "updated_at": mg.updated_at,
        })
    return result


@router.get("/{mg_id}", response_model=MastergraphDetailResponse)
async def get_mastergraph(mg_id: UUID, request: Request, db: AsyncSession = Depends(get_db)):
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
    return await _build_detail_response(db, mg, tasks, deps, groups)


@router.get("/{mg_id}/access", response_model=CanEditResponse)
async def check_mg_access(mg_id: UUID, request: Request, db: AsyncSession = Depends(get_db)):
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
        "is_approved": mg.is_approved,
    }


@router.post("", response_model=MastergraphResponse, status_code=201)
async def create_mastergraph_legacy(data: MastergraphCreate, request: Request, db: AsyncSession = Depends(get_db)):
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


# ---------------- Статус МГ ----------------

@router.put("/{mg_id}/status")
async def approve_mastergraph(mg_id: UUID, data: ApproveRequest, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    token = _token(request)
    try:
        mg = await mgsvc.approve_mastergraph(db, mg_id, user_id, token, data.reason)
    except mgsvc.NotCveError as e:
        raise HTTPException(403, str(e))
    except mgsvc.AlreadyApprovedError as e:
        raise HTTPException(409, str(e))
    except mgsvc.FirstMilestoneCompletedError as e:
        raise HTTPException(400, str(e))
    except mgsvc.MastergraphError as e:
        raise HTTPException(400, str(e))
    return {"mg_id": str(mg.mg_id), "is_approved": mg.is_approved, "approved_at": mg.approved_at, "approved_by": mg.approved_by}


# ---------------- Вехи ----------------

@router.put("/{mg_id}/milestones/{milestone_id}")
async def update_milestone(mg_id: UUID, milestone_id: UUID, data: UpdateMilestoneRequest, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    token = _token(request)
    new_date = date.fromisoformat(data.planned_date) if data.planned_date else None
    try:
        ms = await mgsvc.update_milestone(db, mg_id, milestone_id, user_id, token, new_date, data.reason)
    except mgsvc.ReasonRequiredError as e:
        raise HTTPException(400, str(e))
    except mgsvc.MastergraphError as e:
        raise HTTPException(400, str(e))
    return {"milestone_id": str(ms.milestone_id), "planned_date": ms.planned_date, "status": ms.status, "version": ms.version}


@router.put("/{mg_id}/milestones/{milestone_id}/complete")
async def complete_milestone(mg_id: UUID, milestone_id: UUID, data: CompleteMilestoneRequest, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    token = _token(request)
    actual = date.fromisoformat(data.actual_date) if data.actual_date else None
    try:
        ms = await mgsvc.complete_milestone(db, mg_id, milestone_id, user_id, token, actual)
    except mgsvc.DraftModeError as e:
        raise HTTPException(403, str(e))
    except mgsvc.MastergraphError as e:
        raise HTTPException(400, str(e))
    return {"milestone_id": str(ms.milestone_id), "status": ms.status, "actual_date": ms.actual_date, "version": ms.version}


# ---------------- Задачи ----------------

@router.post("/{mg_id}/tasks", status_code=201)
async def create_task(mg_id: UUID, data: CreateTaskRequest, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    token = _token(request)
    try:
        task = await mgsvc.create_task(
            db, mg_id, user_id, token,
            group_id=data.group_id, title=data.title,
            responsible_id=data.responsible_id,
            start_date=date.fromisoformat(data.start_date) if data.start_date else None,
            end_date=date.fromisoformat(data.end_date) if data.end_date else None,
            sort_order=data.sort_order,
            parent_task_id=UUID(data.parent_task_id) if data.parent_task_id else None,
            comment=data.comment,
        )
    except mgsvc.TaskTitleConflictError as e:
        raise HTTPException(409, {"detail": "task_title_conflict", "conflicts": [
            {"title": t, "group_id": g} for g, t in e.conflicts
        ]})
    except mgsvc.MastergraphError as e:
        raise HTTPException(400, str(e))

    repo = MastergraphRepository(db)
    mg = await repo.get_by_id(mg_id)
    if task.responsible_id and mg:
        bus_url = get_bus_url_for_group(task.group_id)
        if bus_url:
            payload = {
                "project_id": mg.project_id, "task_id": str(task.task_id),
                "responsible_id": task.responsible_id, "title": task.title,
                "target_start": task.start_date.isoformat() if task.start_date else None,
                "target_end": task.end_date.isoformat() if task.end_date else None,
                "predecessor_end": None,
            }
            await init_local_graph(bus_url, payload)

    return {"task_id": str(task.task_id), "version": task.version}


@router.post("/{mg_id}/tasks/batch", response_model=BatchCreateResponse, status_code=201)
async def create_tasks_batch(mg_id: UUID, data: BatchCreateRequest, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    token = _token(request)

    tasks_data = []
    for t in data.tasks:
        tasks_data.append({
            "batch_id": t.batch_id,
            "group_id": t.group_id,
            "title": t.title,
            "responsible_id": t.responsible_id,
            "start_date": date.fromisoformat(t.start_date) if t.start_date else None,
            "end_date": date.fromisoformat(t.end_date) if t.end_date else None,
            "sort_order": t.sort_order,
            "comment": t.comment,
        })
    deps_data = [{"predecessor_batch_id": d.predecessor_batch_id,
                  "successor_batch_id": d.successor_batch_id} for d in data.dependencies]

    try:
        created_tasks, created_deps = await mgsvc.create_tasks_batch(
            db, mg_id, user_id, token, tasks_data, deps_data
        )
    except mgsvc.TaskTitleConflictError as e:
        raise HTTPException(409, {"detail": "task_title_conflict", "conflicts": [
            {"title": t, "group_id": g} for g, t in e.conflicts
        ]})
    except mgsvc.MastergraphError as e:
        raise HTTPException(400, str(e))

    return {
        "tasks": [{"batch_id": t["batch_id"] if isinstance(t, dict) else "",
                   "task_id": task.task_id}
                  for t, task in zip(tasks_data, created_tasks)],
        "dependencies": [dep.dep_id for dep in created_deps],
    }


@router.put("/{mg_id}/tasks/{task_id}")
async def update_task(mg_id: UUID, task_id: UUID, data: UpdateTaskRequest, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    token = _token(request)
    fields = {}
    if data.title is not None: fields["title"] = data.title
    if data.responsible_id is not None: fields["responsible_id"] = data.responsible_id
    if data.status is not None: fields["status"] = data.status
    if data.start_date is not None: fields["start_date"] = date.fromisoformat(data.start_date) if data.start_date else None
    if data.end_date is not None: fields["end_date"] = date.fromisoformat(data.end_date) if data.end_date else None
    if data.progress_current is not None: fields["progress_current"] = data.progress_current
    if data.progress_total is not None: fields["progress_total"] = data.progress_total
    if data.sort_order is not None: fields["sort_order"] = data.sort_order
    if data.comment is not None: fields["comment"] = data.comment
    try:
        task = await mgsvc.update_task(db, mg_id, task_id, user_id, token, fields, data.reason)
    except mgsvc.ReasonRequiredError as e:
        raise HTTPException(400, str(e))
    except mgsvc.MastergraphError as e:
        raise HTTPException(400, str(e))
    return {"task_id": str(task.task_id), "version": task.version}


@router.delete("/{mg_id}/tasks/{task_id}", status_code=204)
async def delete_task(mg_id: UUID, task_id: UUID, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    token = _token(request)
    repo = MastergraphRepository(db)
    mg = await repo.get_by_id(mg_id)
    if not mg:
        raise HTTPException(404, "Mastergraph not found")
    access = await check_project_access(mg.project_id, user_id, "edit_mastergraphic", token)
    if not access.get("has_access"):
        raise HTTPException(403, "Access denied")
    from app.models.mg_task import MgTask
    from app.models.mg_event import MgEntityType, MgEventType
    from app.repositories.event_repo import EventRepository
    from sqlalchemy.sql import func
    result = await db.execute(
        select(MgTask).where(MgTask.task_id == task_id, MgTask.mg_id == mg_id, MgTask.is_deleted == False)
    )
    task = result.scalar_one_or_none()
    if not task:
        raise HTTPException(404, "Task not found")
    task.is_deleted = True
    task.deleted_at = func.now()
    task.deleted_by = user_id
    task.version += 1
    event_repo = EventRepository(db)
    await event_repo.log(
        mg_id=mg_id, entity_type=MgEntityType.TASK, entity_id=task.task_id,
        event_type=MgEventType.DELETE, old_value={"title": task.title}, actor_id=user_id,
    )
    await db.commit()
    return None


@router.put("/{mg_id}/tasks/{task_id}/responsible", response_model=MastergraphDetailResponse)
async def set_responsible(mg_id: UUID, task_id: UUID, data: SetResponsibleRequest, request: Request, db: AsyncSession = Depends(get_db)):
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
            "project_id": mg.project_id, "task_id": str(task.task_id),
            "responsible_id": data.responsible_id, "title": task.title,
            "target_start": task.start_date.isoformat() if task.start_date else None,
            "target_end": task.end_date.isoformat() if task.end_date else None,
            "predecessor_end": predecessor_end,
        }
        result = await init_local_graph(bus_url, payload)
        if result:
            logger.info(f"init_local_graph OK: lg_id={result.get('lg_id')}")

    return await _build_detail_response(
        db, mg,
        await repo.get_tasks(mg_id),
        await repo.get_dependencies(mg_id),
        await repo.get_groups_for_mg(mg_id),
    )


# ---------------- Зависимости ----------------

@router.post("/{mg_id}/dependencies", status_code=201)
async def create_dependency(mg_id: UUID, data: CreateDependencyRequest, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    token = _token(request)
    try:
        dep = await mgsvc.create_dependency(
            db, mg_id, user_id, token,
            predecessor_id=UUID(data.predecessor_id),
            successor_id=UUID(data.successor_id),
        )
    except mgsvc.MastergraphError as e:
        raise HTTPException(400, str(e))
    return {"dep_id": str(dep.dep_id), "version": dep.version}


@router.delete("/{mg_id}/dependencies/{dep_id}", status_code=204)
async def delete_dependency(mg_id: UUID, dep_id: UUID, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    token = _token(request)
    try:
        await mgsvc.delete_dependency(db, mg_id, dep_id, user_id, token)
    except mgsvc.MastergraphError as e:
        raise HTTPException(400, str(e))
    return None


# ---------------- Группы МГ ----------------

@router.get("/{mg_id}/default-groups")
async def get_default_groups(mg_id: UUID, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    token = _token(request)
    repo = MastergraphRepository(db)
    mg = await repo.get_by_id(mg_id)
    if not mg:
        raise HTTPException(404, "Mastergraph not found")
    access = await check_project_access(mg.project_id, user_id, "view_project", token)
    if not access.get("has_access"):
        raise HTTPException(403, "Access denied")

    from app.models.task_group import TaskGroup
    from app.config import load_bus_instances
    bus_instances = load_bus_instances()
    result = await db.execute(
        select(TaskGroup).where(TaskGroup.is_deleted == False).order_by(TaskGroup.group_id)
    )
    groups = result.scalars().all()
    return [
        {"group_id": g.group_id, "name": g.name, "description": g.description,
         "bus_instance_id": g.bus_instance_id,
         "bus_url": bus_instances.get(g.group_id, {}).get("url")}
        for g in groups
    ]


@router.post("/{mg_id}/groups")
async def create_mg_groups(mg_id: UUID, data: CreateGroupsRequest, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    token = _token(request)
    repo = MastergraphRepository(db)
    mg = await repo.get_by_id(mg_id)
    if not mg:
        raise HTTPException(404, "Mastergraph not found")
    access = await check_project_access(mg.project_id, user_id, "edit_mastergraphic", token)
    if not access.get("has_access"):
        raise HTTPException(403, "Access denied")

    mg_repo = MgGroupRepository(db)
    n = len(data.group_ids)

    if data.after_group_id:
        after_order = await mg_repo.get_sort_order(mg_id, data.after_group_id)
        if after_order is None:
            logger.warning(
                f"create_mg_groups: after_group_id {data.after_group_id!r} not found in mg {mg_id}, "
                f"falling back to position={data.position!r}"
            )
            if data.position == "top":
                base = await repo.min_sort_order(mg_id) - (n - 1)
            else:
                base = await repo.next_sort_order(mg_id)
        else:
            await mg_repo.shift_sort_orders_after(mg_id, after_order, n)
            if data.position == "top":
                base = after_order
            else:
                base = after_order + 1
    else:
        if data.position == "top":
            base = await repo.min_sort_order(mg_id) - (n - 1)
        else:
            base = await repo.next_sort_order(mg_id)

    created = []
    for i, gid in enumerate(data.group_ids):
        mg_g = await mg_repo.add(mg_id, gid, sort_order=base + i)
        if mg_g:
            created.append(gid)
    await db.commit()

    groups = await repo.get_groups_for_mg(mg_id)
    is_pure = await repo.is_pure(mg_id)
    return {"created": created, "groups": groups, "is_pure": is_pure}


@router.post("/{mg_id}/groups/custom")
async def create_custom_group(mg_id: UUID, data: CreateCustomGroupRequest, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    token = _token(request)
    repo = MastergraphRepository(db)
    mg = await repo.get_by_id(mg_id)
    if not mg:
        raise HTTPException(404, "Mastergraph not found")
    access = await check_project_access(mg.project_id, user_id, "edit_mastergraphic", token)
    if not access.get("has_access"):
        raise HTTPException(403, "Access denied")

    from app.models.task_group import TaskGroup
    import re, uuid as _uuid
    slug = re.sub(r'[^a-zA-Z0-9_]+', '_', data.name.lower()).strip('_')[:30] or "custom"
    group_id = f"{slug}_{_uuid.uuid4().hex[:6]}"

    tg = TaskGroup(group_id=group_id, name=data.name, description=data.description,
                   bus_instance_id="other_bus")
    db.add(tg)
    await db.flush()

    mg_repo = MgGroupRepository(db)
    if data.after_group_id:
        after_order = await mg_repo.get_sort_order(mg_id, data.after_group_id)
        if after_order is None:
            logger.warning(
                f"create_custom_group: after_group_id {data.after_group_id!r} not found in mg {mg_id}, "
                f"falling back to position={data.position!r}"
            )
            if data.position == "top":
                sort_order = await repo.min_sort_order(mg_id)
            else:
                sort_order = await repo.next_sort_order(mg_id)
        else:
            await mg_repo.shift_sort_orders_after(mg_id, after_order, 1)
            if data.position == "top":
                sort_order = after_order
            else:
                sort_order = after_order + 1
    else:
        if data.position == "top":
            sort_order = await repo.min_sort_order(mg_id)
        else:
            sort_order = await repo.next_sort_order(mg_id)

    mg_g = await mg_repo.add(mg_id, group_id, sort_order=sort_order)
    if mg_g is None:
        # Атомарность: не оставляем «сироту» в task_groups
        await db.rollback()
        raise HTTPException(500, f"Failed to attach group '{group_id}' to mastergraph")
    await db.commit()

    groups = await repo.get_groups_for_mg(mg_id)
    is_pure = await repo.is_pure(mg_id)
    return {"group_id": group_id, "groups": groups, "is_pure": is_pure}


@router.put("/{mg_id}/groups/{group_id}/rename")
async def rename_group(mg_id: UUID, group_id: str, data: RenameGroupRequest, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    token = _token(request)
    repo = MastergraphRepository(db)
    mg = await repo.get_by_id(mg_id)
    if not mg:
        raise HTTPException(404, "Mastergraph not found")
    access = await check_project_access(mg.project_id, user_id, "edit_mastergraphic", token)
    if not access.get("has_access"):
        raise HTTPException(403, "Access denied")

    from app.models.task_group import TaskGroup
    tg_res = await db.execute(
        select(TaskGroup).where(TaskGroup.group_id == group_id, TaskGroup.is_deleted == False)
    )
    tg = tg_res.scalar_one_or_none()
    if not tg:
        raise HTTPException(404, "Group not found")

    tg.name = data.name
    await db.commit()

    groups = await repo.get_groups_for_mg(mg_id)
    return {"group_id": group_id, "name": data.name, "groups": groups}


@router.delete("/{mg_id}/groups/{group_id}", status_code=204)
async def delete_mg_group(mg_id: UUID, group_id: str, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    token = _token(request)
    repo = MastergraphRepository(db)
    mg = await repo.get_by_id(mg_id)
    if not mg:
        raise HTTPException(404, "Mastergraph not found")
    access = await check_project_access(mg.project_id, user_id, "edit_mastergraphic", token)
    if not access.get("has_access"):
        raise HTTPException(403, "Access denied")
    mg_repo = MgGroupRepository(db)
    ok = await mg_repo.remove(mg_id, group_id, user_id)
    if not ok:
        raise HTTPException(404, "Group not found in this mastergraph")
    await db.commit()
    return None


# ---------------- История ----------------

@router.get("/{mg_id}/history")
async def get_history(
    mg_id: UUID,
    request: Request,
    entity_type: Optional[str] = Query(None),
    entity_id: Optional[UUID] = Query(None),
    limit: int = 500,
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
    event_repo = EventRepository(db)
    events = await event_repo.list_by_entity(mg_id, entity_type, entity_id, limit)
    return {
        "mg_id": str(mg_id),
        "events": [
            {"event_id": str(e.event_id), "entity_type": e.entity_type,
             "entity_id": str(e.entity_id), "event_type": e.event_type,
             "field": e.field, "old_value": e.old_value, "new_value": e.new_value,
             "reason": e.reason, "actor_id": e.actor_id,
             "created_at": e.created_at.isoformat() if e.created_at else None}
            for e in events
        ],
    }
