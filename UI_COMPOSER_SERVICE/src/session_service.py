# src/session_service.py
import json
import uuid
import logging
import redis.asyncio as aioredis
from typing import Optional
from src.config import settings
from src.models import WorkflowSession

logger = logging.getLogger(__name__)


class SessionService:
    def __init__(self):
        self.redis: aioredis.Redis = aioredis.Redis(
            host=settings.REDIS_HOST,
            port=settings.REDIS_PORT,
            db=settings.REDIS_SESSIONS_DB,
            decode_responses=True,
        )
        self.ttl = settings.SESSION_TTL_SECONDS

    def _key(self, session_id: str) -> str:
        return f"session:{session_id}"

    async def create(self, workflow_name: str, initial_step: str, user_id: int) -> WorkflowSession:
        session_id = str(uuid.uuid4())
        session = WorkflowSession(
            session_id=session_id,
            workflow_name=workflow_name,
            current_step=initial_step,
            user_id=user_id,
            data={},
            history=[],
        )
        await self._save(session)
        logger.info(f"Session created: {session_id} workflow={workflow_name}")
        return session

    async def get(self, session_id: str) -> Optional[WorkflowSession]:
        raw = await self.redis.get(self._key(session_id))
        if not raw:
            return None
        data = json.loads(raw)
        return WorkflowSession(**data)

    async def _save(self, session: WorkflowSession):
        await self.redis.set(
            self._key(session.session_id),
            json.dumps({
                "session_id": session.session_id,
                "workflow_name": session.workflow_name,
                "current_step": session.current_step,
                "user_id": session.user_id,
                "data": session.data,
                "history": session.history,
            }),
            ex=self.ttl,
        )

    async def save(self, session: WorkflowSession):
        await self._save(session)

    async def close(self, session_id: str):
        await self.redis.delete(self._key(session_id))
        logger.info(f"Session closed: {session_id}")

    async def ping(self) -> bool:
        try:
            return await self.redis.ping()
        except Exception:
            return False
