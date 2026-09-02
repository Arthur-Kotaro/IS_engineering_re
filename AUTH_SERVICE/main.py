import jwt
import redis
import httpx
import logging
from fastapi import FastAPI, Request, HTTPException
from fastapi.responses import Response
from typing import Dict, Optional
from pydantic_settings import BaseSettings
from dotenv import load_dotenv
import os
from urllib.parse import urlencode

load_dotenv()

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class Settings(BaseSettings):
    JWT_SECRET_KEY: str = "wtehi6574-GKEROGjdei-Jdj48jvl1_kfeo_s"
    ALGORITHM: str = "HS256"
    REDIS_HOST: str = "localhost"
    REDIS_PORT: int = 6379
    REDIS_DB: int = 0
    USER_SERVICE_URL: str = "http://localhost:8000"
    UI_COMPOSER_URL: str = "http://localhost:8020"
    AUTH_SERVICE_PORT: int = 8010

    class Config:
        env_file = ".env"
        extra = "ignore"

settings = Settings()

try:
    redis_client = redis.Redis(
        host=settings.REDIS_HOST,
        port=settings.REDIS_PORT,
        db=settings.REDIS_DB,
        decode_responses=True
    )
    redis_client.ping()
    logger.info("Redis connected")
except Exception as e:
    logger.warning(f"Redis connection failed: {e}")
    redis_client = None

app = FastAPI(title="Auth Service", version="2.0.0")

SERVICE_ROUTES = {
    "auth": "http://localhost:8000/api/v1/auth",
    "users": "http://localhost:8000/api/v1/users",
    "admin": "http://localhost:8000/api/v1/admin",
    "hr": "http://localhost:8000/api/v1/hr",
    "projects": "http://localhost:8001/api/v1/projects",
    "pjp": "http://localhost:8002/api/v1/pjp",
    "mg": "http://localhost:8003/api/v1/mg",
    "proto": "http://localhost:8004/api/v1/proto",
    "navigation": "http://localhost:8009/api/v1/navigation",
    "delegations": "http://localhost:8011/api/v1/delegations",
    "delegation-rules": "http://localhost:8011/api/v1/delegation-rules",
    "notifications": "http://localhost:8012/api/v1/notifications",
}

PAGE_ROUTES = {
    "page": settings.UI_COMPOSER_URL,
}

@app.get("/health")
async def health():
    return {"status": "healthy"}

async def decode_token(token: str) -> Optional[Dict]:
    try:
        payload = jwt.decode(
            token,
            settings.JWT_SECRET_KEY,
            algorithms=[settings.ALGORITHM]
        )
        return payload
    except jwt.ExpiredSignatureError:
        raise HTTPException(401, "Token expired")
    except jwt.InvalidTokenError:
        raise HTTPException(401, "Invalid token")

async def is_token_blacklisted(jti: str) -> bool:
    if not redis_client:
        return False
    return redis_client.exists(f"blacklist:{jti}")

