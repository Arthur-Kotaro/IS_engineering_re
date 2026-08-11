# app/models/tile.py
from sqlalchemy import Column, Integer, String, Boolean, DateTime
from sqlalchemy.sql import func
from app.database import Base

class Tile(Base):
    __tablename__ = "tiles"
    
    id = Column(Integer, primary_key=True, autoincrement=True)
    role = Column(String(50), nullable=False)
    tile_id = Column(String(50), nullable=False, unique=True)
    label = Column(String(100), nullable=False)
    endpoint = Column(String(200), nullable=False)
    method = Column(String(10), default="GET")
    icon = Column(String(50), nullable=True)
    is_active = Column(Boolean, default=True)
    sort_order = Column(Integer, default=0)
    badge_enabled = Column(Boolean, default=False)       # НОВОЕ
    badge_endpoint = Column(String(200), nullable=True)  # НОВОЕ
    created_at = Column(DateTime(timezone=True), server_default=func.now())
    updated_at = Column(DateTime(timezone=True), onupdate=func.now())
