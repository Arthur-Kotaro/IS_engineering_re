# app/services/mastergraph_service.py
import logging
from datetime import date, datetime, timezone
from typing import Optional, List, Tuple
from uuid import UUID
from sqlalchemy import select
from sqlalchemy.exc import IntegrityError
from sqlalchemy.ext.asyncio import AsyncSession

from app.models.mastergraph import Mastergraph
from app.models.mg_task import MgTask
from app.models.mg_milestone import MgMilestone, MgMilestoneStatus
from app.models.mg_status_history import MgStatusHistory
from app.models.mg_dependency import MgDependency
from app.models.mg_event import MgEntityType, MgEventType
from app.repositories.event_repo import EventRepository
from app.repositories.milestone_repo import MilestoneRepository
from app.repositories.mastergraph_repo import MastergraphRepository

logger = logging.getLogger(__name__)


class MastergraphError(Exception):
    pass


class NotCveError(MastergraphError):
    pass


class AlreadyApprovedError(MastergraphError):
    pass


class FirstMilestoneCompletedError(MastergraphError):
    pass


class DraftModeError(MastergraphError):
    pass


class ReasonRequiredError(MastergraphError):
    pass


class TaskTitleConflictError(MastergraphError):
    def __init__(self, conflicts: List[Tuple[str, str]]):
        self.conflicts = conflicts
        super().__init__(f"Task title conflicts: {conflicts}")


async def _get_mg(db: AsyncSession, mg_id: UUID) -> Mastergraph:
    result = await db.execute(
        select(Mastergraph).where(Mastergraph.mg_id == mg_id, Mastergraph.is_deleted == False)
    )
    mg = result.scalar_one_or_none()
    if not mg:
        raise MastergraphError("Mastergraph not found")
    return mg


async def _is_cve(db: AsyncSession, mg: Mastergraph, user_id: int, token: Optional[str]) -> bool:
    from app.services.access_client import check_project_access
    access = await check_project_access(mg.project_id, user_id, "edit_mastergraphic", token)
    if not access.get("has_access"):
        return False
    role = access.get("role")
    if role and role != "chief_engineer":
        return False
    return True


async def _check_task_title_conflicts(
    db: AsyncSession, mg_id: UUID, pairs: List[Tuple[str, str]]
) -> List[Tuple[str, str]]:
    seen = set()
    duplicates_in_batch = []
    for pair in pairs:
        if pair in seen:
            duplicates_in_batch.append(pair)
        seen.add(pair)
    if duplicates_in_batch:
        return duplicates_in_batch

    repo = MastergraphRepository(db)
    return await repo.get_conflicts(mg_id, pairs)


async def _get_predecessor_end(db: AsyncSession, mg_id: UUID, task_id: UUID) -> Optional[str]:
    """Возвращает end_date ближайшего предшественника (если есть)."""
    from app.models.mg_dependency import MgDependency
    from app.models.mg_task import MgTask
    dep_res = await db.execute(
        select(MgDependency).where(
            MgDependency.mg_id == mg_id,
            MgDependency.successor_id == task_id,
            MgDependency.is_deleted == False,
        )
    )
    preds = dep_res.scalars().all()
    if not preds:
        return None
    pred_res = await db.execute(select(MgTask).where(MgTask.task_id == preds[0].predecessor_id))
    pred = pred_res.scalar_one_or_none()
    if pred and pred.end_date:
        return pred.end_date.isoformat()
    return None


async def _notify_bus_about_dates(db: AsyncSession, mg: Mastergraph, task: MgTask):
    """MG → BUS: init_local_graph при изменении сроков задачи."""
    from app.services.bus_client import init_local_graph, get_bus_url_for_group
    bus_url = get_bus_url_for_group(task.group_id)
    if not bus_url:
        return
    pred_end = await _get_predecessor_end(db, mg.mg_id, task.task_id)
    payload = {
        "project_id": mg.project_id,
        "task_id": str(task.task_id),
        "responsible_id": task.responsible_id,
        "title": task.title,
        "target_start": task.start_date.isoformat() if task.start_date else None,
        "target_end": task.end_date.isoformat() if task.end_date else None,
        "predecessor_end": pred_end,
    }
    try:
        await init_local_graph(bus_url, payload)
    except Exception as e:
        logger.warning(f"_notify_bus_about_dates: {e}")


