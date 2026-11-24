# app/ws/local_graphs_ws.py
import logging
import aiohttp
from uuid import UUID
from fastapi import APIRouter, WebSocket, WebSocketDisconnect, Query
from sqlalchemy import select
from sqlalchemy import func

from app.database import AsyncSessionLocal
from app.models.local_graph import LocalGraph, LgTask, LgDependency
from app.ws.manager import manager

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


async def _check_project_access(project_id: int, user_id: int, token: str | None = None) -> bool:
    project_service_url = "http://localhost:8001"
    headers = {"X-User-ID": str(user_id)}
    if token:
        headers["Authorization"] = f"Bearer {token}"
    url = f"{project_service_url}/internal/projects/{project_id}/check-access"
    params = {"user_id": user_id, "permission": "view_project"}
    try:
        async with aiohttp.ClientSession(timeout=aiohttp.ClientTimeout(total=3.0)) as session:
            async with session.get(url, params=params, headers=headers) as resp:
                if resp.status == 200:
                    data = await resp.json()
                    return data.get("has_access", False)
                logger.warning(f"check_access {project_id}: HTTP {resp.status}")
    except Exception as e:
        logger.warning(f"check_access {project_id}: {e}")
    return False


@router.websocket("/ws/local-graphs/{lg_id}")
async def ws_local_graph(ws: WebSocket, lg_id: str, last_version: int = Query(0)):
    print(f"[WS LG ENDPOINT] connection for lg={lg_id}", flush=True)
    await ws.accept()

    user_id = _extract_user_id(ws)
    if user_id is None:
        await ws.close(code=1008, reason="Missing X-User-ID")
        return

    token = _extract_token(ws)

    try:
        lg_uuid = UUID(lg_id)
    except ValueError:
        await ws.close(code=1008, reason="Invalid lg_id format")
        return

    async with AsyncSessionLocal() as session:
        result = await session.execute(
            select(LocalGraph).where(LocalGraph.lg_id == lg_uuid, LocalGraph.is_deleted == 0)
        )
        lg = result.scalar_one_or_none()
        if not lg:
            await ws.close(code=1008, reason="LocalGraph not found")
            return

        has_access = await _check_project_access(lg.project_id, user_id, token)
        if not has_access:
            await ws.close(code=1008, reason="Access denied")
            return

    await manager.connect(lg_id, ws)
    try:
        async with AsyncSessionLocal() as session:
            result = await session.execute(
                select(LocalGraph).where(LocalGraph.lg_id == lg_uuid, LocalGraph.is_deleted == 0)
            )
            lg = result.scalar_one_or_none()

            tasks_result = await session.execute(
                select(LgTask).where(LgTask.lg_id == lg_uuid, LgTask.is_deleted == 0).order_by(LgTask.sort_order)
            )
            tasks = tasks_result.scalars().all()

            tasks_count = await session.execute(
                select(func.count(LgTask.lg_task_id)).where(
                    LgTask.lg_id == lg_uuid, LgTask.is_deleted == 0,
                )
            )
            is_pure = (tasks_count.scalar() or 0) == 0

            deps_result = await session.execute(
                select(LgDependency).where(LgDependency.lg_id == lg_uuid, LgDependency.is_deleted == 0)
            )
            deps = deps_result.scalars().all()

            snapshot = {
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
                "dependencies": [
                    {
                        "dep_id": str(d.dep_id),
                        "lg_id": str(d.lg_id),
                        "predecessor_id": str(d.predecessor_id),
                        "successor_id": str(d.successor_id),
                        "dep_type": d.dep_type,
                        "version": d.version,
                    } for d in deps
                ],
            }
            await ws.send_json(snapshot)
            print(f"[WS LG ENDPOINT] snapshot sent for lg={lg_id}", flush=True)

        while True:
            data = await ws.receive_json()
            await ws.send_json({"type": "ack", "received": data.get("type")})

    except WebSocketDisconnect:
        await manager.disconnect(lg_id, ws)
    except Exception as e:
        print(f"[WS LG ENDPOINT] error: {e}", flush=True)
        logger.error(f"WS LG error: {e}")
        await manager.disconnect(lg_id, ws)
