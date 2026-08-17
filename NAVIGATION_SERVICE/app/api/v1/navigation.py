# app/api/v1/navigation.py
from fastapi import APIRouter, Depends, HTTPException, Request
from sqlalchemy import select
from app.database import AsyncSessionLocal
from app.models.tile import Tile
import httpx
import asyncio
import logging
import jwt

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

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


async def get_user_info_from_service(request: Request, user_id: int) -> dict:
    """Получить информацию о пользователе из User Service через Gateway"""
    # Получаем токен из заголовка
    auth_header = request.headers.get("Authorization")
    if not auth_header:
        logger.warning("No Authorization header for user info request")
        return {}
    
    async with httpx.AsyncClient() as client:
        resp = await client.get(
            f"http://localhost:8080/api/v1/admin/users/{user_id}",
            headers={"Authorization": auth_header}
        )
        if resp.status_code != 200:
            logger.warning(f"Failed to get user info for {user_id}: {resp.status_code}")
            logger.warning(f"Response: {resp.text}")
            return {}
        return resp.json()


async def is_user_admin_via_service(request: Request, user_id: int) -> bool:
    """Проверить, является ли пользователь администратором через User Service"""
    user_data = await get_user_info_from_service(request, user_id)
    if not user_data:
        # Если не удалось получить через сервис, проверяем токен
        auth_header = request.headers.get("Authorization")
        if auth_header:
            token = auth_header.replace("Bearer ", "").strip()
            try:
                payload = jwt.decode(token, options={"verify_signature": False})
                roles = payload.get("roles", [])
                is_super_admin = payload.get("is_super_admin", False)
                result = is_super_admin or "admin" in roles
                logger.info(f"📌 Admin check from token: {result}")
                return result
            except Exception as e:
                logger.warning(f"Failed to decode token: {e}")
        return False
    
    roles = user_data.get("roles", [])
    is_super_admin = user_data.get("is_super_admin", False)
    result = is_super_admin or "admin" in roles
    logger.info(f"📌 Is user {user_id} admin? {result} (super_admin: {is_super_admin}, roles: {roles})")
    return result


async def get_badge_count(endpoint: str) -> int:
    """Получить количество из микросервиса по badge_endpoint"""
    if not endpoint:
        return 0
    try:
        async with httpx.AsyncClient(timeout=2.0) as client:
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
    logger.info(f"🔍 Getting dashboard for user {user_id}")
    
    # Проверяем, является ли пользователь администратором
    is_admin = await is_user_admin_via_service(request, user_id)
    logger.info(f"📌 is_admin: {is_admin}")
    
    # Получаем роли из токена
    auth_header = request.headers.get("Authorization")
    roles = []
    if auth_header:
        token = auth_header.replace("Bearer ", "").strip()
        try:
            payload = jwt.decode(token, options={"verify_signature": False})
            roles = payload.get("roles", [])
            logger.info(f"📌 Roles from token: {roles}")
        except Exception as e:
            logger.warning(f"Failed to decode token: {e}")
    
    # Загружаем плитки из БД
    async with AsyncSessionLocal() as db:
        if is_admin:
            logger.info("👑 Admin detected, loading ALL tiles")
            result = await db.execute(
                select(Tile).where(
                    Tile.is_active == True
                ).order_by(Tile.sort_order)
            )
        else:
            if not roles:
                return {"tiles": [], "user_roles": [], "is_admin": False}
            logger.info(f"👤 Regular user with roles: {roles}")
            result = await db.execute(
                select(Tile).where(
                    Tile.role.in_(roles),
                    Tile.is_active == True
                ).order_by(Tile.sort_order)
            )
        tiles = result.scalars().all()
        logger.info(f"📌 Found {len(tiles)} tiles")
    
    # Собираем счетчики параллельно
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
        "user_roles": roles,
        "is_admin": is_admin
    }


@router.post("/tiles")
async def create_tile(
    tile_data: dict,
    request: Request,
    user_id: int = Depends(get_user_id)
):
    """Административный эндпоинт для создания плиток"""
    is_admin = await is_user_admin_via_service(request, user_id)
    if not is_admin:
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
