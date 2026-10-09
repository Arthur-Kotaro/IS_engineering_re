# app/api/v1/internal.py
from fastapi import APIRouter, Depends, Query, Request
from typing import List, Optional
from app.database import get_db
from app.schemas.project import ProjectResponse, CheckAccessResponse
from app.services.project_service import ProjectService
from sqlalchemy.ext.asyncio import AsyncSession

router = APIRouter(tags=["Internal"])


def _token(request: Request) -> Optional[str]:
    auth = request.headers.get("Authorization")
    if not auth:
        return None
    return auth.removeprefix("Bearer ").strip()


@router.get("/projects/list-with-access", response_model=List[ProjectResponse])
async def list_with_access(
    user_id: int = Query(...),
    permission: str = Query("view_project"),
    request: Request = None,
    db: AsyncSession = Depends(get_db),
):
    return await ProjectService(db).list_with_access(user_id, permission, _token(request))


@router.get("/projects/{project_id}/check-access", response_model=CheckAccessResponse)
async def check_access(
    project_id: int,
    user_id: int = Query(...),
    permission: str = Query("view_project"),
    request: Request = None,
    db: AsyncSession = Depends(get_db),
):
    return await ProjectService(db).check_access(project_id, user_id, permission, _token(request))
