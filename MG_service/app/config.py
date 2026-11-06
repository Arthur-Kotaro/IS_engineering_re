# app/config.py
from pydantic_settings import BaseSettings
from dotenv import load_dotenv
import yaml
from pathlib import Path

load_dotenv()


class Settings(BaseSettings):
    DATABASE_URL: str
    DEBUG: bool = False
    POOL_SIZE: int = 5
    MAX_OVERFLOW: int = 10

    PROJECT_SERVICE_URL: str = "http://localhost:8001"
    USER_SERVICE_URL: str = "http://localhost:8000"
    INTERNAL_API_KEY: str = "dev-internal-key"
    SECRET_KEY: str
    ALGORITHM: str = "HS256"

    class Config:
        env_file = (".env", "../.env")
        env_file_encoding = "utf-8"
        extra = "ignore"


settings = Settings()


def load_bus_instances() -> dict:
    """Читает config/bus_instances.yaml — связи group_id -> url."""
    path = Path(__file__).parent.parent / "config" / "bus_instances.yaml"
    if not path.exists():
        return {}
    with open(path, "r") as f:
        data = yaml.safe_load(f)
    return data.get("instances", {})
