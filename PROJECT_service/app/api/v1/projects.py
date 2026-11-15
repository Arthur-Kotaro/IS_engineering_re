# app/api/v1/projects.py
from fastapi import APIRouter, Depends, HTTPException, Request, Query
from typing import List, Optional
from sqlalchemy.ext.asyncio import AsyncSession

from app.database import get_db
from app.schemas.project import (
    ProjectCreate, ProjectUpdate, ProjectResponse,
    ProjectDetailResponse, ProjectMemberCreate, ProjectMemberResponse,
    CheckAccessResponse,
)
from app.services.project_service import ProjectService
from app.models.project import ProjectStatus

router = APIRouter(prefix="/projects", tags=["Projects"])


def _user_id(request: Request) -> int:
    uid = request.headers.get("X-User-ID")
    if not uid:
        raise HTTPException(401, "Missing X-User-ID")
    return int(uid)


def _token(request: Request) -> Optional[str]:
    auth = request.headers.get("Authorization")
    return auth.removeprefix("Bearer ").strip() if auth else None


@router.post("/create", response_model=ProjectResponse, status_code=201)
async def create_project(data: ProjectCreate, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    svc = ProjectService(db)
    return await svc.create_project(data, created_by=user_id)


@router.get("/list", response_model=List[ProjectResponse])
async def list_projects(
    skip: int = 0,
    limit: int = 100,
    status_filter: Optional[str] = None,
    db: AsyncSession = Depends(get_db),
):
    svc = ProjectService(db)
    return await svc.list_projects(skip, limit, status_filter)


@router.get("/list-with-access", response_model=List[ProjectResponse])
async def list_with_access(
    request: Request,
    permission: str = "view_project",
    db: AsyncSession = Depends(get_db),
):
    user_id = _user_id(request)
    token = _token(request)
    svc = ProjectService(db)
    return await svc.list_with_access(user_id, permission, token)


@router.get("/{project_id}", response_model=ProjectDetailResponse)
async def get_project(project_id: int, db: AsyncSession = Depends(get_db)):
    svc = ProjectService(db)
    p = await svc.get_project(project_id)
    if not p:
        raise HTTPException(404, "Project not found")
    return p


@router.put("/{project_id}", response_model=ProjectResponse)
async def update_project(
    project_id: int,
    data: ProjectUpdate,
    request: Request,
    db: AsyncSession = Depends(get_db),
):
    user_id = _user_id(request)
    token = _token(request)
    svc = ProjectService(db)
    p = await svc.update_project(project_id, data, user_id, token)
    if not p:
        raise HTTPException(404, "Project not found")
    return p


@router.patch("/{project_id}/status", response_model=ProjectResponse)
async def patch_project_status(
    project_id: int,
    status: str = Query(..., description="Новый статус проекта"),
    request: Request = None,
    db: AsyncSession = Depends(get_db),
):
    """Смена статуса проекта. Валидные статусы: draft, active, suspended, completed, cancelled."""
    valid_statuses = {
        ProjectStatus.DRAFT,
        ProjectStatus.ACTIVE,
        ProjectStatus.SUSPENDED,
        ProjectStatus.COMPLETED,
        ProjectStatus.CANCELLED,
    }
    if status not in valid_statuses:
        raise HTTPException(
            400,
            f"Invalid status '{status}'. Allowed: {', '.join(sorted(valid_statuses))}",
        )

    user_id = _user_id(request)
    token = _token(request)
    svc = ProjectService(db)
    data = ProjectUpdate(status=status)
    p = await svc.update_project(project_id, data, user_id, token)
    if not p:
        raise HTTPException(404, "Project not found")
    return p


@router.delete("/{project_id}", status_code=204)
async def delete_project(
    project_id: int,
    request: Request,
    db: AsyncSession = Depends(get_db),
):
    user_id = _user_id(request)
    token = _token(request)
    svc = ProjectService(db)
    await svc.delete_project(project_id, user_id, token)
    return None


@router.get("/{project_id}/check-access", response_model=CheckAccessResponse)
async def check_access(
    project_id: int,
    request: Request,
    user_id: Optional[int] = None,
    permission: str = "view_project",
    db: AsyncSession = Depends(get_db),
):
    # user_id можно передать в query (для MG → Project Service), иначе — из заголовка
    if user_id is None:
        user_id = _user_id(request)
    token = _token(request)
    svc = ProjectService(db)
    return await svc.check_access(project_id, user_id, permission, token)


@router.post("/{project_id}/members", response_model=ProjectMemberResponse, status_code=201)
async def add_member(
    project_id: int,
    data: ProjectMemberCreate,
    request: Request,
    db: AsyncSession = Depends(get_db),
):
    user_id = _user_id(request)
    token = _token(request)
    svc = ProjectService(db)
    return await svc.add_member(project_id, data, user_id, token)


@router.delete("/{project_id}/members/{member_user_id}", status_code=204)
async def remove_member(
    project_id: int,
    member_user_id: int,
    request: Request,
    db: AsyncSession = Depends(get_db),
):
    user_id = _user_id(request)
    token = _token(request)
    svc = ProjectService(db)
    await svc.remove_member(project_id, member_user_id, user_id, token)
    return None
