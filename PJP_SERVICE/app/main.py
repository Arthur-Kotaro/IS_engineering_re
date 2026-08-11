from fastapi import FastAPI
from app.api.v1 import pjp, pdm
from app.database import engine, Base

app = FastAPI(title="PJP + PDM Service", version="1.0.0")

@app.on_event("startup")
async def startup():
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)

app.include_router(pjp.router, prefix="/api/v1/pjp", tags=["PJP"])
app.include_router(pdm.router, prefix="/api/v1/pdm", tags=["PDM"])

@app.get("/health")
async def health():
    return {"status": "healthy"}
