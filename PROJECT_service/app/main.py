# app/main.py
from fastapi import FastAPI
from contextlib import asynccontextmanager
from app.database import engine, Base
from app.api.v1 import projects, internal


@asynccontextmanager
async def lifespan(app: FastAPI):
    print("Starting up Project Service...")
    yield
    await engine.dispose()
    print("Shutting down Project Service...")


app = FastAPI(
    title="Project Service",
    description="Сервис управления проектами",
    version="3.0.0",
    lifespan=lifespan,
)


@app.get("/health")
async def health_check():
    return {"status": "healthy"}


app.include_router(projects.router, prefix="/api/v1")
app.include_router(internal.router, prefix="/internal")
