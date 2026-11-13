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

    async def connect(self, lg_id: str, ws: WebSocket):
        await ws.accept()
        async with self._lock:
            self._connections.setdefault(lg_id, set()).add(ws)
            total = len(self._connections[lg_id])
        logger.info(f"WS connected: lg={lg_id}, total={total}")

    async def disconnect(self, lg_id: str, ws: WebSocket):
        async with self._lock:
            if lg_id in self._connections:
                self._connections[lg_id].discard(ws)
                if not self._connections[lg_id]:
                    del self._connections[lg_id]
        logger.info(f"WS disconnected: lg={lg_id}")

    async def broadcast(self, lg_id: str, message: dict):
        async with self._lock:
            conns = list(self._connections.get(lg_id, set()))
        dead = []
        for ws in conns:
            try:
                await ws.send_json(message)
            except Exception as e:
                logger.warning(f"WS send failed: {e}")
                dead.append(ws)
        for ws in dead:
            await self.disconnect(lg_id, ws)


manager = ConnectionManager()
