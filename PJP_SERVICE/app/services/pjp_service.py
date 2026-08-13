from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from sqlalchemy.orm import selectinload
from app.models.pjp import PJP, PJPPart, PJPStatus, PJPChangeLog
from app.models.draft import PJPDraft, PJPDraftPart
from app.services.validator import validate_pjp
from uuid import UUID
from decimal import Decimal
from datetime import datetime, timezone
from typing import List, Optional, Dict, Any


class PJPService:
    def __init__(self, db: AsyncSession):
        self.db = db
    
    async def _convert_quantity(self, quantity_str: str) -> Decimal:
        """Конвертация строки в Decimal с обработкой ошибок"""
        try:
            # Убираем пробелы и заменяем запятую на точку
            clean = quantity_str.strip().replace(',', '.')
            # Убираем всё после пробела (единицы измерения)
            if ' ' in clean:
                clean = clean.split(' ')[0]
            return Decimal(clean) if clean else Decimal('0')
        except (ValueError, TypeError):
            return Decimal('0')
    
    async def publish_draft(self, draft_id: UUID, user_id: int) -> PJP:
        draft = await self.db.get(PJPDraft, draft_id)
        if not draft:
            raise ValueError("Draft not found")
        if draft.is_published:
            raise ValueError("Draft already published")
        
        parts_result = await self.db.execute(
            select(PJPDraftPart).where(PJPDraftPart.draft_id == draft_id)
        )
        parts_list = parts_result.scalars().all()
        if not parts_list:
            raise ValueError("Draft is empty")
        
        # Валидация
        part_dicts = [self._part_to_dict(part) for part in parts_list]
        is_valid, errors, warnings = validate_pjp(part_dicts)
        if not is_valid:
            raise ValueError(f"Validation failed: {errors}")
        
        # Создаём PJP
        pjp = PJP(
            lot_id=draft.lot_id,
            version="1.0",
            status=PJPStatus.UPLOADED,
            created_by=user_id,
            comment=draft.comment
        )
        self.db.add(pjp)
        await self.db.flush()
        
        # Переносим части
        for draft_part in parts_list:
            part = PJPPart(
                pjp_id=pjp.id,
                line_status=draft_part.line_status,
                gfe_code=draft_part.gfe_code,
                part_number=draft_part.part_number,
                part_name=draft_part.part_name,
                part_name_en=draft_part.part_name_en,
                cd_status=draft_part.cd_status,
                cd_ready_week=draft_part.cd_ready_week,
                quantity=await self._convert_quantity(draft_part.quantity),
                unit=draft_part.unit,
                source_type=draft_part.source_type,
                supplier=draft_part.supplier,
                supplier_code=draft_part.supplier_code,
                parent_model=draft_part.parent_model,
                requires_cd=draft_part.requires_cd,
                comment=draft_part.comment,
                architecture_zone=draft_part.architecture_zone,
                function=draft_part.function,
                pg_number=draft_part.pg_number,
                responsible=draft_part.responsible,
                applies_to_prototypes=draft_part.applies_to_prototypes or []
            )
            self.db.add(part)
        
        # Логируем
        log = PJPChangeLog(
            pjp_id=pjp.id,
            user_id=user_id,
            action="PUBLISHED",
            details={"draft_id": str(draft_id), "warnings": warnings}
        )
        self.db.add(log)
        
        draft.is_published = True
        await self.db.commit()
        await self.db.refresh(pjp)
        return pjp
    
    def _part_to_dict(self, part: PJPDraftPart) -> Dict[str, Any]:
        """Преобразование объекта в словарь для валидации"""
        return {
            "part_number": part.part_number,
            "part_name": part.part_name,
            "part_name_en": part.part_name_en,
            "gfe_code": part.gfe_code,
            "quantity": part.quantity,
            "unit": part.unit,
            "source_type": part.source_type,
            "supplier": part.supplier,
            "supplier_code": part.supplier_code,
            "parent_model": part.parent_model,
            "cd_status": part.cd_status,
            "cd_ready_week": part.cd_ready_week,
            "comment": part.comment,
            "architecture_zone": part.architecture_zone,
            "function": part.function,
            "pg_number": part.pg_number,
            "responsible": part.responsible,
            "applies_to_prototypes": part.applies_to_prototypes or [],
            "line_status": part.line_status,
            "requires_cd": part.requires_cd
        }
    
    async def get_pjp(self, pjp_id: UUID) -> Optional[PJP]:
        result = await self.db.execute(
            select(PJP)
            .where(PJP.id == pjp_id)
            .options(selectinload(PJP.parts))
        )
        return result.unique().scalar_one_or_none()
    
    async def update_status(self, pjp_id: UUID, new_status: PJPStatus, user_id: int, comment: Optional[str] = None) -> PJP:
        pjp = await self.get_pjp(pjp_id)
        if not pjp:
            raise ValueError("PJP not found")
        
        old_status = pjp.status
        pjp.status = new_status
        pjp.updated_at = datetime.now(timezone.utc)
        
        log = PJPChangeLog(
            pjp_id=pjp.id,
            user_id=user_id,
            action="STATUS_CHANGED",
            details={
                "old_status": old_status.value,
                "new_status": new_status.value,
                "comment": comment
            }
        )
        self.db.add(log)
        await self.db.commit()
        await self.db.refresh(pjp)
        return pjp
    
    async def get_pjp_parts(self, pjp_id: UUID, gfe: Optional[str] = None, source_type: Optional[str] = None) -> List[PJPPart]:
        query = select(PJPPart).where(PJPPart.pjp_id == pjp_id)
        if gfe:
            query = query.where(PJPPart.gfe_code == gfe)
        if source_type:
            query = query.where(PJPPart.source_type == source_type)
        result = await self.db.execute(query)
        return result.scalars().all()
    
    async def get_pjp_history(self, pjp_id: UUID) -> List[PJPChangeLog]:
        result = await self.db.execute(
            select(PJPChangeLog)
            .where(PJPChangeLog.pjp_id == pjp_id)
            .order_by(PJPChangeLog.created_at.desc())
        )
        return result.scalars().all()
