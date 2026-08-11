from fastapi import FastAPI
from app.api.v1 import navigation
from app.database import engine, Base

app = FastAPI(title="Navigation Service", version="1.0.0")

@app.on_event("startup")
async def startup():
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)

app.include_router(navigation.router, prefix="/api/v1/navigation", tags=["Navigation"])

@app.get("/health")
async def health():
    return {"status": "healthy"}
