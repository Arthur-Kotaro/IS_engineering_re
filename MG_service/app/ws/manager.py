# app/ws/manager.py
import asyncio
import logging
from typing import Dict, Set
from fastapi import WebSocket

logger = logging.getLogger(__name__)


class ConnectionManager:
    def __init__(self):
        self._connections: Dict[str, Set[WebSocket]] = {}
        self._lock = asyncio.Lock()

    async def connect(self, mg_id: str, ws: WebSocket):
        await ws.accept()
        async with self._lock:
            self._connections.setdefault(mg_id, set()).add(ws)
        logger.info(f"WS connected: mg={mg_id}, total={len(self._connections[mg_id])}")

    async def disconnect(self, mg_id: str, ws: WebSocket):
        async with self._lock:
            if mg_id in self._connections:
                self._connections[mg_id].discard(ws)
                if not self._connections[mg_id]:
                    del self._connections[mg_id]
        logger.info(f"WS disconnected: mg={mg_id}")

    async def broadcast(self, mg_id: str, message: dict):
        async with self._lock:
            conns = list(self._connections.get(mg_id, set()))
        dead = []
        for ws in conns:
            try:
                await ws.send_json(message)
            except Exception as e:
                logger.warning(f"WS send failed: {e}")
                dead.append(ws)
        for ws in dead:
            await self.disconnect(mg_id, ws)


manager = ConnectionManager()
