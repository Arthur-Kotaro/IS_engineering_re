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
        # ВАЖНО: accept() уже вызван в endpoint, здесь только регистрация
        async with self._lock:
            self._connections.setdefault(lg_id, set()).add(ws)
            total = len(self._connections[lg_id])
        print(f"[WS MANAGER BUS] connected: lg={lg_id}, total={total}", flush=True)
        logger.info(f"WS BUS connected: lg={lg_id}, total={total}")

    async def disconnect(self, lg_id: str, ws: WebSocket):
        async with self._lock:
            if lg_id in self._connections:
                self._connections[lg_id].discard(ws)
                if not self._connections[lg_id]:
                    del self._connections[lg_id]
        print(f"[WS MANAGER BUS] disconnected: lg={lg_id}", flush=True)
        logger.info(f"WS BUS disconnected: lg={lg_id}")

    async def broadcast(self, lg_id: str, message: dict):
        async with self._lock:
            conns = list(self._connections.get(lg_id, set()))
        print(f"[WS MANAGER BUS] broadcast to lg={lg_id}, connections={len(conns)}", flush=True)
        dead = []
        for ws in conns:
            try:
                await ws.send_json(message)
            except Exception as e:
                print(f"[WS MANAGER BUS] send failed: {e}", flush=True)
                logger.warning(f"WS BUS send failed: {e}")
                dead.append(ws)
        for ws in dead:
            await self.disconnect(lg_id, ws)


manager = ConnectionManager()
