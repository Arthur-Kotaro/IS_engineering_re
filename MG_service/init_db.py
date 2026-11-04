# init_db.py
import asyncio
from datetime import date
from sqlalchemy import select
from app.database import engine, Base, AsyncSessionLocal
from app.models import TaskGroup, Mastergraph, MgTask, MgDependency, Holiday


async def seed_holidays():
    """Заполнить праздники (если пусто)."""
    async with AsyncSessionLocal() as session:
        result = await session.execute(select(Holiday))
        if result.scalars().first():
            print("ℹ️  Holidays already exist, skipping seed")
            return
        holidays = [
            Holiday(name="Новогодние каникулы", start_date=date(2000, 1, 1), end_date=date(2000, 1, 8), is_yearly=True),
            Holiday(name="День защитника Отечества", start_date=date(2000, 2, 23), is_yearly=True),
            Holiday(name="Международный женский день", start_date=date(2000, 3, 8), is_yearly=True),
            Holiday(name="Праздник Весны и Труда", start_date=date(2000, 5, 1), is_yearly=True),
            Holiday(name="День Победы", start_date=date(2000, 5, 9), is_yearly=True),
            Holiday(name="День России", start_date=date(2000, 6, 12), is_yearly=True),
            Holiday(name="День народного единства", start_date=date(2000, 11, 4), is_yearly=True),
        ]
        session.add_all(holidays)
        await session.commit()
        print(f"✅ Seeded {len(holidays)} holidays")


async def init_db():
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)
    print("✅ Mastergraphics DB created/updated")
    await seed_holidays()


if __name__ == "__main__":
    asyncio.run(init_db())
