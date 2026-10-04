# app/services/event_consumer.py
import asyncio
import logging
import os
import httpx
import redis.asyncio as aioredis

logger = logging.getLogger(__name__)

REDIS_HOST = os.getenv("REDIS_HOST", "localhost")
REDIS_PORT = int(os.getenv("REDIS_PORT", "6379"))
REDIS_EVENTS_DB = int(os.getenv("REDIS_EVENTS_DB", "3"))
CHANNEL = "user.events"


async def event_consumer_task():
    redis = aioredis.Redis(host=REDIS_HOST, port=REDIS_PORT, db=REDIS_EVENTS_DB, decode_responses=True)
    pubsub = redis.pubsub()
    await pubsub.subscribe(CHANNEL)
    logger.info(f"Subscribed to {CHANNEL}")

    from app.repositories.delegation_repo import DelegationRepository
    from app.repositories.history_repo import HistoryRepository
    from app.repositories.rule_repo import RuleRepository
    from app.services.validation_service import ValidationService
    from app.services.delegation_service import DelegationService
    from app.database import AsyncSessionLocal
    import json

    async for message in pubsub.listen():
        if message.get("type") != "message":
            continue
        try:
            event = json.loads(message["data"])
        except Exception:
            continue

        event_type = event.get("type")
        if event_type not in ("user.deleted", "user.blocked"):
            continue

        donor_id = event.get("user_id")
        if not donor_id:
            continue

        try:
            async with AsyncSessionLocal() as db:
                service = DelegationService(
                    DelegationRepository(db),
                    HistoryRepository(db),
                    ValidationService(DelegationRepository(db), RuleRepository(db)),
                )
                count = await service.revoke_all_by_donor(donor_id, reason=event_type)
                if count:
                    logger.info(f"Revoked {count} delegations for donor {donor_id} ({event_type})")
        except Exception as e:
            logger.error(f"event_consumer error: {e}")
