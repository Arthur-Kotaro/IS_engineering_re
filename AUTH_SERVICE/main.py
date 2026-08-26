import jwt
import redis
import httpx
from fastapi import FastAPI, Request, Response
from fastapi.responses import JSONResponse
import logging
from dotenv import load_dotenv
from urllib.parse import urlparse, parse_qs

load_dotenv()

from config import (
    SECRET_KEY, ALGORITHM, REDIS_HOST, REDIS_PORT, REDIS_DB,
    REDIS_BLACKLIST_PREFIX, SERVICE_ROUTES, PUBLIC_ENDPOINTS
)

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = FastAPI(title="Auth Service", version="1.0.0")

try:
    redis_client = redis.Redis(
        host=REDIS_HOST, 
        port=REDIS_PORT, 
        db=REDIS_DB,
        decode_responses=True
    )
    redis_client.ping()
    logger.info("✅ Connected to Redis")
except Exception as e:
    logger.error(f"❌ Redis connection failed: {e}")
    redis_client = None

@app.get("/health")
async def health_check():
    return {
        "status": "healthy",
        "redis": "connected" if redis_client and redis_client.ping() else "disconnected"
    }

@app.api_route("/{path:path}", methods=["GET", "POST", "PUT", "DELETE", "PATCH", "OPTIONS", "HEAD"])
async def gateway(request: Request, path: str):
    query_string = request.url.query
    full_path = f"/{path}"
    if query_string:
        full_path = f"{full_path}?{query_string}"
    
    logger.info(f"🔹 Gateway received: path='{path}', full_path='{full_path}'")
    
    path_without_params = full_path.split('?')[0]
    if path_without_params in PUBLIC_ENDPOINTS:
        logger.info(f"Public endpoint: {full_path}")
        return await proxy_to_service(request, full_path, add_auth_headers=False)
    
    auth_header = request.headers.get("Authorization")
    if not auth_header:
        logger.warning(f"No Authorization header for {full_path}")
        return JSONResponse(
            status_code=401,
            content={"detail": "Missing Authorization header"}
        )
    
    token = auth_header.replace("Bearer ", "").strip()
    if not token:
        logger.warning(f"Empty token for {full_path}")
        return JSONResponse(
            status_code=401,
            content={"detail": "Empty token"}
        )
    
    try:
        payload = jwt.decode(token, SECRET_KEY, algorithms=[ALGORITHM])
        logger.info(f"Token validated for user: {payload.get('user_id', 'unknown')}")
    except jwt.ExpiredSignatureError:
        logger.warning(f"Expired token for {full_path}")
        return JSONResponse(
            status_code=401,
            content={"detail": "Token has expired"}
        )
    except jwt.InvalidTokenError as e:
        logger.warning(f"Invalid token for {full_path}: {str(e)}")
        return JSONResponse(
            status_code=401,
            content={"detail": f"Invalid token: {str(e)}"}
        )
    
    jti = payload.get("jti")
    if jti and redis_client:
        blacklist_key = f"{REDIS_BLACKLIST_PREFIX}{jti}"
        if redis_client.exists(blacklist_key):
            logger.warning(f"Token in blacklist: {jti}")
            return JSONResponse(
                status_code=401,
                content={"detail": "Token has been revoked"}
            )
    
    user_id = payload.get("user_id")
    role = payload.get("roles", ["user"])
    
    if not user_id:
        logger.error(f"No user_id in token: {payload}")
        return JSONResponse(
            status_code=401,
            content={"detail": "Invalid token payload"}
        )
    
    if isinstance(role, list):
        role = role[0] if role else "user"
    
    return await proxy_to_service(
        request, 
        full_path, 
        add_auth_headers=True,
        user_id=str(user_id),
        role=role
    )

async def proxy_to_service(
    request: Request, 
    full_path: str, 
    add_auth_headers: bool = True,
    user_id: str = None,
    role: str = None
):
    logger.info(f"🔸 proxy_to_service: full_path='{full_path}'")
    
    path_without_params = full_path.split('?')[0]
    parts = path_without_params.strip('/').split('/')
    
    logger.info(f"🔸 parts={parts}")
    
    if len(parts) >= 1 and parts[0] == "api":
        if len(parts) >= 3:
            service_name = parts[2]
            target_path = "/" + "/".join(parts[3:]) if parts[3:] else "/"
            target_full_path = f"/api/v1/{service_name}{target_path}"
        else:
            logger.error(f"Invalid path format with api prefix: {full_path}")
            return JSONResponse(
                status_code=404,
                content={"detail": f"Invalid path format: {full_path}"}
            )
    else:
        if len(parts) >= 1:
            service_name = parts[0]
            target_full_path = full_path
        else:
            logger.error(f"Invalid path format: {full_path}")
            return JSONResponse(
                status_code=404,
                content={"detail": f"Invalid path format: {full_path}"}
            )
    
    if service_name not in SERVICE_ROUTES:
        logger.error(f"Unknown service: {service_name} for path: {full_path}")
        return JSONResponse(
            status_code=404,
            content={"detail": f"Service not found: {service_name}"}
        )
    
    target_url = f"{SERVICE_ROUTES[service_name]}{target_full_path}"
    logger.info(f"🔸 target_url={target_url}")
    
    headers = dict(request.headers)
    headers.pop("host", None)
    headers.pop("content-length", None)
    
    if add_auth_headers and user_id:
        headers["X-User-ID"] = str(user_id)
        if role:
            headers["X-User-Role"] = str(role)
    
    body = await request.body()
    
    try:
        async with httpx.AsyncClient(timeout=30.0, follow_redirects=True) as client:
            response = await client.request(
                method=request.method,
                url=target_url,
                headers=headers,
                content=body if body else None
            )
        
        return Response(
            content=response.content,
            status_code=response.status_code,
            headers=dict(response.headers)
        )
    except httpx.TimeoutException:
        logger.error(f"Timeout connecting to {target_url}")
        return JSONResponse(
            status_code=504,
            content={"detail": "Gateway timeout"}
        )
    except Exception as e:
        logger.error(f"Error proxying to {target_url}: {str(e)}")
        return JSONResponse(
            status_code=502,
            content={"detail": f"Bad gateway: {str(e)}"}
        )
