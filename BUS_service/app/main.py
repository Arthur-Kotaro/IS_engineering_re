# app/main.py
from fastapi import FastAPI
from contextlib import asynccontextmanager
from app.database import engine
from app.api.v1 import local_graphs


@asynccontextmanager
async def lifespan(app: FastAPI):
    print("Starting up BUS Service...")
    yield
    await engine.dispose()
    print("Shutting down BUS Service...")


app = FastAPI(
    title="BUS Service (Base Universal Service)",
    description="Управление локальными графиками (ЛГ)",
    version="1.0.0",
    lifespan=lifespan,
)


@app.get("/health")
async def health_check():
    return {"status": "healthy", "service": "bus_service"}


app.include_router(local_graphs.router)
