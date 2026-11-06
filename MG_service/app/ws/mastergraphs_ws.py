# app/ws/mastergraphs_ws.py
import logging
from fastapi import APIRouter, WebSocket, WebSocketDisconnect, Query
from sqlalchemy import select

from app.database import AsyncSessionLocal
from app.models.mastergraph import Mastergraph
from app.models.mg_task import MgTask
from app.models.mg_dependency import MgDependency
from app.models.task_group import TaskGroup
from app.ws.manager import manager

logger = logging.getLogger(__name__)
router = APIRouter(tags=["WebSocket"])


@router.websocket("/ws/mastergraphs/{mg_id}")
async def ws_mastergraph(ws: WebSocket, mg_id: str, last_version: int = Query(0)):
    print(f"[WS ENDPOINT] connection request for mg={mg_id}", flush=True)
    await manager.connect(mg_id, ws)
    try:
        async with AsyncSessionLocal() as session:
            result = await session.execute(select(Mastergraph).where(Mastergraph.mg_id == mg_id))
            mg = result.scalar_one_or_none()
            if not mg:
                await ws.send_json({"type": "error", "message": "Mastergraph not found"})
                return

            tasks_result = await session.execute(select(MgTask).where(MgTask.mg_id == mg_id))
            tasks = tasks_result.scalars().all()
            deps_result = await session.execute(select(MgDependency).where(MgDependency.mg_id == mg_id))
            deps = deps_result.scalars().all()
            groups_result = await session.execute(select(TaskGroup))
            groups = groups_result.scalars().all()

            snapshot = {
                "type": "snapshot",
                "mg_id": str(mg.mg_id),
                "project_id": mg.project_id,
                "name": mg.name,
                "version": mg.version,
                "tasks": [
                    {
                        "task_id": str(t.task_id),
                        "group_id": t.group_id,
                        "title": t.title,
                        "responsible_id": t.responsible_id,
                        "status": t.status,
                        "start_date": t.start_date.isoformat() if t.start_date else None,
                        "end_date": t.end_date.isoformat() if t.end_date else None,
                        "forecast_start": t.forecast_start.isoformat() if t.forecast_start else None,
                        "forecast_end": t.forecast_end.isoformat() if t.forecast_end else None,
                        "progress_current": t.progress_current,
                        "progress_total": t.progress_total,
                        "sort_order": t.sort_order,
                        "version": t.version,
                    } for t in tasks
                ],
                "dependencies": [
                    {
                        "dep_id": str(d.dep_id),
                        "predecessor_id": str(d.predecessor_id),
                        "successor_id": str(d.successor_id),
                        "dep_type": d.dep_type,
                        "version": d.version,
                    } for d in deps
                ],
                "groups": [
                    {
                        "group_id": g.group_id,
                        "name": g.name,
                        "description": g.description,
                        "bus_instance_id": g.bus_instance_id,
                    } for g in groups
                ],
            }
            await ws.send_json(snapshot)
            print(f"[WS ENDPOINT] snapshot sent for mg={mg_id}", flush=True)

        while True:
            data = await ws.receive_json()
            print(f"[WS ENDPOINT] received: {data}", flush=True)
            await ws.send_json({"type": "ack", "received": data.get("type")})

    except WebSocketDisconnect:
        await manager.disconnect(mg_id, ws)
    except Exception as e:
        print(f"[WS ENDPOINT] error: {e}", flush=True)
        logger.error(f"WS error: {e}")
        await manager.disconnect(mg_id, ws)
