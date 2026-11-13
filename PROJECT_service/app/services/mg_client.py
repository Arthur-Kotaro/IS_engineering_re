# app/services/mg_client.py
import logging
from typing import Optional
import aiohttp
from app.config import settings

logger = logging.getLogger(__name__)


async def create_mastergraph(
    project_id: int,
    project_type: str,
    typology_name: str,
    cve_id: int,
    ipp_id: int,
) -> Optional[dict]:
    """POST /internal/mastergraphs в MG Service."""
    url = f"{settings.MG_SERVICE_URL}/internal/mastergraphs"
    payload = {
        "project_id": project_id,
        "project_type": project_type,
        "typology_name": typology_name,
        "cve_id": cve_id,
        "ipp_id": ipp_id,
    }
    headers = {}
    if settings.INTERNAL_API_KEY:
        headers["X-Internal-Key"] = settings.INTERNAL_API_KEY

    try:
        async with aiohttp.ClientSession(timeout=aiohttp.ClientTimeout(total=10.0)) as session:
            async with session.post(url, json=payload, headers=headers) as resp:
                if resp.status == 200:
                    data = await resp.json()
                    logger.info(f"MG created for project {project_id}: mg_id={data.get('mg_id')}")
                    return data
                text = await resp.text()
                logger.warning(f"MG create failed for project {project_id}: HTTP {resp.status} {text[:300]}")
                return None
    except Exception as e:
        logger.warning(f"MG create exception for project {project_id}: {e}")
        return None
