from fastapi import APIRouter, Depends, HTTPException, Security
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
from sqlalchemy import select
from app.database import AsyncSessionLocal
from app.models.pdm import PDMDocument
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

@router.get("/documents/{reference_number}")
async def get_document(
    reference_number: str,
    credentials: HTTPAuthorizationCredentials = Security(security)
):
    """Получить КД по референсному номеру"""
    async with AsyncSessionLocal() as db:
        doc = await db.get(PDMDocument, reference_number)
        if not doc:
            raise HTTPException(404, "Document not found")
        return {
            "reference_number": doc.reference_number,
            "name": doc.name,
            "description": doc.description,
            "file_url": doc.file_url,
            "version": doc.version,
            "status": doc.status,
            "uploaded_by": doc.uploaded_by,
            "uploaded_at": doc.uploaded_at.isoformat() if doc.uploaded_at else None
        }

@router.post("/documents")
async def create_document(
    doc_data: dict,
    credentials: HTTPAuthorizationCredentials = Security(security)
):
    """Загрузить КД (только PFE/IST)"""
    token = credentials.credentials
    roles = await get_user_roles(token)
    
    if "pfe_ist" not in roles:
        raise HTTPException(403, "Only PFE/IST can upload documents")
    
    async with AsyncSessionLocal() as db:
        # Проверяем, есть ли уже документ с таким номером
        existing = await db.execute(
            select(PDMDocument).where(PDMDocument.reference_number == doc_data["reference_number"])
        )
        if existing.scalar_one_or_none():
            raise HTTPException(400, "Document with this reference number already exists")
        
        doc = PDMDocument(
            reference_number=doc_data["reference_number"],
            name=doc_data["name"],
            description=doc_data.get("description"),
            file_url=doc_data.get("file_url"),
            version=doc_data.get("version", "1.0"),
            uploaded_by=1  # Временный ID, позже нужно брать из токена
        )
        db.add(doc)
        await db.commit()
        return {"message": "Document uploaded", "reference_number": doc.reference_number}
