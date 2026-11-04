from pydantic import BaseModel
from typing import Optional, List
from datetime import date
from uuid import UUID


class HolidayResponse(BaseModel):
    holiday_id: UUID
    name: str
    start_date: date
    end_date: Optional[date]
    is_yearly: bool

    class Config:
        from_attributes = True


class HolidayListResponse(BaseModel):
    holidays: List[HolidayResponse]


class HolidayCreate(BaseModel):
    name: str
    start_date: date
    end_date: Optional[date] = None
    is_yearly: bool = True
