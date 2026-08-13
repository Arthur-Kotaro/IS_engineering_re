from fastapi import APIRouter, Depends, HTTPException, Security, status
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
from app.database import get_db
from app.schemas.pjp import PJPResponse, PJPStatusUpdate, PJPPartResponse
from app.services.pjp_service import PJPService
from sqlalchemy.ext.asyncio import AsyncSession
from uuid import UUID

router = APIRouter()
security = HTTPBearer()


@router.get("/{pjp_id}", response_model=PJPResponse)
async def get_pjp(
    pjp_id: UUID,
    credentials: HTTPAuthorizationCredentials = Security(security),
    db: AsyncSession = Depends(get_db)
):
    """Получить PJP по ID"""
    service = PJPService(db)
    pjp = await service.get_pjp(pjp_id)
    if not pjp:
        raise HTTPException(status_code=404, detail="PJP not found")
    return pjp


@router.patch("/{pjp_id}/status")
async def update_pjp_status(
    pjp_id: UUID,
    data: PJPStatusUpdate,
    credentials: HTTPAuthorizationCredentials = Security(security),
    db: AsyncSession = Depends(get_db)
):
    """Обновить статус PJP"""
    user_id = 1  # Временно
    service = PJPService(db)
    try:
        pjp = await service.update_status(pjp_id, data.status, user_id, data.comment)
        return {"message": f"Status updated to {data.status.value}", "pjp_id": pjp.id}
    except ValueError as e:
        raise HTTPException(status_code=404, detail=str(e))


@router.get("/{pjp_id}/parts", response_model=list[PJPPartResponse])
async def get_pjp_parts(
    pjp_id: UUID,
    gfe: str = None,
    source_type: str = None,
    credentials: HTTPAuthorizationCredentials = Security(security),
    db: AsyncSession = Depends(get_db)
):
    """Получить части PJP с фильтрацией"""
    service = PJPService(db)
    parts = await service.get_pjp_parts(pjp_id, gfe, source_type)
    return parts


@router.get("/{pjp_id}/history")
async def get_pjp_history(
    pjp_id: UUID,
    credentials: HTTPAuthorizationCredentials = Security(security),
    db: AsyncSession = Depends(get_db)
):
    """Получить историю изменений PJP"""
    service = PJPService(db)
    history = await service.get_pjp_history(pjp_id)
    return history
