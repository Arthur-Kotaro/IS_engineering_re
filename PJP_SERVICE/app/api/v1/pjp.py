from fastapi import APIRouter, Depends, HTTPException, Security
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
from sqlalchemy import select, update
from app.database import AsyncSessionLocal
from app.models.pjp import PJP, PJPStatus
import httpx

router = APIRouter()
security = HTTPBearer()

async def get_user_roles(token: str) -> list:
    async with httpx.AsyncClient() as client:
        resp = await client.get(
            "http://localhost:8000/users/me",
            headers={"Authorization": f"Bearer {token}"}
        )
        if resp.status_code != 200:
            raise HTTPException(401, "Invalid token")
        user_data = resp.json()
        return user_data.get("roles", [])

@router.get("/tasks/proctech")
async def get_proctech_tasks(
    credentials: HTTPAuthorizationCredentials = Security(security)
):
    """Список PJP для проработчика"""
    token = credentials.credentials
    roles = await get_user_roles(token)
    
    if "proctech" not in roles:
        raise HTTPException(403, "Only proctech can view this list")
    
    async with AsyncSessionLocal() as db:
        result = await db.execute(
            select(PJP).where(PJP.status == PJPStatus.UPLOADED)
        )
        return result.scalars().all()

@router.patch("/{pjp_id}/approve")
async def approve_pjp(
    pjp_id: int,
    credentials: HTTPAuthorizationCredentials = Security(security)
):
    """Утвердить PJP (проработчик)"""
    token = credentials.credentials
    roles = await get_user_roles(token)
    
    if "proctech" not in roles:
        raise HTTPException(403, "Only proctech can approve PJP")
    
    async with AsyncSessionLocal() as db:
        pjp = await db.get(PJP, pjp_id)
        if not pjp:
            raise HTTPException(404, "PJP not found")
        if pjp.status != PJPStatus.UPLOADED:
            raise HTTPException(400, "PJP is not in uploaded status")
        
        pjp.status = PJPStatus.PROCTECH_REVIEW
        await db.commit()
        return {"message": "PJP approved", "status": pjp.status}

@router.patch("/{pjp_id}/reject")
async def reject_pjp(
    pjp_id: int,
    credentials: HTTPAuthorizationCredentials = Security(security)
):
    """Отклонить PJP (проработчик)"""
    token = credentials.credentials
    roles = await get_user_roles(token)
    
    if "proctech" not in roles:
        raise HTTPException(403, "Only proctech can reject PJP")
    
    async with AsyncSessionLocal() as db:
        pjp = await db.get(PJP, pjp_id)
        if not pjp:
            raise HTTPException(404, "PJP not found")
        if pjp.status != PJPStatus.UPLOADED:
            raise HTTPException(400, "PJP is not in uploaded status")
        
        pjp.status = PJPStatus.PROCTECH_REVIEW  # Или можно создать статус REJECTED
        await db.commit()
        return {"message": "PJP rejected", "status": pjp.status}
