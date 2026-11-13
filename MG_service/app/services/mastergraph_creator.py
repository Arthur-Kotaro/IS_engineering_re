# app/services/mastergraph_creator.py
import logging
from datetime import date
from typing import Optional
from uuid import UUID
from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession

from app.models.mastergraph import Mastergraph
from app.models.mg_event import MgEntityType, MgEventType
from app.repositories.event_repo import EventRepository
from app.repositories.milestone_repo import MilestoneRepository
from app.services.typology_loader import load_typology, build_milestones

logger = logging.getLogger(__name__)


class MastergraphCreationError(Exception):
    pass


async def create_mastergraph_from_typology(
    db: AsyncSession,
    project_id: int,
    project_type: str,
    typology_name: str,
    cve_id: int,
    ipp_id: int,
    name: Optional[str] = None,
    start_date: Optional[date] = None,
) -> Mastergraph:
    """
    Создаёт МГ (идемпотентно по project_id) + вехи из типологии.
    Если МГ уже есть — возвращает существующий (без дубликатов вех).
    """
    # 1. Идемпотентность
    existing = await db.execute(
        select(Mastergraph).where(
            Mastergraph.project_id == project_id,
            Mastergraph.is_deleted == False,
        )
    )
    mg = existing.scalar_one_or_none()
    if mg:
        logger.info(f"MG for project {project_id} already exists: {mg.mg_id}")
        return mg

    # 2. Типология
    typology = load_typology(project_type, typology_name)
    if not typology:
        raise MastergraphCreationError(
            f"Typology '{typology_name}' for project_type '{project_type}' not found"
        )

    # 3. Создаём МГ
    mg = Mastergraph(
        project_id=project_id,
        name=name or f"Мастерграфик проекта #{project_id}",
        version=1,
        is_approved=False,
        typology_name=typology_name,
        created_by=cve_id,
    )
    db.add(mg)
    await db.flush()  # получить mg_id

    # 4. Создаём вехи
    milestones_data = build_milestones(typology)
    project_start = start_date or date.today()
    milestone_repo = MilestoneRepository(db)
    await milestone_repo.create_many(mg.mg_id, milestones_data, project_start)

    # 5. Событие создания
    event_repo = EventRepository(db)
    await event_repo.log(
        mg_id=mg.mg_id,
        entity_type=MgEntityType.MASTERGRAPH,
        entity_id=mg.mg_id,
        event_type=MgEventType.CREATE,
        new_value={
            "project_id": project_id,
            "project_type": project_type,
            "typology_name": typology_name,
            "cve_id": cve_id,
            "ipp_id": ipp_id,
        },
        actor_id=cve_id,
    )

    await db.commit()
    await db.refresh(mg)
    logger.info(f"Created MG {mg.mg_id} for project {project_id} with {len(milestones_data)} milestones")
    return mg
