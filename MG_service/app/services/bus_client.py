# app/services/bus_client.py
import aiohttp
import logging
from typing import Optional
from app.config import load_bus_instances

logger = logging.getLogger(__name__)


async def init_local_graph(bus_url: str, payload: dict) -> Optional[dict]:
    try:
        async with aiohttp.ClientSession(timeout=aiohttp.ClientTimeout(total=5.0)) as s:
            async with s.post(f"{bus_url}/internal/local-graphs/init", json=payload) as resp:
                if resp.status == 200:
                    return await resp.json()
                logger.warning(f"init_local_graph: HTTP {resp.status}")
    except Exception as e:
        logger.warning(f"init_local_graph: {e}")
    return None


async def recalc_forecast(bus_url: str, payload: dict) -> Optional[dict]:
    try:
        async with aiohttp.ClientSession(timeout=aiohttp.ClientTimeout(total=5.0)) as s:
            async with s.post(f"{bus_url}/internal/local-graphs/forecast-recalc", json=payload) as resp:
                if resp.status == 200:
                    return await resp.json()
                logger.warning(f"recalc_forecast: HTTP {resp.status}")
    except Exception as e:
        logger.warning(f"recalc_forecast: {e}")
    return None


async def get_local_graph_state(bus_url: str, task_id: str) -> Optional[int]:
    """
    Возвращает local_graph_state из BUS:
      0 — ЛГ нет (404)
      1 — ЛГ есть, но dateless (REQUIRED)
      2 — ЛГ есть, есть задачи со сроками (ATTACHED)
      3 — ЛГ есть, но задачи без сроков (MISSING? — в BUS не используется)
      None — BUS недоступен (не сбрасывать состояние в MG)
    """
    if not bus_url:
        return 0
    try:
        async with aiohttp.ClientSession(timeout=aiohttp.ClientTimeout(total=2.0)) as s:
            async with s.get(f"{bus_url}/internal/local-graphs/by-task/{task_id}") as resp:
                if resp.status == 200:
                    data = await resp.json()
                    state = data.get("local_graph_state", 0)
                    return int(state) if state is not None else 0
                if resp.status == 404:
                    return 0
                logger.warning(f"get_local_graph_state({task_id}): HTTP {resp.status}")
                return None
    except Exception as e:
        logger.debug(f"get_local_graph_state({task_id}): {e}")
        return None


def get_bus_url_for_group(group_id: str) -> Optional[str]:
    """Читает актуальный конфиг при каждом вызове."""
    instances = load_bus_instances()
    info = instances.get(group_id) or instances.get("other_bus")
    return info.get("url") if info else None
