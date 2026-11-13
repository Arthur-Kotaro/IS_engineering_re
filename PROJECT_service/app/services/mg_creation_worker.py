# app/services/mg_creation_worker.py
import asyncio
import logging
from datetime import datetime, timezone, timedelta
from sqlalchemy import select

from app.database import AsyncSessionLocal
from app.models.mg_creation_queue import MgCreationQueue, MgCreationQueueStatus
from app.services.mg_client import create_mastergraph

logger = logging.getLogger(__name__)

WORKER_INTERVAL_SEC = 5
BACKOFF_SEC = [5, 15, 60, 300, 900]


async def process_queue_once() -> int:
    processed = 0
    async with AsyncSessionLocal() as db:
        now = datetime.now(timezone.utc)
        result = await db.execute(
            select(MgCreationQueue)
            .where(
                MgCreationQueue.status.in_([MgCreationQueueStatus.PENDING, MgCreationQueueStatus.PROCESSING]),
                MgCreationQueue.next_retry_at <= now,
                MgCreationQueue.attempt < MgCreationQueue.max_attempts,
            )
            .order_by(MgCreationQueue.next_retry_at)
            .limit(10)
        )
        items = result.scalars().all()

        for item in items:
            item.status = MgCreationQueueStatus.PROCESSING
            item.attempt += 1
            await db.flush()

            resp = await create_mastergraph(
                project_id=item.project_id,
                project_type=item.project_type,
                typology_name=item.typology_name,
                cve_id=item.cve_id,
                ipp_id=item.ipp_id,
            )

            if resp:
                item.status = MgCreationQueueStatus.DONE
                item.last_error = None
                logger.info(f"Queue item {item.id} (project {item.project_id}) done")
            else:
                item.last_error = "MG service unavailable"
                if item.attempt >= item.max_attempts:
                    item.status = MgCreationQueueStatus.FAILED
                    logger.error(f"Queue item {item.id} (project {item.project_id}) FAILED after {item.attempt} attempts")
                else:
                    delay = BACKOFF_SEC[min(item.attempt - 1, len(BACKOFF_SEC) - 1)]
                    item.next_retry_at = datetime.now(timezone.utc) + timedelta(seconds=delay)
                    item.status = MgCreationQueueStatus.PENDING
                    logger.warning(f"Queue item {item.id} (project {item.project_id}) retry in {delay}s (attempt {item.attempt})")

            processed += 1

        await db.commit()
    return processed


async def queue_worker_loop(stop_event: asyncio.Event):
    logger.info("MG creation queue worker started")
    while not stop_event.is_set():
        try:
            processed = await process_queue_once()
            if processed:
                logger.info(f"Queue worker processed {processed} items")
        except Exception as e:
            logger.exception(f"Queue worker error: {e}")
        try:
            await asyncio.wait_for(stop_event.wait(), timeout=WORKER_INTERVAL_SEC)
        except asyncio.TimeoutError:
            pass
    logger.info("MG creation queue worker stopped")
