from fastapi import FastAPI
from contextlib import asynccontextmanager
from app.database import engine

@asynccontextmanager
async def lifespan(app: FastAPI):
    print("Starting up Project Service...")
    async with engine.begin() as conn:
        from app.models import project, project_member
        from app.database import Base
        await conn.run_sync(Base.metadata.create_all)
    yield
    await engine.dispose()
    print("Shutting down Project Service...")

app = FastAPI(
    title="Project Service",
    description="Сервис управления проектами и проектными командами",
    version="1.0.0",
    lifespan=lifespan
)

@app.get("/health")
async def health_check():
    return {"status": "healthy"}

from app.api.v1 import projects
app.include_router(projects.router, prefix="/api/v1", tags=["Projects"])
