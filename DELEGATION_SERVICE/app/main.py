# app/main.py
from contextlib import asynccontextmanager
from fastapi import FastAPI
from app.database import engine, Base
from app.api.v1 import delegations, rules
from app.services.event_consumer import event_consumer_task
import asyncio
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

_expire_task = None
_event_task = None


@asynccontextmanager
async def lifespan(app: FastAPI):
    logger.info("Starting up Delegation Service...")
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)
    logger.info("Database tables created/verified")

    global _expire_task, _event_task
    _expire_task = asyncio.create_task(_expire_loop())
    _event_task = asyncio.create_task(event_consumer_task())

    yield

    for task in (_expire_task, _event_task):
        if task:
            task.cancel()
            try:
                await task
            except asyncio.CancelledError:
                pass

    logger.info("Shutting down Delegation Service...")
    await engine.dispose()


async def _expire_loop():
    from app.services.delegation_service import DelegationService
    from app.repositories.delegation_repo import DelegationRepository
    from app.repositories.history_repo import HistoryRepository
    from app.services.validation_service import ValidationService
    from app.repositories.rule_repo import RuleRepository
    from app.database import AsyncSessionLocal
    from app.config import settings

    while True:
        try:
            await asyncio.sleep(settings.AUTO_EXPIRE_CHECK_INTERVAL_SECONDS)
            async with AsyncSessionLocal() as db:
                service = DelegationService(
                    DelegationRepository(db),
                    HistoryRepository(db),
                    ValidationService(DelegationRepository(db), RuleRepository(db)),
                )
                count = await service.expire_expired_delegations()
                if count:
                    logger.info(f"Auto-expired {count} delegations")
        except asyncio.CancelledError:
            raise
        except Exception as e:
            logger.error(f"Expire loop error: {e}")


app = FastAPI(
    title="Delegation Service",
    description="Управление делегированием полномочий",
    version="2.0.0",
    lifespan=lifespan,
)


@app.get("/health")
async def health_check():
    return {"status": "healthy", "service": "delegation_service"}


@app.get("/")
async def root():
    return {"message": "Delegation Service is running", "docs": "/docs"}


app.include_router(delegations.router)
app.include_router(rules.router)
