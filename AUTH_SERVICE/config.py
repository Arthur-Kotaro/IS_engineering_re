import os
from typing import Dict
from dotenv import load_dotenv

load_dotenv()

# JWT настройки
SECRET_KEY = os.getenv("JWT_SECRET_KEY", "wtehi6574-GKEROGjdei-Jdj48jvl1_kfeo_s")
ALGORITHM = os.getenv("ALGORITHM", "HS256")
ACCESS_TOKEN_EXPIRE_MINUTES = int(os.getenv("ACCESS_TOKEN_EXPIRE_MINUTES", "15"))

# Redis настройки
REDIS_HOST = os.getenv("REDIS_HOST", "localhost")
REDIS_PORT = int(os.getenv("REDIS_PORT", "6379"))
REDIS_DB = int(os.getenv("REDIS_DB", "0"))
REDIS_BLACKLIST_PREFIX = "blacklist:"

# Сервисы для проксирования
SERVICE_ROUTES: Dict[str, str] = {
    "auth": "http://127.0.0.1:8000",
    "users": "http://127.0.0.1:8000",
    "admin": "http://127.0.0.1:8000",
    "projects": "http://127.0.0.1:8001",
    "pjp": "http://127.0.0.1:8002",
    "mg": "http://127.0.0.1:8003",
    "proto": "http://127.0.0.1:8004",
    "navigation": "http://127.0.0.1:8009",
    "delegations": "http://127.0.0.1:8011",
    "delegation-rules": "http://127.0.0.1:8011",
    "notifications": "http://127.0.0.1:8012",  # НОВЫЙ СЕРВИС
}

# Эндпоинты, которые НЕ требуют авторизации (идут напрямую в user_service)
PUBLIC_ENDPOINTS = [
    "/api/v1/auth/login",
    "/api/v1/auth/register",
    "/api/v1/auth/refresh",
    "/api/v1/health",
]
