# app/main.py
from fastapi import FastAPI
from contextlib import asynccontextmanager
from app.database import engine, Base
from app.api.v1 import projects, internal


@asynccontextmanager
async def lifespan(app: FastAPI):
    print("Starting up Project Service...")
    from app.models import project, project_member  # noqa
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)
    yield
    await engine.dispose()
    print("Shutting down Project Service...")


app = FastAPI(
    title="Project Service",
    description="Сервис управления проектами и правами",
    version="2.0.0",
    lifespan=lifespan,
)


@app.get("/health")
async def health_check():
    return {"status": "healthy"}


app.include_router(projects.router, prefix="/api/v1")
app.include_router(internal.router, prefix="/internal")