async def create_task(
    db: AsyncSession,
    mg_id: UUID,
    user_id: int,
    token: Optional[str],
    group_id: str,
    title: str,
    responsible_id: Optional[int] = None,
    start_date: Optional[date] = None,
    end_date: Optional[date] = None,
    sort_order: int = 0,
    parent_task_id: Optional[UUID] = None,
    comment: Optional[str] = None,
) -> MgTask:
    mg = await _get_mg(db, mg_id)

    from app.services.access_client import check_project_access
    access = await check_project_access(mg.project_id, user_id, "edit_mastergraphic", token)
    if not access.get("has_access"):
        raise MastergraphError("Access denied")

    from app.models.task_group import TaskGroup
    tg = await db.execute(select(TaskGroup).where(TaskGroup.group_id == group_id, TaskGroup.is_deleted == False))
    if not tg.scalar_one_or_none():
        raise MastergraphError(f"Group '{group_id}' not found")

    repo = MastergraphRepository(db)
    conflicts = await _check_task_title_conflicts(db, mg_id, [(group_id, title)])
    if conflicts:
        raise TaskTitleConflictError(conflicts)

    task = MgTask(
        mg_id=mg_id,
        group_id=group_id,
        title=title,
        responsible_id=responsible_id,
        status=0,
        start_date=start_date,
        end_date=end_date,
        forecast_start=start_date,
        forecast_end=end_date,
        progress_current=-1,
        progress_total=-1,
        sort_order=sort_order,
        parent_task_id=parent_task_id,
        comment=comment,
    )
    db.add(task)
    try:
        await db.flush()
    except IntegrityError:
        await db.rollback()
        raise TaskTitleConflictError([(group_id, title)])

    event_repo = EventRepository(db)
    await event_repo.log(
        mg_id=mg_id, entity_type=MgEntityType.TASK, entity_id=task.task_id,
        event_type=MgEventType.CREATE,
        new_value={"title": task.title, "group_id": task.group_id,
                   "start_date": str(task.start_date), "end_date": str(task.end_date)},
        actor_id=user_id,
    )
    await db.commit()
    await db.refresh(task)

    # MG → BUS: уведомить о сроках (если ответственный задан)
    if task.responsible_id:
        await _notify_bus_about_dates(db, mg, task)

    return task


async def create_tasks_batch(
    db: AsyncSession,
    mg_id: UUID,
    user_id: int,
    token: Optional[str],
    tasks_data: List[dict],
    dependencies_data: List[dict],
) -> Tuple[List[MgTask], List[MgDependency]]:
    mg = await _get_mg(db, mg_id)

    from app.services.access_client import check_project_access
    access = await check_project_access(mg.project_id, user_id, "edit_mastergraphic", token)
    if not access.get("has_access"):
        raise MastergraphError("Access denied")

    from app.models.task_group import TaskGroup
    group_ids = set(t["group_id"] for t in tasks_data)
    tg_res = await db.execute(
        select(TaskGroup.group_id).where(TaskGroup.group_id.in_(group_ids), TaskGroup.is_deleted == False)
    )
    existing_groups = set(g for g, in tg_res.all())
    missing_groups = group_ids - existing_groups
    if missing_groups:
        raise MastergraphError(f"Groups not found: {missing_groups}")

    pairs = [(t["group_id"], t["title"]) for t in tasks_data]
    conflicts = await _check_task_title_conflicts(db, mg_id, pairs)
    if conflicts:
        raise TaskTitleConflictError(conflicts)

    created_tasks: List[MgTask] = []
    batch_map: dict[str, UUID] = {}
    try:
        for t in tasks_data:
            task = MgTask(
                mg_id=mg_id,
                group_id=t["group_id"],
                title=t["title"],
                responsible_id=t.get("responsible_id"),
                status=0,
                start_date=t.get("start_date"),
                end_date=t.get("end_date"),
                forecast_start=t.get("start_date"),
                forecast_end=t.get("end_date"),
                progress_current=-1,
                progress_total=-1,
                sort_order=t.get("sort_order", 0),
                comment=t.get("comment"),
            )
            db.add(task)
            await db.flush()
            created_tasks.append(task)
            batch_map[t["batch_id"]] = task.task_id

        created_deps: List[MgDependency] = []
        for d in dependencies_data:
            pred_bid = d["predecessor_batch_id"]
            succ_bid = d["successor_batch_id"]
            if pred_bid not in batch_map or succ_bid not in batch_map:
                raise MastergraphError(f"Dependency batch_id not found: {pred_bid} -> {succ_bid}")
            dep = MgDependency(
                mg_id=mg_id,
                predecessor_id=batch_map[pred_bid],
                successor_id=batch_map[succ_bid],
                dep_type="finish_to_start",
            )
            db.add(dep)
            await db.flush()
            created_deps.append(dep)

        event_repo = EventRepository(db)
        for t in created_tasks:
            await event_repo.log(
                mg_id=mg_id, entity_type=MgEntityType.TASK, entity_id=t.task_id,
                event_type=MgEventType.CREATE,
                new_value={"title": t.title, "group_id": t.group_id,
                           "start_date": str(t.start_date), "end_date": str(t.end_date)},
                actor_id=user_id,
            )
        for dep in created_deps:
            await event_repo.log(
                mg_id=mg_id, entity_type=MgEntityType.DEPENDENCY, entity_id=dep.dep_id,
                event_type=MgEventType.CREATE,
                new_value={"predecessor_id": str(dep.predecessor_id),
                           "successor_id": str(dep.successor_id)},
                actor_id=user_id,
            )

        await db.commit()
    except IntegrityError as e:
        await db.rollback()
        raise TaskTitleConflictError([(t["group_id"], t["title"]) for t in tasks_data])
    except Exception as e:
        await db.rollback()
        raise

    for t in created_tasks:
        await db.refresh(t)

    # MG → BUS: для каждой задачи с ответственным
    for t in created_tasks:
        if t.responsible_id:
            await _notify_bus_about_dates(db, mg, t)

    return created_tasks, created_deps


