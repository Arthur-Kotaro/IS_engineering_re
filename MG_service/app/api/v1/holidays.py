from fastapi import APIRouter, Depends
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select

from app.database import get_db
from app.models.holiday import Holiday
from app.schemas.holiday import HolidayResponse, HolidayListResponse, HolidayCreate

router = APIRouter(prefix="/holidays", tags=["Holidays"])


@router.get("", response_model=HolidayListResponse)
async def list_holidays(db: AsyncSession = Depends(get_db)):
    """Список праздников (для построения графика)."""
    result = await db.execute(select(Holiday).order_by(Holiday.start_date))
    holidays = result.scalars().all()
    return {"holidays": holidays}


@router.put("", response_model=HolidayListResponse)
async def replace_holidays(data: list[HolidayCreate], db: AsyncSession = Depends(get_db)):
    """Заменить весь список праздников."""
    # Удалить все существующие
    await db.execute(select(Holiday))
    result = await db.execute(select(Holiday))
    for h in result.scalars().all():
        await db.delete(h)
    # Добавить новые
    for item in data:
        h = Holiday(
            name=item.name,
            start_date=item.start_date,
            end_date=item.end_date,
            is_yearly=item.is_yearly,
        )
        db.add(h)
    await db.commit()
    result = await db.execute(select(Holiday).order_by(Holiday.start_date))
    holidays = result.scalars().all()
    return {"holidays": holidays}
