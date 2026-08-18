# app/main.py
from fastapi import FastAPI
from contextlib import asynccontextmanager
from app.database import engine, Base
from app.api.v1 import delegations, rules
import asyncio
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


@asynccontextmanager
async def lifespan(app: FastAPI):
    logger.info("Starting up Delegation Service...")
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)
    logger.info("Database tables created/verified")
    yield
    logger.info("Shutting down Delegation Service...")
    await engine.dispose()


app = FastAPI(
    title="Delegation Service",
    description="Управление делегированием полномочий",
    version="1.0.0",
    lifespan=lifespan
)


@app.get("/health")
async def health_check():
    return {"status": "healthy", "service": "delegation_service"}


@app.get("/")
async def root():
    return {
        "message": "Delegation Service is running",
        "docs": "/docs",
        "redoc": "/redoc"
    }


# Регистрируем роутеры
app.include_router(delegations.router)
app.include_router(rules.router)


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
        port=8011,
        reload=True,
        log_level="info"
    )
