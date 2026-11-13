from sqlalchemy import Column, String, Text, Integer, Boolean, ForeignKey
from app.database import Base


class Typology(Base):
    __tablename__ = "typologies"

    code = Column(String(50), primary_key=True)
    project_type = Column(String(50), ForeignKey("project_types.code"), nullable=False, index=True)
    name_ru = Column(String(100), nullable=False)
    name_en = Column(String(100), nullable=False)
    description_ru = Column(Text, nullable=True)
    sort_order = Column(Integer, nullable=False, default=0)
    is_active = Column(Boolean, nullable=False, default=True)

    def __repr__(self):
        return f"<Typology({self.code}, project_type={self.project_type})>"
