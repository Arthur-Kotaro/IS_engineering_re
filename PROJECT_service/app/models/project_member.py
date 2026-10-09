# app/models/project_member.py
from sqlalchemy import Column, Integer, DateTime, ForeignKey, UniqueConstraint, Index
from sqlalchemy.sql import func
from sqlalchemy.orm import relationship
from app.database import Base


class ProjectMember(Base):
    __tablename__ = "project_members"

    id = Column(Integer, primary_key=True, autoincrement=True)
    project_id = Column(Integer, ForeignKey("projects.project_id", ondelete="CASCADE"), nullable=False)
    user_id = Column(Integer, nullable=False)
    joined_at = Column(DateTime(timezone=True), server_default=func.now())

    project = relationship("Project", back_populates="members")

    __table_args__ = (
        UniqueConstraint("project_id", "user_id", name="uq_project_user"),
        Index("idx_pm_user", "user_id"),
        Index("idx_pm_project_user", "project_id", "user_id"),
    )

    def __repr__(self):
        return f"<ProjectMember(project_id={self.project_id}, user_id={self.user_id})>"
