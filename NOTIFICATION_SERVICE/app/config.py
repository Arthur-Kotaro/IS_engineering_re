# app/config.py
import os
from typing import Optional
from pydantic_settings import BaseSettings
from dotenv import load_dotenv

load_dotenv()

class Settings(BaseSettings):
    # Database
    DATABASE_URL: str
    
    # Service
    DEBUG: bool = True
    PORT: int = 8012
    HOST: str = "0.0.0.0"
    
    # External Services
    USER_SERVICE_URL: str = "http://localhost:8000"
    DELEGATION_SERVICE_URL: str = "http://localhost:8011"
    
    # Email
    SMTP_HOST: str = "smtp.gmail.com"
    SMTP_PORT: int = 587
    SMTP_USER: str = ""
    SMTP_PASSWORD: str = ""
    FROM_EMAIL: str = "noreply@company.com"
    
    # JWT
    SECRET_KEY: str = "wtehi6574-GKEROGjdei-Jdj48jvl1_kfeo_s"
    ALGORITHM: str = "HS256"
    
    # SSE
    SSE_HEARTBEAT_INTERVAL: int = 30
    
    class Config:
        env_file = ".env"
        env_file_encoding = "utf-8"
        extra = "ignore"

settings = Settings()
