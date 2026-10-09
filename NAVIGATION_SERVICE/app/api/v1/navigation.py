# app/api/v1/navigation.py
import asyncio
import logging
import httpx
from typing import Optional, List, Dict
from fastapi import APIRouter, Depends, HTTPException, Request
from sqlalchemy import select, or_
from app.database import AsyncSessionLocal
from app.models.tile import Tile, TileByRole, TileByPosition, TileUniversal

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

router = APIRouter()


def _user_id(request: Request) -> int:
    uid = request.headers.get("X-User-ID")
    if not uid:
        raise HTTPException(401, "Missing X-User-ID")
    try:
        return int(uid)
    except ValueError:
        raise HTTPException(401, "Invalid X-User-ID")


def _user_roles(request: Request) -> List[str]:
    """X-User-Role: comma-separated list or 'super_admin' or 'user'."""
    raw = request.headers.get("X-User-Role", "").strip()
    if not raw:
        return []
    return [r.strip() for r in raw.split(",") if r.strip()]


def _user_position(request: Request) -> Optional[str]:
    p = request.headers.get("X-User-Position")
    return p.strip() if p else None


async def _get_badge_count(endpoint: str, token: Optional[str], user_id: int) -> int:
    if not endpoint:
        return 0
    try:
        headers = {"X-User-ID": str(user_id)}
        if token:
            headers["Authorization"] = f"Bearer {token}"
        async with httpx.AsyncClient(timeout=2.0) as client:
            url = f"http://localhost:8080{endpoint}"
            resp = await client.get(url, headers=headers)
            if resp.status_code == 200:
                data = resp.json()
                return int(data.get("count", 0))
    except Exception as e:
        logger.warning(f"badge {endpoint}: {e}")
    return 0


@router.get("/dashboard")
async def get_dashboard(request: Request):
    user_id = _user_id(request)
    roles = _user_roles(request)
    position = _user_position(request)

    logger.info(f"dashboard: user_id={user_id} roles={roles} position={position}")

    async with AsyncSessionLocal() as db:
        # admin — все плитки
        if "admin" in roles or "super_admin" in roles:
            stmt = select(Tile).where(Tile.is_active == True).order_by(Tile.sort_order)
            result = await db.execute(stmt)
            tiles = result.scalars().all()
        else:
            # плитки по ролям
            role_tiles_stmt = select(TileByRole.tile_id).where(TileByRole.role_code.in_(roles)) if roles else None
            # плитки по должности
            position_tiles_stmt = select(TileByPosition.tile_id).where(TileByPosition.position_code == position) if position else None
            # универсальные
            universal_stmt = select(TileUniversal.tile_id)

            tile_ids = set()
            if role_tiles_stmt is not None:
                r = await db.execute(role_tiles_stmt)
                tile_ids.update(row[0] for row in r.all())
            if position_tiles_stmt is not None:
                r = await db.execute(position_tiles_stmt)
                tile_ids.update(row[0] for row in r.all())
            r = await db.execute(universal_stmt)
            tile_ids.update(row[0] for row in r.all())

            if not tile_ids:
                return {"tiles": [], "user_roles": roles, "user_position": position, "is_admin": False}

            stmt = select(Tile).where(
                Tile.tile_id.in_(list(tile_ids)),
                Tile.is_active == True,
            ).order_by(Tile.sort_order)
            result = await db.execute(stmt)
            tiles = result.scalars().all()

    # badge
    token = request.headers.get("Authorization", "").removeprefix("Bearer ").strip() or None
    tasks = []
    for t in tiles:
        if t.badge_enabled and t.badge_endpoint:
            tasks.append(_get_badge_count(t.badge_endpoint, token, user_id))
        else:
            tasks.append(asyncio.sleep(0, result=0))
    counts = await asyncio.gather(*tasks)

    result_tiles = []
    for t, c in zip(tiles, counts):
        item = {
            "id": t.tile_id,
            "label": t.label,
            "endpoint": t.endpoint,
            "method": t.method,
            "icon": t.icon,
        }
        if t.badge_enabled:
            item["badge_count"] = c
        result_tiles.append(item)

    return {
        "tiles": result_tiles,
        "user_roles": roles,
        "user_position": position,
        "is_admin": "admin" in roles or "super_admin" in roles,
    }
