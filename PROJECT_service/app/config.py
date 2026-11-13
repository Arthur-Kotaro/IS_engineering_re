# app/config.py
from pydantic_settings import BaseSettings
from dotenv import load_dotenv

load_dotenv()


class Settings(BaseSettings):
    DATABASE_URL: str
    DEBUG: bool = False
    POOL_SIZE: int = 5
    MAX_OVERFLOW: int = 10
    SECRET_KEY: str
    ALGORITHM: str = "HS256"

    MG_SERVICE_URL: str = "http://localhost:8003"
    USER_SERVICE_URL: str = "http://localhost:8000"
    INTERNAL_API_KEY: str = "k4x9pLm2Qw8Rt5Yv7Bn3Fd1Gs6Hj0CzX"

    class Config:
        env_file = (".env", "../.env")
        env_file_encoding = "utf-8"
        extra = "ignore"


settings = Settings()
