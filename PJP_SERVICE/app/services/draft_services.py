from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, update, delete
from sqlalchemy.orm import selectinload
from app.models.draft import PJPDraft, PJPDraftPart
from app.schemas.draft import PJPDraftCreate
from uuid import UUID


class DraftService:
    def __init__(self, db: AsyncSession):
        self.db = db

    async def create_draft(self, data: PJPDraftCreate, user_id: int) -> PJPDraft:
        draft = PJPDraft(
            lot_id=data.lot_id,
            created_by=user_id,
            comment=data.comment
        )
        self.db.add(draft)
        await self.db.commit()
        await self.db.refresh(draft)
        return draft

    async def get_draft(self, draft_id: UUID) -> PJPDraft | None:
        result = await self.db.execute(
            select(PJPDraft)
            .where(PJPDraft.id == draft_id)
            .options(selectinload(PJPDraft.parts))
        )
        return result.scalar_one_or_none()

    async def add_part(self, draft_id: UUID, part_data: dict) -> PJPDraftPart:
        draft = await self.get_draft(draft_id)
        if not draft:
            raise ValueError("Draft not found")
        if draft.is_published:
            raise ValueError("Cannot add part to published draft")

        part = PJPDraftPart(
            draft_id=draft_id,
            **part_data
        )
        self.db.add(part)
        await self.db.commit()
        await self.db.refresh(part)
        return part

    async def update_part(self, draft_id: UUID, part_id: UUID, part_data: dict) -> PJPDraftPart | None:
        part = await self.db.get(PJPDraftPart, part_id)
        if not part or part.draft_id != draft_id:
            return None
        
        draft = await self.get_draft(draft_id)
        if draft.is_published:
            raise ValueError("Cannot update part in published draft")

        for key, value in part_data.items():
            if hasattr(part, key):
                setattr(part, key, value)
        
        await self.db.commit()
        await self.db.refresh(part)
        return part

    async def delete_part(self, draft_id: UUID, part_id: UUID) -> bool:
        draft = await self.get_draft(draft_id)
        if draft.is_published:
            raise ValueError("Cannot delete part from published draft")

        result = await self.db.execute(
            delete(PJPDraftPart)
            .where(PJPDraftPart.id == part_id, PJPDraftPart.draft_id == draft_id)
        )
        await self.db.commit()
        return result.rowcount > 0

    async def clear_draft(self, draft_id: UUID) -> bool:
        draft = await self.get_draft(draft_id)
        if draft.is_published:
            raise ValueError("Cannot clear published draft")

        result = await self.db.execute(
            delete(PJPDraftPart)
            .where(PJPDraftPart.draft_id == draft_id)
        )
        await self.db.commit()
        return result.rowcount > 0

    async def get_draft_parts(self, draft_id: UUID) -> list[PJPDraftPart]:
        result = await self.db.execute(
            select(PJPDraftPart)
            .where(PJPDraftPart.draft_id == draft_id)
        )
        return result.scalars().all()

    async def get_user_drafts(self, user_id: int) -> list[PJPDraft]:
        result = await self.db.execute(
            select(PJPDraft)
            .where(PJPDraft.created_by == user_id, PJPDraft.is_published == False)
            .order_by(PJPDraft.updated_at.desc())
        )
        return result.scalars().all()
