# app/config.py
from pydantic_settings import BaseSettings
from dotenv import load_dotenv

load_dotenv()


class Settings(BaseSettings):
    DATABASE_URL: str
    DEBUG: bool = False
    POOL_SIZE: int = 5
    MAX_OVERFLOW: int = 10

    USER_SERVICE_URL: str = "http://localhost:8000"
    DELEGATION_SERVICE_URL: str = "http://localhost:8011"

    SECRET_KEY: str
    ALGORITHM: str = "HS256"

    ACCESS_CACHE_TTL: int = 30

    class Config:
        env_file = (".env", "../.env")
        env_file_encoding = "utf-8"
        extra = "ignore"


settings = Settings()
