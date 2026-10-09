# app/api/v1/projects.py
from fastapi import APIRouter, Depends, HTTPException, Query, Request
from typing import List, Optional
from app.database import get_db
from app.schemas.project import (
    ProjectCreate, ProjectUpdate,
    ProjectResponse, ProjectDetailResponse,
    ProjectMemberCreate, ProjectMemberResponse,
    CheckAccessResponse, ProjectRoleResponse,
)
from app.services.project_service import ProjectService
from app.repositories.project_repo import ProjectRepository
from sqlalchemy.ext.asyncio import AsyncSession

router = APIRouter(prefix="/projects", tags=["Projects"])


def _user_id(request: Request) -> int:
    uid = request.headers.get("X-User-ID")
    if not uid:
        raise HTTPException(401, "Missing X-User-ID")
    return int(uid)


def _token(request: Request) -> Optional[str]:
    auth = request.headers.get("Authorization")
    if not auth:
        return None
    return auth.removeprefix("Bearer ").strip()


@router.get("/list", response_model=List[ProjectResponse])
async def list_projects(
    skip: int = Query(0, ge=0),
    limit: int = Query(100, ge=1, le=500),
    status_filter: Optional[str] = None,
    db: AsyncSession = Depends(get_db),
):
    return await ProjectService(db).list_projects(skip, limit, status_filter)


@router.post("/create", response_model=ProjectResponse, status_code=201)
async def create_project(
    data: ProjectCreate,
    request: Request,
    db: AsyncSession = Depends(get_db),
):
    return await ProjectService(db).create_project(data, _user_id(request))


@router.get("/list-with-access", response_model=List[ProjectResponse])
async def list_with_access(
    permission: str = Query("view_project"),
    request: Request = None,
    db: AsyncSession = Depends(get_db),
):
    return await ProjectService(db).list_with_access(_user_id(request), permission, _token(request))


@router.get("/roles", response_model=List[ProjectRoleResponse])
async def list_project_roles(db: AsyncSession = Depends(get_db)):
    repo = ProjectRepository(db)
    return await repo.list_roles()


@router.get("/{project_id}", response_model=ProjectDetailResponse)
async def get_project(project_id: int, db: AsyncSession = Depends(get_db)):
    p = await ProjectService(db).get_project(project_id)
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
    return await ProjectService(db).update_project(project_id, data, _user_id(request), _token(request))


@router.post("/{project_id}/members", response_model=ProjectMemberResponse)
async def add_member(
    project_id: int,
    data: ProjectMemberCreate,
    request: Request,
    db: AsyncSession = Depends(get_db),
):
    return await ProjectService(db).add_member(project_id, data, _user_id(request), _token(request))


@router.delete("/{project_id}/members/{user_id}", status_code=204)
async def remove_member(
    project_id: int,
    user_id: int,
    request: Request,
    db: AsyncSession = Depends(get_db),
):
    await ProjectService(db).remove_member(project_id, user_id, _user_id(request), _token(request))


@router.get("/{project_id}/check-access", response_model=CheckAccessResponse)
async def check_access(
    project_id: int,
    user_id: int = Query(...),
    permission: str = Query("view_project"),
    request: Request = None,
    db: AsyncSession = Depends(get_db),
):
    return await ProjectService(db).check_access(project_id, user_id, permission, _token(request))
