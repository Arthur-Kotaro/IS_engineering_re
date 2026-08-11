# app/api/v1/projects.py (МИНИМАЛЬНАЯ ВЕРСИЯ)
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
from sqlalchemy.ext.asyncio import AsyncSession

router = APIRouter(prefix="/api/v1/projects", tags=["Projects"])

async def get_project_service(db: AsyncSession = Depends(get_db)) -> ProjectService:
    return ProjectService(db)

@router.get("/", response_model=List[ProjectResponse])
async def get_projects(
    skip: int = Query(0, ge=0),
    limit: int = Query(100, ge=1, le=500),
    status: Optional[ProjectStatus] = None,
    user_id: Optional[int] = Query(None, description="ID пользователя для фильтрации"),
    service: ProjectService = Depends(get_project_service)
):
    return await service.get_projects(skip, limit, status, user_id)

@router.post("/", response_model=ProjectResponse, status_code=status.HTTP_201_CREATED)
async def create_project(
    data: ProjectCreate,
    created_by: int = Query(..., description="ID пользователя"),
    service: ProjectService = Depends(get_project_service)
):
    return await service.create_project(data, created_by)

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
    service: ProjectService = Depends(get_project_service)
):
    project = await service.update_project(project_id, data)
    if not project:
        raise HTTPException(status_code=404, detail="Project not found")
    return project

@router.patch("/{project_id}/status", response_model=ProjectResponse)
async def update_project_status(
    project_id: int,
    data: ProjectStatusUpdate,
    service: ProjectService = Depends(get_project_service)
):
    project = await service.update_project_status(project_id, data.status)
    if not project:
        raise HTTPException(status_code=404, detail="Project not found")
    return project

@router.delete("/{project_id}", status_code=status.HTTP_204_NO_CONTENT)
async def delete_project(
    project_id: int,
    service: ProjectService = Depends(get_project_service)
):
    deleted = await service.delete_project(project_id)
    if not deleted:
        raise HTTPException(status_code=404, detail="Project not found")

@router.post("/{project_id}/members", response_model=ProjectMemberResponse)
async def add_member(
    project_id: int,
    data: ProjectMemberCreate,
    service: ProjectService = Depends(get_project_service)
):
    member = await service.add_member(project_id, data.user_id, data.role)
    if not member:
        raise HTTPException(status_code=400, detail="User already in project")
    return member

@router.delete("/{project_id}/members/{user_id}", status_code=status.HTTP_204_NO_CONTENT)
async def remove_member(
    project_id: int,
    user_id: int,
    service: ProjectService = Depends(get_project_service)
):
    removed = await service.remove_member(project_id, user_id)
    if not removed:
        raise HTTPException(status_code=404, detail="Member not found")

@router.get("/{project_id}/check-access", response_model=CheckAccessResponse)
async def check_access(
    project_id: int,
    user_id: int = Query(..., description="ID пользователя для проверки"),
    service: ProjectService = Depends(get_project_service)
):
    return await service.check_access(project_id, user_id)
