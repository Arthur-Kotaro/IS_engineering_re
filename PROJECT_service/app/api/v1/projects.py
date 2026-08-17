# app/api/v1/projects.py
from fastapi import APIRouter, Depends, HTTPException, status, Query
from typing import List, Optional
from app.database import get_db
from app.schemas.project import (
    ProjectCreate, ProjectUpdate, ProjectStatusUpdate,
    ProjectResponse, ProjectDetailResponse, ProjectMemberCreate,
    ProjectMemberResponse, CheckAccessResponse
)
from app.services.project_service import ProjectService
from app.models.project import ProjectStatus
from app.dependencies import get_current_user_id
from sqlalchemy.ext.asyncio import AsyncSession

router = APIRouter(prefix="/projects", tags=["Projects"])

async def get_project_service(db: AsyncSession = Depends(get_db)) -> ProjectService:
    return ProjectService(db)

# ============================================================
# СПЕЦИФИЧНЫЕ МАРШРУТЫ (без параметров) - ПЕРВЫЕ
# ============================================================

@router.get("/list", response_model=List[ProjectResponse])
async def get_projects(
    skip: int = Query(0, ge=0),
    limit: int = Query(100, ge=1, le=500),
    status: Optional[ProjectStatus] = None,
    user_id: Optional[int] = Query(None, description="ID пользователя для фильтрации"),
    service: ProjectService = Depends(get_project_service)
):
    """Получить список проектов."""
    return await service.get_projects(skip, limit, status, user_id)

@router.post("/create", response_model=ProjectResponse, status_code=status.HTTP_201_CREATED)
async def create_project(
    data: ProjectCreate,
    current_user_id: int = Depends(get_current_user_id),
    service: ProjectService = Depends(get_project_service)
):
    """Создать новый проект."""
    return await service.create_project(data, current_user_id)

# ============================================================
# МАРШРУТЫ С ПАРАМЕТРАМИ
# ============================================================

@router.get("/{project_id}", response_model=ProjectDetailResponse)
async def get_project(
    project_id: int,
    service: ProjectService = Depends(get_project_service)
):
    project = await service.get_project(project_id)
    if not project:
        raise HTTPException(status_code=404, detail="Project not found")
    return project

@router.put("/{project_id}", response_model=ProjectResponse)
async def update_project(
    project_id: int,
    data: ProjectUpdate,
    current_user_id: int = Depends(get_current_user_id),
    service: ProjectService = Depends(get_project_service)
):
    project = await service.update_project(project_id, data, current_user_id)
    if not project:
        raise HTTPException(status_code=404, detail="Project not found or not owner")
    return project

@router.patch("/{project_id}/status", response_model=ProjectResponse)
async def update_project_status(
    project_id: int,
    data: ProjectStatusUpdate,
    current_user_id: int = Depends(get_current_user_id),
    service: ProjectService = Depends(get_project_service)
):
    project = await service.update_project_status(project_id, data.status, current_user_id)
    if not project:
        raise HTTPException(status_code=404, detail="Project not found or not owner")
    return project

@router.delete("/{project_id}", status_code=status.HTTP_204_NO_CONTENT)
async def delete_project(
    project_id: int,
    current_user_id: int = Depends(get_current_user_id),
    service: ProjectService = Depends(get_project_service)
):
    deleted = await service.delete_project(project_id, current_user_id)
    if not deleted:
        raise HTTPException(status_code=404, detail="Project not found or not owner")

@router.post("/{project_id}/members", response_model=ProjectMemberResponse)
async def add_member(
    project_id: int,
    data: ProjectMemberCreate,
    current_user_id: int = Depends(get_current_user_id),
    service: ProjectService = Depends(get_project_service)
):
    member = await service.add_member(project_id, data.user_id, data.role, current_user_id)
    if not member:
        raise HTTPException(status_code=400, detail="User already in project or not owner")
    return member

@router.delete("/{project_id}/members/{user_id}", status_code=status.HTTP_204_NO_CONTENT)
async def remove_member(
    project_id: int,
    user_id: int,
    current_user_id: int = Depends(get_current_user_id),
    service: ProjectService = Depends(get_project_service)
):
    removed = await service.remove_member(project_id, user_id, current_user_id)
    if not removed:
        raise HTTPException(status_code=404, detail="Member not found or not owner")

@router.get("/{project_id}/check-access", response_model=CheckAccessResponse)
async def check_access(
    project_id: int,
    user_id: int = Query(..., description="ID пользователя для проверки"),
    service: ProjectService = Depends(get_project_service)
):
    return await service.check_access(project_id, user_id)
