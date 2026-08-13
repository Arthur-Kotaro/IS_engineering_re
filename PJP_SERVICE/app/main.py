from fastapi import FastAPI
from contextlib import asynccontextmanager
from app.database import engine, Base
from app.api.v1 import pjp, draft

@asynccontextmanager
async def lifespan(app: FastAPI):
    # Startup
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)
    yield
    # Shutdown
    await engine.dispose()

app = FastAPI(
    title="PJP Service",
    description="Service for managing prototype parts lists (PJP)",
    version="1.0.0",
    lifespan=lifespan
)

app.include_router(pjp.router, prefix="/api/v1/pjp", tags=["PJP"])
app.include_router(draft.router, prefix="/api/v1/pjp", tags=["Drafts"])

@app.get("/health")
async def health():
    return {"status": "healthy"}
