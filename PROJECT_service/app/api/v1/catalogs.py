# app/api/v1/catalogs.py
from fastapi import APIRouter, Depends
from typing import List, Optional
from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession

from app.database import get_db
from app.models.project_type import ProjectType
from app.models.typology import Typology
from app.schemas.project import ProjectTypeResponse, TypologyResponse

router = APIRouter(tags=["Catalogs"])


@router.get("/project-types", response_model=List[ProjectTypeResponse])
async def list_project_types(db: AsyncSession = Depends(get_db)):
    result = await db.execute(
        select(ProjectType).where(ProjectType.is_active == True).order_by(ProjectType.sort_order)
    )
    return result.scalars().all()


@router.get("/typologies", response_model=List[TypologyResponse])
async def list_typologies(project_type: Optional[str] = None, db: AsyncSession = Depends(get_db)):
    q = select(Typology).where(Typology.is_active == True)
    if project_type:
        q = q.where(Typology.project_type == project_type)
    q = q.order_by(Typology.sort_order)
    result = await db.execute(q)
    return result.scalars().all()
