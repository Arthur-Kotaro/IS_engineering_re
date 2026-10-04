# app/config.py
from pydantic_settings import BaseSettings
from dotenv import load_dotenv

load_dotenv()


class Settings(BaseSettings):
    DATABASE_URL: str
    DEBUG: bool = True
    PORT: int = 8012
    HOST: str = "0.0.0.0"

    USER_SERVICE_URL: str = "http://localhost:8000"
    DELEGATION_SERVICE_URL: str = "http://localhost:8011"

    SECRET_KEY: str
    ALGORITHM: str = "HS256"

    SMTP_HOST: str = "smtp.gmail.com"
    SMTP_PORT: int = 587
    SMTP_USER: str = ""
    SMTP_PASSWORD: str = ""
    FROM_EMAIL: str = "noreply@company.com"

    SSE_HEARTBEAT_INTERVAL: int = 30

    class Config:
        env_file = (".env", "../.env")
        env_file_encoding = "utf-8"
        extra = "ignore"


settings = Settings()
