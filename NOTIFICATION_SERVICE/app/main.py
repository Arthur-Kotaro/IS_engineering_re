# app/main.py
from fastapi import FastAPI
from contextlib import asynccontextmanager
from app.database import engine, Base
from app.api.v1 import notifications
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


@asynccontextmanager
async def lifespan(app: FastAPI):
    logger.info("Starting up Notification Service...")
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)
    logger.info("Database tables created/verified")
    yield
    logger.info("Shutting down Notification Service...")
    await engine.dispose()


app = FastAPI(
    title="Notification Service",
    description="Сервис уведомлений для ERP системы",
    version="1.0.0",
    lifespan=lifespan
)


@app.get("/health")
async def health_check():
    return {"status": "healthy", "service": "notification_service"}


@app.get("/")
async def root():
    return {
        "message": "Notification Service is running",
        "docs": "/docs",
        "redoc": "/redoc"
    }


app.include_router(notifications.router)


def print_routes():
    print("\n=== Available routes ===")
    for route in app.routes:
        methods = getattr(route, "methods", None)
        if methods:
            print(f"  {route.path} -> [{', '.join(sorted(methods))}]")
    print("=======================\n")


if __name__ != "__main__":
    print_routes()


if __name__ == "__main__":
    import uvicorn
    print_routes()
    uvicorn.run(
        "app.main:app",
        host="0.0.0.0",
        port=8012,
        reload=True,
        log_level="info"
    )
