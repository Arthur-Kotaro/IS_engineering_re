# AUTH_SERVICE/main.py
import jwt
import redis.asyncio as aioredis
import logging
from fastapi import FastAPI, Request, HTTPException, Response
from pydantic_settings import BaseSettings
from dotenv import load_dotenv

load_dotenv()

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class Settings(BaseSettings):
    JWT_SECRET_KEY: str
    ALGORITHM: str = "HS256"
    REDIS_HOST: str = "localhost"
    REDIS_PORT: int = 6379
    REDIS_DB: int = 0
    AUTH_SERVICE_PORT: int = 8010

    class Config:
        env_file = ".env"
        extra = "ignore"


settings = Settings()

redis_client: aioredis.Redis = aioredis.Redis(
    host=settings.REDIS_HOST,
    port=settings.REDIS_PORT,
    db=settings.REDIS_DB,
    decode_responses=True,
)

app = FastAPI(title="Auth Service", version="2.0.0")


@app.get("/health")
async def health():
    try:
        await redis_client.ping()
        redis_ok = True
    except Exception:
        redis_ok = False
    return {"status": "healthy", "redis": redis_ok}


def _blacklist_key(jti: str) -> str:
    return f"blacklist:{jti}"


def _impersonation_key(impersonation_id: str) -> str:
    return f"impersonation:{impersonation_id}"


@app.api_route("/verify", methods=["GET", "POST", "PUT", "PATCH", "DELETE"])
async def verify(request: Request):
    auth_header = request.headers.get("Authorization")
    if not auth_header or not auth_header.startswith("Bearer "):
        raise HTTPException(401, "Missing or invalid Authorization header")

    token = auth_header.removeprefix("Bearer ").strip()

    try:
        payload = jwt.decode(
            token,
            settings.JWT_SECRET_KEY,
            algorithms=[settings.ALGORITHM],
        )
    except jwt.ExpiredSignatureError:
        raise HTTPException(401, "Token expired")
    except jwt.InvalidTokenError as e:
        logger.warning(f"Invalid token: {e}")
        raise HTTPException(401, "Invalid token")

    jti = payload.get("jti")
    if jti:
        try:
            if await redis_client.exists(_blacklist_key(jti)):
                raise HTTPException(401, "Token revoked")
        except HTTPException:
            raise
        except Exception as e:
            logger.warning(f"Redis unavailable (blacklist), fail-open: {e}")

    impersonation_id = payload.get("impersonation_id")
    if impersonation_id:
        try:
            if not await redis_client.exists(_impersonation_key(impersonation_id)):
                raise HTTPException(401, "Impersonation session closed")
        except HTTPException:
            raise
        except Exception as e:
            logger.warning(f"Redis unavailable (impersonation), fail-open: {e}")

    user_id = payload.get("user_id")
    if user_id is None:
        raise HTTPException(401, "Invalid token: missing user_id")

    roles = payload.get("roles", [])
    is_super_admin = payload.get("is_super_admin", False)
    impersonated_by = payload.get("impersonated_by")

    headers = {
        "X-User-ID": str(user_id),
        "X-User-Role": "super_admin" if is_super_admin else (",".join(roles) if roles else "user"),
    }
    if impersonated_by is not None:
        headers["X-Impersonated-By"] = str(impersonated_by)
    if impersonation_id:
        headers["X-Impersonation-Id"] = str(impersonation_id)

    return Response(status_code=200, headers=headers)


if __name__ == "__main__":
    import uvicorn
    uvicorn.run("main:app", host="0.0.0.0", port=settings.AUTH_SERVICE_PORT, reload=True)
