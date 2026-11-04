# app/main.py
from fastapi import FastAPI
from contextlib import asynccontextmanager
from app.database import engine
from app.api.v1 import mastergraphs, internal
from app.api.v1 import mastergraphs, internal, holidays


@asynccontextmanager
async def lifespan(app: FastAPI):
    print("Starting up Mastergraphics Service...")
    yield
    await engine.dispose()
    print("Shutting down Mastergraphics Service...")


app = FastAPI(
    title="Mastergraphics Service",
    description="Сервис управления мастерграфиками",
    version="1.0.0",
    lifespan=lifespan,
)


@app.get("/health")
async def health_check():
    return {"status": "healthy", "service": "mastergraph_service"}


app.include_router(mastergraphs.router, prefix="/api/v1")
app.include_router(internal.router, prefix="/internal")
app.include_router(holidays.router, prefix="/api/v1")
