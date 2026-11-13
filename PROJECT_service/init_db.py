# init_db.py
import asyncio
from app.database import engine, Base
from app.models import (
    Project, ProjectMember, ProjectRole, ProjectType, Typology, MgCreationQueue,
)


async def init_db():
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)
    print("✅ Project DB created/updated")


if __name__ == "__main__":
    asyncio.run(init_db())
