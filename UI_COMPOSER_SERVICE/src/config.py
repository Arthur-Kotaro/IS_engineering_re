# src/config.py
from pydantic_settings import BaseSettings
from dotenv import load_dotenv

load_dotenv()


class Settings(BaseSettings):
    REDIS_HOST: str = "localhost"
    REDIS_PORT: int = 6379
    REDIS_SESSIONS_DB: int = 1

    SESSION_TTL_SECONDS: int = 3600

    SERVICES_CONFIG_PATH: str = "config/services.yaml"
    TEMPLATES_DIR: str = "templates"
    WORKFLOWS_DIR: str = "workflows"

    INTERNAL_API_KEY: str = ""

    class Config:
        env_file = (".env", "../.env")
        extra = "ignore"


settings = Settings()
