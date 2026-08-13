from fastapi import APIRouter, Depends, HTTPException, Security, status
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
from app.database import get_db
from app.schemas.draft import (
    PJPDraftCreate, PJPDraftResponse, PJPDraftPublishResponse,
    PJPDraftPartCreate, PJPDraftPartUpdate
)
from app.services.draft_services import DraftService
from app.services.pjp_service import PJPService
from sqlalchemy.ext.asyncio import AsyncSession
from uuid import UUID

router = APIRouter()
security = HTTPBearer()


@router.post("/drafts", response_model=PJPDraftResponse, status_code=status.HTTP_201_CREATED)
async def create_draft(
    data: PJPDraftCreate,
    credentials: HTTPAuthorizationCredentials = Security(security),
    db: AsyncSession = Depends(get_db)
):
    """Создать новый черновик"""
    user_id = 1  # Временно, позже из токена
    service = DraftService(db)
    draft = await service.create_draft(data, user_id)
    return draft


@router.get("/drafts", response_model=list[PJPDraftResponse])
async def get_user_drafts(
    credentials: HTTPAuthorizationCredentials = Security(security),
    db: AsyncSession = Depends(get_db)
):
    """Получить все черновики пользователя"""
    user_id = 1  # Временно
    service = DraftService(db)
    drafts = await service.get_user_drafts(user_id)
    return drafts


@router.get("/drafts/{draft_id}", response_model=PJPDraftResponse)
async def get_draft(
    draft_id: UUID,
    credentials: HTTPAuthorizationCredentials = Security(security),
    db: AsyncSession = Depends(get_db)
):
    """Получить черновик по ID"""
    service = DraftService(db)
    draft = await service.get_draft(draft_id)
    if not draft:
        raise HTTPException(status_code=404, detail="Draft not found")
    return draft


@router.post("/drafts/{draft_id}/parts", response_model=PJPDraftPartCreate)
async def add_draft_part(
    draft_id: UUID,
    data: PJPDraftPartCreate,
    credentials: HTTPAuthorizationCredentials = Security(security),
    db: AsyncSession = Depends(get_db)
):
    """Добавить часть в черновик"""
    service = DraftService(db)
    try:
        part = await service.add_part(draft_id, data.part.model_dump())
        return part
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))


@router.put("/drafts/{draft_id}/parts/{part_id}")
async def update_draft_part(
    draft_id: UUID,
    part_id: UUID,
    data: PJPDraftPartUpdate,
    credentials: HTTPAuthorizationCredentials = Security(security),
    db: AsyncSession = Depends(get_db)
):
    """Обновить часть в черновике"""
    service = DraftService(db)
    try:
        part = await service.update_part(draft_id, part_id, data.part.model_dump())
        if not part:
            raise HTTPException(status_code=404, detail="Part not found")
        return part
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))


@router.delete("/drafts/{draft_id}/parts/{part_id}")
async def delete_draft_part(
    draft_id: UUID,
    part_id: UUID,
    credentials: HTTPAuthorizationCredentials = Security(security),
    db: AsyncSession = Depends(get_db)
):
    """Удалить часть из черновика"""
    service = DraftService(db)
    try:
        deleted = await service.delete_part(draft_id, part_id)
        if not deleted:
            raise HTTPException(status_code=404, detail="Part not found")
        return {"message": "Part deleted"}
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))


@router.delete("/drafts/{draft_id}/clear")
async def clear_draft(
    draft_id: UUID,
    credentials: HTTPAuthorizationCredentials = Security(security),
    db: AsyncSession = Depends(get_db)
):
    """Очистить все части черновика"""
    service = DraftService(db)
    try:
        await service.clear_draft(draft_id)
        return {"message": "Draft cleared"}
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))


@router.post("/drafts/{draft_id}/publish", response_model=PJPDraftPublishResponse)
async def publish_draft(
    draft_id: UUID,
    credentials: HTTPAuthorizationCredentials = Security(security),
    db: AsyncSession = Depends(get_db)
):
    """Опубликовать черновик (создать PJP)"""
    user_id = 1  # Временно
    pjp_service = PJPService(db)
    try:
        pjp = await pjp_service.publish_draft(draft_id, user_id)
        return PJPDraftPublishResponse(
            message="PJP published successfully",
            pjp_id=pjp.id,
            status=pjp.status.value,
            warnings=[]  # TODO: передать warnings из валидации
        )
    except ValueError as e:
        raise HTTPException(status_code=422, detail=str(e))