@app.api_route("/{path:path}", methods=["GET", "POST", "PUT", "DELETE", "PATCH"])
async def proxy(request: Request, path: str):
    logger.info(f"=== PROXY: {request.method} {path} ===")
    
    path_parts = path.split("/")
    first_part = path_parts[0] if path_parts else ""
    
    # Для /page/* используем UI Composer
    if first_part == "page" or first_part == "pages":
        target_base = PAGE_ROUTES.get("page")
        logger.info(f"Route to UI Composer: {target_base}")
        target_url = f"{target_base}/{path}"
        
        # Добавляем параметры
        query_params = dict(request.query_params)
        if query_params:
            target_url = f"{target_url}?{urlencode(query_params)}"
        
        logger.info(f"Forwarding to: {target_url}")
        
        headers = dict(request.headers)
        headers.pop("host", None)
        headers.pop("content-length", None)
        
        # Проверяем токен для страниц тоже
        auth_header = request.headers.get("Authorization")
        if auth_header:
            token = auth_header.replace("Bearer ", "").strip()
            try:
                payload = await decode_token(token)
                user_id = payload.get("user_id")
                if user_id:
                    headers["X-User-ID"] = str(user_id)
                    logger.info(f"Added X-User-ID: {user_id} for page request")
            except Exception as e:
                logger.warning(f"Failed to decode token for page: {e}")
        
        client = httpx.AsyncClient(timeout=30.0)
        try:
            body = await request.body()
            
            if request.method == "GET":
                response = await client.get(target_url, headers=headers)
            elif request.method == "POST":
                response = await client.post(target_url, content=body, headers=headers)
            elif request.method == "PUT":
                response = await client.put(target_url, content=body, headers=headers)
            elif request.method == "DELETE":
                response = await client.delete(target_url, headers=headers)
            else:
                raise HTTPException(405, "Method not allowed")
            
            return Response(
                content=response.content,
                status_code=response.status_code,
                headers=dict(response.headers)
            )
        except Exception as e:
            logger.error(f"Proxy error: {e}")
            raise HTTPException(502, "Bad gateway")
        finally:
            await client.aclose()
    
    # API маршруты
    if len(path_parts) >= 3 and path_parts[0] == "api" and path_parts[1] == "v1":
        service_name = path_parts[2]
        remaining = "/".join(path_parts[3:]) if len(path_parts) > 3 else ""
    else:
        service_name = path_parts[0]
        remaining = "/".join(path_parts[1:]) if len(path_parts) > 1 else ""
    
    target_base = SERVICE_ROUTES.get(service_name)
    if not target_base:
        raise HTTPException(404, f"Service not found: {service_name}")
    
    # Проверка JWT для API маршрутов
    auth_header = request.headers.get("Authorization")
    if not auth_header:
        raise HTTPException(401, "Authorization header required")
    
    token = auth_header.replace("Bearer ", "").strip()
    
    try:
        payload = await decode_token(token)
        jti = payload.get("jti")
        if jti and await is_token_blacklisted(jti):
            raise HTTPException(401, "Token blacklisted")
        
        user_id = payload.get("user_id")
        roles = payload.get("roles", [])
        is_super_admin = payload.get("is_super_admin", False)
        
        if user_id is None:
            raise HTTPException(401, "Invalid token: missing user_id")
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Token decode error: {e}")
        raise HTTPException(401, "Invalid token")
    
    # Формируем target URL
    if remaining:
        target_url = f"{target_base}/{remaining}"
    else:
        target_url = target_base
    
    query_params = dict(request.query_params)
    if query_params:
        target_url = f"{target_url}?{urlencode(query_params)}"
    
    logger.info(f"Forwarding to: {target_url}")
    
    headers = dict(request.headers)
    headers.pop("host", None)
    headers.pop("content-length", None)
    headers["X-User-ID"] = str(user_id)
    headers["X-User-Role"] = ",".join(roles) if roles else "user"
    if is_super_admin:
        headers["X-User-Role"] = "super_admin"
    
    client = httpx.AsyncClient(timeout=30.0)
    try:
        body = await request.body()
        
        if request.method == "GET":
            response = await client.get(target_url, headers=headers)
        elif request.method == "POST":
            response = await client.post(target_url, content=body, headers=headers)
        elif request.method == "PUT":
            response = await client.put(target_url, content=body, headers=headers)
        elif request.method == "DELETE":
            response = await client.delete(target_url, headers=headers)
        else:
            raise HTTPException(405, "Method not allowed")
        
        return Response(
            content=response.content,
            status_code=response.status_code,
            headers=dict(response.headers)
        )
    except Exception as e:
        logger.error(f"Proxy error: {e}")
        raise HTTPException(502, "Bad gateway")
    finally:
        await client.aclose()

if __name__ == "__main__":
    import uvicorn
    uvicorn.run("main:app", host="0.0.0.0", port=settings.AUTH_SERVICE_PORT, reload=True)
