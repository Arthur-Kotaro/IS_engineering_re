# app/main.py
import asyncio
import logging
from fastapi import FastAPI
from contextlib import asynccontextmanager
from app.database import engine, Base
from app.api.v1 import projects, internal, catalogs
from app.services.mg_creation_worker import queue_worker_loop

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


@asynccontextmanager
async def lifespan(app: FastAPI):
    print("Starting up Project Service...")
    stop_event = asyncio.Event()
    worker_task = asyncio.create_task(queue_worker_loop(stop_event))
    yield
    stop_event.set()
    try:
        await asyncio.wait_for(worker_task, timeout=5)
    except asyncio.TimeoutError:
        worker_task.cancel()
    await engine.dispose()
    print("Shutting down Project Service...")


app = FastAPI(
    title="Project Service",
    description="Сервис управления проектами",
    version="3.1.0",
    lifespan=lifespan,
)


@app.get("/health")
async def health_check():
    return {"status": "healthy"}


app.include_router(projects.router, prefix="/api/v1")
app.include_router(internal.router, prefix="/internal")
app.include_router(catalogs.router, prefix="/api/v1")
