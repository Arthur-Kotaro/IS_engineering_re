from sqlalchemy import Column, String, Text, Integer, Boolean
from app.database import Base


class ProjectType(Base):
    __tablename__ = "project_types"

    code = Column(String(50), primary_key=True)
    name_ru = Column(String(100), nullable=False)
    name_en = Column(String(100), nullable=False)
    description_ru = Column(Text, nullable=True)
    sort_order = Column(Integer, nullable=False, default=0)
    is_active = Column(Boolean, nullable=False, default=True)

    def __repr__(self):
        return f"<ProjectType({self.code})>"
