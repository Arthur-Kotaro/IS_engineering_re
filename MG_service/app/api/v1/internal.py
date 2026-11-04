# app/api/v1/internal.py
from fastapi import APIRouter
import logging

logger = logging.getLogger(__name__)

router = APIRouter(tags=["Internal"])


@router.post("/forecast-update")
async def forecast_update(payload: dict):
    """TODO: обновление прогноза от BUS."""
    logger.info(f"forecast-update: {payload}")
    return {"status": "ok"}
