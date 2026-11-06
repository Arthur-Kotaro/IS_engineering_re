# init_db.py
import asyncio
from app.database import engine, Base
from app.models import LocalGraph, LgTask, LgDependency


async def init_db():
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)
    print("✅ BUS DB created/updated")


if __name__ == "__main__":
    asyncio.run(init_db())
