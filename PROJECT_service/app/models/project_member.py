# app/models/project_member.py
from sqlalchemy import Column, Integer, String, DateTime, ForeignKey, UniqueConstraint, Index
from sqlalchemy.sql import func
from sqlalchemy.orm import relationship
from app.database import Base


class ProjectRole:
    OWNER = "owner"
    MANAGER = "manager"
    EDITOR = "editor"
    VIEWER = "viewer"

    ALL = (OWNER, MANAGER, EDITOR, VIEWER)

    ROLE_PERMISSIONS = {
        OWNER:    {"view_project", "edit_project", "edit_mastergraphic", "manage_members", "delete_project"},
        MANAGER:  {"view_project", "edit_project", "edit_mastergraphic", "manage_members"},
        EDITOR:   {"view_project", "edit_mastergraphic"},
        VIEWER:   {"view_project"},
    }

    @classmethod
    def permissions_for(cls, role: str):
        return cls.ROLE_PERMISSIONS.get(role, set())


class ProjectMember(Base):
    __tablename__ = "project_members"

    id = Column(Integer, primary_key=True, autoincrement=True)
    project_id = Column(Integer, ForeignKey("projects.project_id", ondelete="CASCADE"), nullable=False)
    user_id = Column(Integer, nullable=False)
    role = Column(String(50), nullable=False, default=ProjectRole.VIEWER)
    joined_at = Column(DateTime(timezone=True), server_default=func.now())

    project = relationship("Project", back_populates="members")

    __table_args__ = (
        UniqueConstraint("project_id", "user_id", name="uq_project_user"),
        Index("idx_pm_user", "user_id"),
        Index("idx_pm_project_user", "project_id", "user_id"),
    )

    def __repr__(self):
        return f"<ProjectMember(project_id={self.project_id}, user_id={self.user_id}, role='{self.role}')>"
