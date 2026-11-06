# app/config.py
from pydantic_settings import BaseSettings
from dotenv import load_dotenv

load_dotenv()


class Settings(BaseSettings):
    DATABASE_URL: str = "sqlite+aiosqlite:///./bus_service.db"
    DEBUG: bool = False
    SECRET_KEY: str = "dev-secret"

    class Config:
        env_file = (".env", "../.env")
        env_file_encoding = "utf-8"
        extra = "ignore"


settings = Settings()
