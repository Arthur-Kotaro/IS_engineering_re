# app/api/v1/diff.py
import logging
from datetime import date
from typing import Optional, List
from uuid import UUID
from fastapi import APIRouter, Depends, HTTPException, Request
from pydantic import BaseModel
from sqlalchemy.ext.asyncio import AsyncSession

from app.database import get_db
from app.repositories.mastergraph_repo import MastergraphRepository
from app.services.access_client import check_project_access
from app.services import mastergraph_service as mgsvc
from app.ws.manager import manager

logger = logging.getLogger(__name__)
router = APIRouter(prefix="/mastergraphs", tags=["Diff"])


def _user_id(request: Request) -> int:
    uid = request.headers.get("X-User-ID")
    if not uid:
        raise HTTPException(401, "Missing X-User-ID")
    return int(uid)


def _token(request: Request) -> Optional[str]:
    auth = request.headers.get("Authorization")
    return auth.removeprefix("Bearer ").strip() if auth else None


class DiffChange(BaseModel):
    entity: str            # task | milestone
    op: str                # update
    id: str                # uuid
    fields: dict           # {field: new_value}
    reason: Optional[str] = None


class DiffRequest(BaseModel):
    changes: List[DiffChange]
    reason: Optional[str] = None


@router.put("/{mg_id}/diff")
async def apply_diff(mg_id: UUID, data: DiffRequest, request: Request, db: AsyncSession = Depends(get_db)):
    user_id = _user_id(request)
    token = _token(request)

    repo = MastergraphRepository(db)
    mg = await repo.get_by_id(mg_id)
    if not mg:
        raise HTTPException(404, "Mastergraph not found")

    access = await check_project_access(mg.project_id, user_id, "edit_mastergraphic", token)
    if not access.get("has_access"):
        raise HTTPException(403, "Access denied")

    applied = 0
    errors = []

    for ch in data.changes:
        try:
            if ch.entity == "task":
                fields = {}
                for k, v in ch.fields.items():
                    if k in ("start_date", "end_date") and v:
                        fields[k] = date.fromisoformat(v) if isinstance(v, str) else v
                    else:
                        fields[k] = v
                await mgsvc.update_task(db, mg_id, UUID(ch.id), user_id, token, fields, ch.reason or data.reason)
                applied += 1
            elif ch.entity == "milestone":
                new_date = None
                if "planned_date" in ch.fields and ch.fields["planned_date"]:
                    new_date = date.fromisoformat(ch.fields["planned_date"])
                await mgsvc.update_milestone(db, mg_id, UUID(ch.id), user_id, token, new_date, ch.reason or data.reason)
                applied += 1
            else:
                errors.append({"id": ch.id, "error": f"Unknown entity: {ch.entity}"})
        except mgsvc.ReasonRequiredError as e:
            errors.append({"id": ch.id, "error": "reason_required", "detail": str(e)})
        except Exception as e:
            logger.warning(f"diff apply error for {ch.id}: {e}")
            errors.append({"id": ch.id, "error": str(e)})

    if applied > 0:
        await db.commit()
        # broadcast
        await manager.broadcast(str(mg_id), {
            "type": "diff",
            "changes": [{"op": "update", "entity": ch.entity, "id": ch.id, "fields": ch.fields} for ch in data.changes if not any(e["id"] == ch.id for e in errors)],
        })

    if errors and applied == 0:
        raise HTTPException(400, {"applied": 0, "errors": errors})

    return {"status": "ok", "applied": applied, "errors": errors}
