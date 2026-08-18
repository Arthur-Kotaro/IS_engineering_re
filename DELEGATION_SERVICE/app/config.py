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
    PORT: int = 8011
    HOST: str = "0.0.0.0"
    
    # External Services
    USER_SERVICE_URL: str = "http://localhost:8000"
    AUTH_SERVICE_URL: str = "http://localhost:8010"
    NOTIFICATION_SERVICE_URL: str = "http://localhost:8012"
    
    # JWT (shared with Auth Service)
    SECRET_KEY: str = "wtehi6574-GKEROGjdei-Jdj48jvl1_kfeo_s"
    ALGORITHM: str = "HS256"
    
    # Delegation rules
    MAX_DELEGATIONS_PER_USER: int = 3
    MAX_DELEGATION_DURATION_DAYS: int = 30
    AUTO_EXPIRE_CHECK_INTERVAL_SECONDS: int = 3600
    
    class Config:
        env_file = ".env"
        env_file_encoding = "utf-8"
        extra = "ignore"

settings = Settings()
