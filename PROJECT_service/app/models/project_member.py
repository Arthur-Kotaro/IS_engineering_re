# app/models/project_member.py
from sqlalchemy import Column, Integer, BigInteger, String, DateTime, ForeignKey, UniqueConstraint, Index
from sqlalchemy.sql import func
from sqlalchemy.orm import relationship
from app.database import Base


class ProjectMember(Base):
    __tablename__ = "project_members"

    member_id = Column(BigInteger, primary_key=True, autoincrement=True)
    project_id = Column(BigInteger, ForeignKey("projects.project_id", ondelete="CASCADE"), nullable=False)
    role_code = Column(String(50), ForeignKey("project_roles.role_code"), nullable=False)
    user_id = Column(BigInteger, nullable=False)
    joined_at = Column(DateTime(timezone=True), server_default=func.now(), nullable=False)

    project = relationship("Project", back_populates="members")
    role = relationship("ProjectRole")

    __table_args__ = (
        UniqueConstraint("project_id", "user_id", name="project_members_project_id_user_id_key"),
        Index("idx_pm_project", "project_id"),
        Index("idx_pm_role", "role_code"),
        Index("idx_pm_user", "user_id"),
    )

    def __repr__(self):
        return f"<ProjectMember(project_id={self.project_id}, user_id={self.user_id}, role={self.role_code})>"
