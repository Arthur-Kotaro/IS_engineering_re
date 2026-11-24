# app/ws/mastergraphs_ws.py
import logging
from fastapi import APIRouter, WebSocket, WebSocketDisconnect, Query
from sqlalchemy import select

from app.database import AsyncSessionLocal
from app.models.mastergraph import Mastergraph
from app.models.mg_task import MgTask
from app.models.mg_dependency import MgDependency
from app.models.mg_milestone import MgMilestone
from app.repositories.mastergraph_repo import MastergraphRepository
from app.ws.manager import manager
from app.services.access_client import check_project_access
from app.services.user_client import get_user_name
from app.services.bus_client import get_bus_url_for_group, get_local_graph_state
from app.api.v1.mastergraphs import _lg_state_cache

logger = logging.getLogger(__name__)
router = APIRouter(tags=["WebSocket"])


def _extract_user_id(ws: WebSocket) -> int | None:
    user_id_str = ws.headers.get("X-User-ID")
    if not user_id_str:
        return None
    try:
        return int(user_id_str)
    except ValueError:
        return None


def _extract_token(ws: WebSocket) -> str | None:
    auth = ws.headers.get("Authorization", "")
    return auth.removeprefix("Bearer ").strip() or None


@router.websocket("/ws/mastergraphs/{mg_id}")
async def ws_mastergraph(ws: WebSocket, mg_id: str, last_version: int = Query(0)):
    print(f"[WS ENDPOINT] connection for mg={mg_id}", flush=True)
    await ws.accept()

    user_id = _extract_user_id(ws)
    if user_id is None:
        await ws.close(code=1008, reason="Missing X-User-ID")
        return

    token = _extract_token(ws)

    async with AsyncSessionLocal() as session:
        result = await session.execute(
            select(Mastergraph).where(Mastergraph.mg_id == mg_id, Mastergraph.is_deleted == False)
        )
        mg = result.scalar_one_or_none()
        if not mg:
            await ws.close(code=1008, reason="Mastergraph not found")
            return

        access = await check_project_access(mg.project_id, user_id, "view_project", token)
        if not access.get("has_access"):
            await ws.close(code=1008, reason="Access denied")
            return

    await manager.connect(mg_id, ws)
    try:
        async with AsyncSessionLocal() as session:
            result = await session.execute(select(Mastergraph).where(Mastergraph.mg_id == mg_id))
            mg = result.scalar_one_or_none()

            tasks_result = await session.execute(
                select(MgTask).where(MgTask.mg_id == mg_id, MgTask.is_deleted == False)
            )
            tasks = tasks_result.scalars().all()

            # Подтягиваем ФИО ответственных
            enriched_tasks = []
            for t in tasks:
                resp_name = None
                if t.responsible_id:
                    resp_name = await get_user_name(t.responsible_id)
                bus_url = get_bus_url_for_group(t.group_id)
                task_key = str(t.task_id)
                lg_state_raw = await get_local_graph_state(bus_url, task_key) if bus_url else 0
                if lg_state_raw is None:
                    lg_state = _lg_state_cache.get(task_key, 0)
                else:
                    lg_state = lg_state_raw
                    _lg_state_cache[task_key] = lg_state
                enriched_tasks.append({
                    "task_id": str(t.task_id), "group_id": t.group_id, "title": t.title,
                    "responsible_id": t.responsible_id, "responsible_name": resp_name,
                    "local_graph_state": lg_state,
                    "status": t.status,
                    "start_date": t.start_date.isoformat() if t.start_date else None,
                    "end_date": t.end_date.isoformat() if t.end_date else None,
                    "forecast_start": t.forecast_start.isoformat() if t.forecast_start else None,
                    "forecast_end": t.forecast_end.isoformat() if t.forecast_end else None,
                    "progress_current": t.progress_current, "progress_total": t.progress_total,
                    "sort_order": t.sort_order, "version": t.version,
                    "comment": t.comment, "parent_task_id": str(t.parent_task_id) if t.parent_task_id else None,
                })

            deps_result = await session.execute(
                select(MgDependency).where(MgDependency.mg_id == mg_id, MgDependency.is_deleted == False)
            )
            deps = deps_result.scalars().all()
            ms_result = await session.execute(
                select(MgMilestone)
                .where(MgMilestone.mg_id == mg_id, MgMilestone.is_deleted == False)
                .order_by(MgMilestone.sort_order)
            )
            milestones = ms_result.scalars().all()

            # Группы — тот же источник правды, что и REST: только привязанные к МГ, с sort_order
            repo = MastergraphRepository(session)
            groups = await repo.get_groups_for_mg(mg.mg_id)

            snapshot = {
                "type": "snapshot",
                "mg_id": str(mg.mg_id),
                "project_id": mg.project_id,
                "name": mg.name,
                "version": mg.version,
                "is_approved": mg.is_approved,
                "typology_name": mg.typology_name,
                "tasks": enriched_tasks,
                "dependencies": [
                    {"dep_id": str(d.dep_id), "predecessor_id": str(d.predecessor_id),
                     "successor_id": str(d.successor_id), "dep_type": d.dep_type, "version": d.version}
                    for d in deps
                ],
                "milestones": [
                    {"milestone_id": str(m.milestone_id), "abbreviation": m.abbreviation,
                     "full_name": m.full_name, "tooltip": m.tooltip, "week_offset": m.week_offset,
                     "planned_date": m.planned_date.isoformat() if m.planned_date else None,
                     "actual_date": m.actual_date.isoformat() if m.actual_date else None,
                     "status": m.status, "sort_order": m.sort_order, "version": m.version}
                    for m in milestones
                ],
                "groups": groups,
            }
            await ws.send_json(snapshot)
            print(f"[WS ENDPOINT] snapshot sent for mg={mg_id}", flush=True)

        while True:
            data = await ws.receive_json()
            await ws.send_json({"type": "ack", "received": data.get("type")})

    except WebSocketDisconnect:
        await manager.disconnect(mg_id, ws)
    except Exception as e:
        print(f"[WS ENDPOINT] error: {e}", flush=True)
        logger.error(f"WS error: {e}")
        await manager.disconnect(mg_id, ws)
