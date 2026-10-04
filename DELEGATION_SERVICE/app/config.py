# app/config.py
from pydantic_settings import BaseSettings
from dotenv import load_dotenv

load_dotenv()


class Settings(BaseSettings):
    DATABASE_URL: str
    DEBUG: bool = True
    PORT: int = 8011
    HOST: str = "0.0.0.0"

    USER_SERVICE_URL: str = "http://localhost:8000"
    NOTIFICATION_SERVICE_URL: str = "http://localhost:8012"

    SECRET_KEY: str
    ALGORITHM: str = "HS256"

    MAX_DELEGATIONS_PER_USER: int = 3
    MAX_DELEGATION_DURATION_DAYS: int = 30
    AUTO_EXPIRE_CHECK_INTERVAL_SECONDS: int = 3600

    REDIS_HOST: str = "localhost"
    REDIS_PORT: int = 6379
    REDIS_EVENTS_DB: int = 3

    class Config:
        env_file = (".env", "../.env")
        env_file_encoding = "utf-8"
        extra = "ignore"


settings = Settings()
