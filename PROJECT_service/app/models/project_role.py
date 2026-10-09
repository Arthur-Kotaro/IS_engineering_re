# app/models/project_role.py
from sqlalchemy import Column, String, Text
from app.database import Base


class ProjectRole(Base):
    __tablename__ = "project_roles"

    role_code = Column(String(50), primary_key=True)
    role_name_en = Column(String(100), nullable=False)
    role_name_ru = Column(String(100), nullable=False)
    role_description_ru = Column(Text)

    def __repr__(self):
        return f"<ProjectRole(code={self.role_code}, name_ru={self.role_name_ru})>"
