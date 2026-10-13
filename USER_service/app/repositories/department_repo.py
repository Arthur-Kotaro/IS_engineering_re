# app/repositories/department_repo.py
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, update
from typing import Optional, List
from app.models.department import Department


class DepartmentRepository:
    def __init__(self, db: AsyncSession):
        self.db = db

    async def get_by_id(self, dept_id) -> Optional[Department]:
        result = await self.db.execute(
            select(Department).where(Department.dept_id == dept_id)
        )
        return result.scalar_one_or_none()

    async def get_by_code(self, dept_code: str) -> Optional[Department]:
        result = await self.db.execute(
            select(Department).where(Department.dept_code == dept_code)
        )
        return result.scalar_one_or_none()

    async def get_all(self, skip: int = 0, limit: int = 500) -> List[Department]:
        result = await self.db.execute(
            select(Department).order_by(Department.level, Department.dept_code).offset(skip).limit(limit)
        )
        return result.scalars().all()

    async def get_children(self, parent_dept_id) -> List[Department]:
        result = await self.db.execute(
            select(Department).where(Department.parent_dept_id == parent_dept_id).order_by(Department.dept_code)
        )
        return result.scalars().all()

    async def get_by_level(self, level: int) -> List[Department]:
        result = await self.db.execute(
            select(Department).where(Department.level == level).order_by(Department.dept_code)
        )
        return result.scalars().all()

    # --- Руководитель подразделения ---

    async def get_head(self, dept_id) -> Optional[Department]:
        """Возвращает Department с заполненным head_user_id (или None)."""
        result = await self.db.execute(
            select(Department).where(Department.dept_id == dept_id)
        )
        return result.scalar_one_or_none()

    async def set_head(self, dept_id, user_id: int) -> bool:
        """Назначает пользователя руководителем подразделения."""
        result = await self.db.execute(
            update(Department)
            .where(Department.dept_id == dept_id)
            .values(head_user_id=user_id)
        )
        await self.db.commit()
        print(f"[department_repo] set_head: dept={dept_id} user={user_id} rows={result.rowcount}")
        return result.rowcount > 0

    async def clear_head(self, dept_id) -> bool:
        """Снимает руководителя с подразделения."""
        result = await self.db.execute(
            update(Department)
            .where(Department.dept_id == dept_id)
            .values(head_user_id=None)
        )
        await self.db.commit()
        print(f"[department_repo] clear_head: dept={dept_id} rows={result.rowcount}")
        return result.rowcount > 0
