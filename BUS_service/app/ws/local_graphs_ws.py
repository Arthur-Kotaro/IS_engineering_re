# app/ws/local_graphs_ws.py
import logging
from fastapi import APIRouter, WebSocket, WebSocketDisconnect, Query
from sqlalchemy import select
from sqlalchemy import func

from app.database import AsyncSessionLocal
from app.models.local_graph import LocalGraph, LgTask
from app.ws.manager import manager

logger = logging.getLogger(__name__)
router = APIRouter(tags=["WebSocket"])


@router.websocket("/ws/local-graphs/{lg_id}")
async def ws_local_graph(ws: WebSocket, lg_id: str, last_version: int = Query(0)):
    await manager.connect(lg_id, ws)
    try:
        async with AsyncSessionLocal() as session:
            result = await session.execute(
                select(LocalGraph).where(LocalGraph.lg_id == lg_id, LocalGraph.is_deleted == 0)
            )
            lg = result.scalar_one_or_none()
            if not lg:
                await ws.send_json({"type": "error", "message": "LocalGraph not found"})
                return

            tasks_result = await session.execute(
                select(LgTask).where(LgTask.lg_id == lg_id, LgTask.is_deleted == 0)
            )
            tasks = tasks_result.scalars().all()

            tasks_count = await session.execute(
                select(func.count(LgTask.lg_task_id)).where(
                    LgTask.lg_id == lg_id, LgTask.is_deleted == 0,
                )
            )
            is_pure = (tasks_count.scalar() or 0) == 0

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
            }
            await ws.send_json(snapshot)

        while True:
            data = await ws.receive_json()
            await ws.send_json({"type": "ack", "received": data.get("type")})

    except WebSocketDisconnect:
        await manager.disconnect(lg_id, ws)
    except Exception as e:
        logger.error(f"WS error: {e}")
        await manager.disconnect(lg_id, ws)