async def create_dependency(
    db: AsyncSession,
    mg_id: UUID,
    user_id: int,
    token: Optional[str],
    predecessor_id: UUID,
    successor_id: UUID,
) -> MgDependency:
    mg = await _get_mg(db, mg_id)

    from app.services.access_client import check_project_access
    access = await check_project_access(mg.project_id, user_id, "edit_mastergraphic", token)
    if not access.get("has_access"):
        raise MastergraphError("Access denied")

    if predecessor_id == successor_id:
        raise MastergraphError("predecessor_id == successor_id")

    res = await db.execute(
        select(MgTask.task_id).where(
            MgTask.mg_id == mg_id,
            MgTask.task_id.in_([predecessor_id, successor_id]),
            MgTask.is_deleted == False,
        )
    )
    found = set(t for t, in res.all())
    if len(found) != 2:
        raise MastergraphError("Task not found in this mastergraph")

    existing = await db.execute(
        select(MgDependency).where(
            MgDependency.mg_id == mg_id,
            MgDependency.predecessor_id == predecessor_id,
            MgDependency.successor_id == successor_id,
            MgDependency.is_deleted == False,
        )
    )
    if existing.scalar_one_or_none():
        raise MastergraphError("Dependency already exists")

    dep = MgDependency(
        mg_id=mg_id,
        predecessor_id=predecessor_id,
        successor_id=successor_id,
        dep_type="finish_to_start",
    )
    db.add(dep)
    await db.flush()

    event_repo = EventRepository(db)
    await event_repo.log(
        mg_id=mg_id, entity_type=MgEntityType.DEPENDENCY, entity_id=dep.dep_id,
        event_type=MgEventType.CREATE,
        new_value={"predecessor_id": str(predecessor_id), "successor_id": str(successor_id)},
        actor_id=user_id,
    )
    await db.commit()
    await db.refresh(dep)

    # MG → BUS: уведомить successor о сроке предшественника
    succ_res = await db.execute(select(MgTask).where(MgTask.task_id == successor_id))
    succ = succ_res.scalar_one_or_none()
    if succ and succ.responsible_id:
        await _notify_bus_about_dates(db, mg, succ)

    return dep


