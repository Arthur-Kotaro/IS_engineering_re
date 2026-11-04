# app/api/v1/mastergraphs.py
from fastapi import APIRouter, Depends, HTTPException, Request
from typing import List, Optional
from uuid import UUID
from sqlalchemy.ext.asyncio import AsyncSession

from app.database import get_db
from app.repositories.mastergraph_repo import MastergraphRepository
from app.schemas.mastergraph import (
    MastergraphResponse, MastergraphDetailResponse,
    MastergraphCreate, CanEditResponse,
)
from app.services.access_client import check_project_access, list_accessible_projects

router = APIRouter(prefix="/mastergraphs", tags=["Mastergraphs"])


def _user_id(request: Request) -> int:
    uid = request.headers.get("X-User-ID")
    if not uid:
        raise HTTPException(401, "Missing X-User-ID")
    return int(uid)


def _token(request: Request) -> Optional[str]:
    auth = request.headers.get("Authorization")
    return auth.removeprefix("Bearer ").strip() if auth else None


@router.get("", response_model=List[MastergraphResponse])
async def list_mastergraphs(
    request: Request,
    permission: str = "view_project",
    db: AsyncSession = Depends(get_db),
):
    """Список МГ, доступных пользователю (с учётом прав)."""
    user_id = _user_id(request)
    token = _token(request)

    accessible = await list_accessible_projects(user_id, permission, token)
    project_ids = [p["project_id"] for p in accessible]
    if not project_ids:
        return []

    repo = MastergraphRepository(db)
    return await repo.list_by_project_ids(project_ids)


@router.get("/{mg_id}", response_model=MastergraphDetailResponse)
async def get_mastergraph(
    mg_id: UUID,
    request: Request,
    db: AsyncSession = Depends(get_db),
):
    """Полный граф МГ (задачи, зависимости, группы)."""
    user_id = _user_id(request)
    token = _token(request)

    repo = MastergraphRepository(db)
    mg = await repo.get_by_id(mg_id)
    if not mg:
        raise HTTPException(404, "Mastergraph not found")

    access = await check_project_access(mg.project_id, user_id, "view_project", token)
    if not access.get("has_access"):
        raise HTTPException(403, "Access denied")

    tasks = await repo.get_tasks(mg_id)
    deps = await repo.get_dependencies(mg_id)
    groups = await repo.get_groups_for_mg(mg_id)

    return {
        **{c.name: getattr(mg, c.name) for c in mg.__table__.columns},
        "tasks": tasks,
        "dependencies": deps,
        "groups": groups,
    }


@router.get("/{mg_id}/access", response_model=CanEditResponse)
async def check_mg_access(
    mg_id: UUID,
    request: Request,
    db: AsyncSession = Depends(get_db),
):
    """Проверка прав на редактирование/просмотр конкретного МГ."""
    user_id = _user_id(request)
    token = _token(request)

    repo = MastergraphRepository(db)
    mg = await repo.get_by_id(mg_id)
    if not mg:
        raise HTTPException(404, "Mastergraph not found")

    edit = await check_project_access(mg.project_id, user_id, "edit_mastergraphic", token)
    view = await check_project_access(mg.project_id, user_id, "view_project", token)

    return {
        "can_edit": bool(edit.get("has_access")),
        "can_view": bool(view.get("has_access")),
        "reason": edit.get("reason"),
    }


@router.post("", response_model=MastergraphResponse, status_code=201)
async def create_mastergraph(
    data: MastergraphCreate,
    request: Request,
    db: AsyncSession = Depends(get_db),
):
    user_id = _user_id(request)
    token = _token(request)

    access = await check_project_access(data.project_id, user_id, "edit_mastergraphic", token)
    if not access.get("has_access"):
        raise HTTPException(403, "Access denied")

    repo = MastergraphRepository(db)
    existing = await repo.get_by_project_id(data.project_id)
    if existing:
        raise HTTPException(409, "Mastergraph for this project already exists")

    return await repo.create(data.project_id, data.name, user_id)
