# app/config.py
from pydantic_settings import BaseSettings
from typing import Optional

class Settings(BaseSettings):
    DATABASE_URL: str
    DEBUG: bool = False
    POOL_SIZE: int = 5
    MAX_OVERFLOW: int = 10
    
    # User Service URL для проверки пользователей
    USER_SERVICE_URL: str = "http://localhost:8000"
    
    class Config:
        env_file = ".env"
        env_file_encoding = "utf-8"
        extra = "ignore"

settings = Settings()