async def delete_dependency(
    db: AsyncSession,
    mg_id: UUID,
    dep_id: UUID,
    user_id: int,
    token: Optional[str],
):
    mg = await _get_mg(db, mg_id)

    from app.services.access_client import check_project_access
    access = await check_project_access(mg.project_id, user_id, "edit_mastergraphic", token)
    if not access.get("has_access"):
        raise MastergraphError("Access denied")

    result = await db.execute(
        select(MgDependency).where(
            MgDependency.dep_id == dep_id,
            MgDependency.mg_id == mg_id,
            MgDependency.is_deleted == False,
        )
    )
    dep = result.scalar_one_or_none()
    if not dep:
        raise MastergraphError("Dependency not found")

    dep.is_deleted = True
    dep.deleted_at = datetime.now(timezone.utc)
    dep.deleted_by = user_id

    event_repo = EventRepository(db)
    await event_repo.log(
        mg_id=mg_id, entity_type=MgEntityType.DEPENDENCY, entity_id=dep.dep_id,
        event_type=MgEventType.DELETE,
        old_value={"predecessor_id": str(dep.predecessor_id), "successor_id": str(dep.successor_id)},
        actor_id=user_id,
    )
    await db.commit()


async def approve_mastergraph(
    db: AsyncSession,
    mg_id: UUID,
    user_id: int,
    token: Optional[str],
    reason: Optional[str] = None,
) -> Mastergraph:
    mg = await _get_mg(db, mg_id)

    if mg.is_approved:
        raise AlreadyApprovedError("Mastergraph is already approved")

    if not await _is_cve(db, mg, user_id, token):
        raise NotCveError("Only CVE can approve mastergraph")

    ms_repo = MilestoneRepository(db)
    first = await ms_repo.first_milestone(mg_id)
    if first and first.status == MgMilestoneStatus.COMPLETED:
        raise FirstMilestoneCompletedError(
            f"First milestone '{first.abbreviation}' is already completed"
        )

    old_status = "approved" if mg.is_approved else "draft"
    mg.is_approved = True
    mg.approved_at = datetime.now(timezone.utc)
    mg.approved_by = user_id
    mg.version += 1

    history = MgStatusHistory(
        mg_id=mg.mg_id,
        old_status=old_status,
        new_status="approved",
        reason=reason,
        actor_id=user_id,
    )
    db.add(history)

    event_repo = EventRepository(db)
    await event_repo.log(
        mg_id=mg.mg_id,
        entity_type=MgEntityType.MASTERGRAPH,
        entity_id=mg.mg_id,
        event_type=MgEventType.APPROVE,
        old_value={"is_approved": False},
        new_value={"is_approved": True},
        reason=reason,
        actor_id=user_id,
    )

    await db.commit()
    await db.refresh(mg)
    return mg


async def update_milestone(
    db: AsyncSession,
    mg_id: UUID,
    milestone_id: UUID,
    user_id: int,
    token: Optional[str],
    new_planned_date: Optional[date] = None,
    reason: Optional[str] = None,
) -> MgMilestone:
    mg = await _get_mg(db, mg_id)

    from app.services.access_client import check_project_access
    access = await check_project_access(mg.project_id, user_id, "edit_mastergraphic", token)
    if not access.get("has_access"):
        raise MastergraphError("Access denied")

    ms_repo = MilestoneRepository(db)
    ms = await ms_repo.get_by_id(milestone_id)
    if not ms or ms.mg_id != mg_id:
        raise MastergraphError("Milestone not found")

    if ms.status == MgMilestoneStatus.COMPLETED:
        raise MastergraphError("Cannot reschedule completed milestone")

    if mg.is_approved and not reason:
        raise ReasonRequiredError("Reason is required in approved mode")

    old_date = ms.planned_date
    if new_planned_date:
        ms.planned_date = new_planned_date
        ms.status = MgMilestoneStatus.RESCHEDULED
        ms.version += 1

    event_repo = EventRepository(db)
    await event_repo.log(
        mg_id=mg.mg_id,
        entity_type=MgEntityType.MILESTONE,
        entity_id=ms.milestone_id,
        event_type=MgEventType.RESCHEDULE if new_planned_date else MgEventType.UPDATE,
        field="planned_date" if new_planned_date else None,
        old_value={"planned_date": old_date.isoformat()} if new_planned_date else None,
        new_value={"planned_date": new_planned_date.isoformat()} if new_planned_date else None,
        reason=reason,
        actor_id=user_id,
    )

    await db.commit()
    await db.refresh(ms)
    return ms


