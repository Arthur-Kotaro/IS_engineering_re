# app/models/tile.py
from sqlalchemy import Column, String, Boolean, Integer, DateTime
from sqlalchemy.sql import func
from app.database import Base


class Tile(Base):
    __tablename__ = "tiles"

    tile_id = Column(String(50), primary_key=True)
    label = Column(String(200), nullable=False)
    endpoint = Column(String(200), nullable=False)
    method = Column(String(10), nullable=False, default="GET")
    icon = Column(String(50), nullable=True)
    is_active = Column(Boolean, nullable=False, default=True)
    sort_order = Column(Integer, nullable=False, default=0)
    badge_enabled = Column(Boolean, nullable=False, default=False)
    badge_endpoint = Column(String(200), nullable=True)
    created_at = Column(DateTime(timezone=True), server_default=func.now())


class TileByRole(Base):
    __tablename__ = "tiles_by_role"

    tile_id = Column(String(50), primary_key=True)
    role_code = Column(String(50), primary_key=True)


class TileByPosition(Base):
    __tablename__ = "tiles_by_position"

    tile_id = Column(String(50), primary_key=True)
    position_code = Column(String(50), primary_key=True)


class TileUniversal(Base):
    __tablename__ = "tiles_universal"

    tile_id = Column(String(50), primary_key=True)
