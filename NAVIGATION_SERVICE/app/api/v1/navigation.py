# app/api/v1/navigation.py (ОБНОВЛЕННАЯ ВЕРСИЯ - ЧЕРЕЗ AUTH SERVICE)
from fastapi import APIRouter, Depends, HTTPException, Request
from sqlalchemy import select
from app.database import AsyncSessionLocal
from app.models.tile import Tile
import httpx
import asyncio

router = APIRouter()


async def get_user_id(request: Request) -> int:
    """Получить ID пользователя из заголовка X-User-ID (от Auth Service)"""
    user_id = request.headers.get("X-User-ID")
    if not user_id:
        raise HTTPException(401, "Missing X-User-ID header")
    try:
        return int(user_id)
    except ValueError:
        raise HTTPException(401, "Invalid X-User-ID format")


async def get_user_roles_from_service(user_id: int) -> list:
    """Получить роли пользователя из User Service по ID"""
    async with httpx.AsyncClient() as client:
        resp = await client.get(
            f"http://localhost:8000/api/v1/users/{user_id}/roles"
        )
        if resp.status_code != 200:
            return []
        return resp.json().get("roles", [])


async def get_badge_count(endpoint: str) -> int:
    """Получить количество из микросервиса по badge_endpoint"""
    if not endpoint:
        return 0
    try:
        async with httpx.AsyncClient(timeout=2.0) as client:
            # Запрос к микросервису через API Gateway
            resp = await client.get(f"http://localhost:8080{endpoint}")
            if resp.status_code == 200:
                data = resp.json()
                return data.get("count", 0)
    except Exception as e:
        print(f"Failed to get badge count from {endpoint}: {e}")
    return 0


@router.get("/dashboard")
async def get_dashboard(
    request: Request,
    user_id: int = Depends(get_user_id)
):
    """Получить дашборд с плитками для пользователя"""
    # Получаем роли пользователя из User Service
    roles = await get_user_roles_from_service(user_id)
    
    if not roles:
        # Если у пользователя нет ролей, возвращаем пустой дашборд
        return {"tiles": [], "user_roles": []}
    
    # Загружаем плитки из БД
    async with AsyncSessionLocal() as db:
        result = await db.execute(
            select(Tile).where(
                Tile.role.in_(roles),
                Tile.is_active == True
            ).order_by(Tile.sort_order)
        )
        tiles = result.scalars().all()
    
    # Собираем счетчики параллельно (для плиток с badge_enabled)
    tasks = []
    for tile in tiles:
        if tile.badge_enabled and tile.badge_endpoint:
            tasks.append(get_badge_count(tile.badge_endpoint))
        else:
            tasks.append(asyncio.sleep(0, result=0))
    
    counts = await asyncio.gather(*tasks)
    
    # Формируем ответ
    result_tiles = []
    for tile, count in zip(tiles, counts):
        tile_data = {
            "id": tile.tile_id,
            "label": tile.label,
            "endpoint": tile.endpoint,
            "method": tile.method,
            "icon": tile.icon,
        }
        if tile.badge_enabled:
            tile_data["badge_count"] = count
        result_tiles.append(tile_data)
    
    return {
        "tiles": result_tiles,
        "user_roles": roles
    }


@router.post("/tiles")
async def create_tile(
    tile_data: dict,
    user_id: int = Depends(get_user_id),
    roles: list = Depends(get_user_roles_from_service)
):
    """Административный эндпоинт для создания плиток"""
    if "admin" not in roles:
        raise HTTPException(403, "Only admin can manage tiles")
    
    async with AsyncSessionLocal() as db:
        from app.models.tile import Tile
        tile = Tile(
            role=tile_data["role"],
            tile_id=tile_data["tile_id"],
            label=tile_data["label"],
            endpoint=tile_data["endpoint"],
            method=tile_data.get("method", "GET"),
            icon=tile_data.get("icon"),
            sort_order=tile_data.get("sort_order", 0),
            badge_enabled=tile_data.get("badge_enabled", False),
            badge_endpoint=tile_data.get("badge_endpoint")
        )
        db.add(tile)
        await db.commit()
        return {"message": "Tile created", "tile_id": tile.tile_id}