async def complete_milestone(
    db: AsyncSession,
    mg_id: UUID,
    milestone_id: UUID,
    user_id: int,
    token: Optional[str],
    actual_date: Optional[date] = None,
) -> MgMilestone:
    mg = await _get_mg(db, mg_id)

    if not mg.is_approved:
        raise DraftModeError("Cannot complete milestone in draft mode")

    from app.services.access_client import check_project_access
    access = await check_project_access(mg.project_id, user_id, "edit_mastergraphic", token)
    if not access.get("has_access"):
        raise MastergraphError("Access denied")

    ms_repo = MilestoneRepository(db)
    ms = await ms_repo.get_by_id(milestone_id)
    if not ms or ms.mg_id != mg_id:
        raise MastergraphError("Milestone not found")

    if ms.status == MgMilestoneStatus.COMPLETED:
        raise MastergraphError("Milestone already completed")

    all_ms = await ms_repo.list_by_mg(mg_id)
    for prev in all_ms:
        if prev.sort_order >= ms.sort_order:
            break
        if prev.status != MgMilestoneStatus.COMPLETED:
            raise MastergraphError(
                f"Previous milestone '{prev.abbreviation}' is not completed"
            )

    ms.status = MgMilestoneStatus.COMPLETED
    ms.actual_date = actual_date or date.today()
    ms.version += 1

    event_repo = EventRepository(db)
    await event_repo.log(
        mg_id=mg.mg_id,
        entity_type=MgEntityType.MILESTONE,
        entity_id=ms.milestone_id,
        event_type=MgEventType.COMPLETE,
        new_value={"actual_date": ms.actual_date.isoformat()},
        actor_id=user_id,
    )

    await db.commit()
    await db.refresh(ms)
    return ms


async def update_task(
    db: AsyncSession,
    mg_id: UUID,
    task_id: UUID,
    user_id: int,
    token: Optional[str],
    fields: dict,
    reason: Optional[str] = None,
) -> MgTask:
    mg = await _get_mg(db, mg_id)

    from app.services.access_client import check_project_access
    access = await check_project_access(mg.project_id, user_id, "edit_mastergraphic", token)
    if not access.get("has_access"):
        raise MastergraphError("Access denied")

    result = await db.execute(
        select(MgTask).where(MgTask.task_id == task_id, MgTask.mg_id == mg_id, MgTask.is_deleted == False)
    )
    task = result.scalar_one_or_none()
    if not task:
        raise MastergraphError("Task not found")

    date_fields = {"start_date", "end_date"}
    changing_dates = any(f in fields for f in date_fields)
    if mg.is_approved and changing_dates and not reason:
        raise ReasonRequiredError("Reason is required for date changes in approved mode")

    event_repo = EventRepository(db)
    changes = []

    allowed = {"title", "responsible_id", "status", "start_date", "end_date",
               "progress_current", "progress_total", "sort_order", "comment"}

    dates_old = None
    if changing_dates:
        dates_old = {
            "start_date": task.start_date.isoformat() if task.start_date else None,
            "end_date": task.end_date.isoformat() if task.end_date else None,
        }

    for field, value in fields.items():
        if field not in allowed:
            continue
        old = getattr(task, field)
        if old == value:
            continue
        setattr(task, field, value)
        if field not in date_fields:
            changes.append({
                "field": field,
                "old": str(old) if old is not None else None,
                "new": str(value) if value is not None else None,
            })

    if dates_old is not None:
        dates_new = {
            "start_date": task.start_date.isoformat() if task.start_date else None,
            "end_date": task.end_date.isoformat() if task.end_date else None,
        }
        if dates_old != dates_new:
            changes.append({
                "field": "dates",
                "old": dates_old,
                "new": dates_new,
            })

    if not changes:
        return task

    task.version += 1

    try:
        await db.flush()
    except IntegrityError:
        await db.rollback()
        raise MastergraphError("Integrity error (возможно, дубль title)")

    for ch in changes:
        is_dates = ch["field"] == "dates"
        await event_repo.log(
            mg_id=mg.mg_id,
            entity_type=MgEntityType.TASK,
            entity_id=task.task_id,
            event_type=MgEventType.UPDATE,
            field=ch["field"],
            old_value=ch["old"] if is_dates else {"value": ch["old"]},
            new_value=ch["new"] if is_dates else {"value": ch["new"]},
            reason=reason,
            actor_id=user_id,
        )

    await db.commit()
    await db.refresh(task)

    # MG → BUS: уведомить, если изменились сроки ИЛИ ответственный
    responsible_changed = any(c["field"] == "responsible_id" for c in changes)
    dates_changed = (dates_old is not None and dates_old != dates_new)
    if (dates_changed or responsible_changed) and task.responsible_id:
        await _notify_bus_about_dates(db, mg, task)

    return task
