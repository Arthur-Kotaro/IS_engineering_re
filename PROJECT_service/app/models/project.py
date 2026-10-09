# app/models/project.py
from sqlalchemy import Column, BigInteger, String, Text, Integer, DateTime, Index
from sqlalchemy.sql import func
from sqlalchemy.orm import relationship
from app.database import Base


class ProjectStatus:
    DRAFT = "draft"
    ACTIVE = "active"
    SUSPENDED = "suspended"
    COMPLETED = "completed"
    CANCELLED = "cancelled"


class Project(Base):
    __tablename__ = "projects"

    project_id = Column(BigInteger, primary_key=True, autoincrement=True)
    title = Column(String(200), nullable=False)
    description = Column(Text, nullable=True)
    priority = Column(Integer, nullable=False, default=0)
    status = Column(String(20), nullable=False, default=ProjectStatus.DRAFT, index=True)

    chief_engineer_id = Column(BigInteger, nullable=True, index=True)
    planning_engineer_id = Column(BigInteger, nullable=True, index=True)

    created_by = Column(BigInteger, nullable=False, index=True)
    created_at = Column(DateTime(timezone=True), server_default=func.now())
    updated_at = Column(DateTime(timezone=True), server_default=func.now(), onupdate=func.now())

    members = relationship("ProjectMember", back_populates="project", cascade="all, delete-orphan")

    __table_args__ = (
        Index("idx_projects_status", "status"),
        Index("idx_projects_priority", "priority"),
    )

    def __repr__(self):
        return f"<Project(id={self.project_id}, title='{self.title}', status='{self.status}')>"
