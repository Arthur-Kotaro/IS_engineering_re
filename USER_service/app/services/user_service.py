# app/services/user_service.py
from typing import Optional, List, Dict, Any
from fastapi import HTTPException, status
from app.repositories.user_repo import UserRepository
from app.repositories.role_repo import RoleRepository
from app.repositories.department_repo import DepartmentRepository
from app.repositories.position_repo import PositionRepository
from app.schemas.user import (
    UserResponse, UserCreate, user_to_response,
    UserCreatedResponse, ManagerInfo,
)
from app.schemas.errors import ValidationError, ValidationErrorItem
from app.models.user import User
from app.utils.hasher import hash_password, generate_temporary_password
from datetime import datetime, timezone, timedelta


# Коды должностей, которые считаются руководящими.
# area_foreman (мастер участка) НЕ входит: на участке до 3 смен,
# у каждой смены свой мастер.
HEAD_POSITION_CODES = {
    "director",
    "division_head",
    "department_head",
    "section_head",
    "bureau_head",
    "shop_head",
}


TEMP_PASSWORD_TTL_HOURS = 48


class UserService:
    def __init__(
        self,
        user_repo: UserRepository,
        role_repo: RoleRepository,
        department_repo: Optional[DepartmentRepository] = None,
        position_repo: Optional[PositionRepository] = None,
    ):
        self.user_repo = user_repo
        self.role_repo = role_repo
        self.department_repo = department_repo
        self.position_repo = position_repo

    async def get_user_by_id(self, user_id: int, include_deleted: bool = False) -> Optional[UserResponse]:
        user = await self.user_repo.get_by_id(user_id, include_deleted=include_deleted)
        if not user:
            return None
        return user_to_response(user)

    async def get_user_by_email(self, email: str, include_deleted: bool = False) -> Optional[User]:
        return await self.user_repo.get_by_email(email, include_deleted=include_deleted)

    async def get_user_by_username(self, username: str, include_deleted: bool = False) -> Optional[User]:
        return await self.user_repo.get_by_username(username, include_deleted=include_deleted)

    async def get_all_users(
        self,
        skip: int = 0,
        limit: int = 100,
        include_deleted: bool = False,
        only_active: bool = True,
    ) -> List[UserResponse]:
        users = await self.user_repo.get_all(skip, limit, include_deleted, only_active)
        return [user_to_response(u) for u in users]

    # ========== Создание пользователя (HR) ==========

    async def create_user(
        self,
        user_data: UserCreate,
        created_by: Optional[int] = None,
    ) -> UserCreatedResponse:
        """
        Создаёт пользователя.

        1. Собирает все ошибки бизнес-валидации.
        2. Если есть хотя бы одна — бросает ValidationError.
        3. Генерирует временный пароль, хеширует.
        4. Сохраняет пользователя с temp_password.
        5. Если должность руководящая — назначает head_user_id подразделения.
        6. Возвращает UserCreatedResponse с plaintext-паролем и данными руководителя.
        """
        errors: List[ValidationErrorItem] = []

        print(f"[user_service] create_user: start email={user_data.email} user_name={user_data.user_name}")

        # 1. Email
        existing_email = await self.user_repo.get_by_email(user_data.email)
        if existing_email:
            errors.append(ValidationErrorItem(
                field="email",
                code="already_exists",
                message=f"Email «{user_data.email}» уже занят",
            ))

        # 2. user_name
        existing_username = await self.user_repo.get_by_username(user_data.user_name)
        if existing_username:
            errors.append(ValidationErrorItem(
                field="user_name",
                code="already_exists",
                message=f"Логин «{user_data.user_name}» уже занят",
            ))

        # 3. Позиция
        position = None
        if self.position_repo:
            position = await self.position_repo.get_by_id(user_data.position_id)
            if not position:
                errors.append(ValidationErrorItem(
                    field="position_id",
                    code="not_found",
                    message=f"Должность с id={user_data.position_id} не найдена",
                ))

        # 4. Руководитель подразделения
        is_head_position = position is not None and position.position_code in HEAD_POSITION_CODES
        existing_head: Optional[User] = None
        department = None

        if is_head_position and user_data.dept_id and self.department_repo:
            department = await self.department_repo.get_by_id(user_data.dept_id)
            if not department:
                errors.append(ValidationErrorItem(
                    field="dept_id",
                    code="not_found",
                    message=f"Подразделение с id={user_data.dept_id} не найдено",
                ))
            elif department.head_user_id:
                existing_head = await self.user_repo.get_by_id(department.head_user_id, include_deleted=True)
                head_name = existing_head.full_name if existing_head else f"id={department.head_user_id}"
                head_position = (
                    existing_head.position.position_name_ru
                    if existing_head and existing_head.position else "—"
                )
                errors.append(ValidationErrorItem(
                    field="dept_id",
                    code="department_head_taken",
                    message=(
                        f"У подразделения «{department.dept_name_ru}» уже есть руководитель: "
                        f"{head_name}, {head_position}"
                    ),
                ))

        # 5. Если ошибки — бросаем разом
        if errors:
            print(f"[user_service] create_user: validation failed, errors={len(errors)}")
            raise ValidationError(detail="Validation failed", errors=errors)

        # 6. Генерируем временный пароль
        temp_password_plain = generate_temporary_password(12)
        temp_password_hash = hash_password(temp_password_plain)
        temp_password_expires_at = datetime.now(timezone.utc) + timedelta(hours=TEMP_PASSWORD_TTL_HOURS)
        print(f"[user_service] create_user: temp password generated, expires_at={temp_password_expires_at}")

        # 6a. В password_hash пишем "мусорный" хеш, чтобы по нему нельзя было войти.
        # Вход возможен только по temp_password. После смены пароля
        # (change_password) password_hash будет перезаписан настоящим.
        unusable_password_hash = hash_password(generate_temporary_password(12))

        # 7. Создаём пользователя (flush без commit — связи не истекают)
        user = await self.user_repo.create(
            email=user_data.email,
            username=user_data.user_name,
            hashed_password=unusable_password_hash,
            position_id=user_data.position_id,
            last_name=user_data.last_name,
            first_name=user_data.first_name,
            middle_name=user_data.middle_name,
            dept_id=user_data.dept_id,
            created_by=created_by,
            gender=user_data.gender,
            birth_date=user_data.birth_date,
            phone_work=user_data.phone_work,
            phone_mobile=user_data.phone_mobile,
        )
        # Сохраняем user_id в локальную переменную. Объект user может
        # "протухнуть" после следующих commit'ов, но id — уже есть.
        new_user_id = user.user_id
        print(f"[user_service] create_user: user_id={new_user_id} created (flushed)")

        # 8. Устанавливаем temp_password
        await self.user_repo.set_temp_password(
            new_user_id,
            temp_password_hash,
            temp_password_expires_at,
        )
        print(f"[user_service] create_user: user_id={new_user_id} temp_password set")

        # 9. Роли
        for role_id in (user_data.role_ids or []):
            await self.role_repo.assign_role_to_user(new_user_id, role_id)

        # 10. Назначаем руководителем подразделения
        if is_head_position and user_data.dept_id and self.department_repo:
            await self.department_repo.set_head(user_data.dept_id, new_user_id)
            print(f"[user_service] create_user: dept {user_data.dept_id} head_user_id = {new_user_id}")

        # 11. Перечитываем пользователя со связями
        user = await self.user_repo.get_by_id(new_user_id)
        user_response = user_to_response(user)

        # 12. Данные руководителя (для информационной страницы).
        # Если руководитель не назначен — возвращаем заглушку,
        # чтобы клиент показал осмысленный текст, а не пустые поля.
        manager_info: ManagerInfo = ManagerInfo(
            user_id=0,
            full_name="Руководитель ещё не назначен",
            position_name_ru=None,
            dept_name_ru=None,
            email=None,
            phone_work=None,
            phone_mobile=None,
        )

        if user.department and user.department.head_user_id and user.department.head_user_id != new_user_id:
            manager_user = await self.user_repo.get_by_id(user.department.head_user_id, include_deleted=True)
            if manager_user:
                manager_info = ManagerInfo(
                    user_id=manager_user.user_id,
                    full_name=manager_user.full_name,
                    position_name_ru=(
                        manager_user.position.position_name_ru
                        if manager_user.position else None
                    ),
                    dept_name_ru=(
                        manager_user.department.dept_name_ru
                        if manager_user.department else None
                    ),
                    email=manager_user.email,
                    phone_work=manager_user.phone_work,
                    phone_mobile=manager_user.phone_mobile,
                )

        print(f"[user_service] create_user: done user_id={new_user_id}")

        return UserCreatedResponse(
            user=user_response,
            temporary_password=temp_password_plain,
            manager=manager_info,
        )

    # ========== Обновление, удаление, блокировка ==========

    async def update_user(self, user_id: int, update_data: Dict[str, Any]) -> Optional[UserResponse]:
        forbidden = {
            "user_id", "password_hash", "created_at", "deleted_at",
            "blocked_at", "locked_until", "failed_login_attempts",
            "is_super_admin", "created_by",
        }
        filtered_data = {k: v for k, v in update_data.items() if k not in forbidden}

        user = await self.user_repo.update(user_id, filtered_data)
        if not user:
            return None
        return user_to_response(user)

    async def delete_user(self, user_id: int, soft: bool = True) -> bool:
        user = await self.user_repo.get_by_id(user_id)
        if not user:
            return False
        return await self.user_repo.delete(user_id, soft=soft)

    async def restore_user(self, user_id: int) -> Optional[UserResponse]:
        user = await self.user_repo.get_by_id(user_id, include_deleted=True)
        if not user or user.deleted_at is None:
            return None
        restored = await self.user_repo.restore(user_id)
        if not restored:
            return None
        user = await self.user_repo.get_by_id(user_id)
        return user_to_response(user)

    async def block_user(self, user_id: int, reason: str, blocked_by: int, expires_at=None) -> bool:
        user = await self.user_repo.get_by_id(user_id)
        if not user:
            return False
        return await self.user_repo.block_user(user_id, reason, blocked_by, expires_at)

    async def unblock_user(self, user_id: int) -> bool:
        user = await self.user_repo.get_by_id(user_id, include_deleted=True)
        if not user:
            return False
        return await self.user_repo.unblock_user(user_id)

    async def is_user_blocked(self, user_id: int) -> bool:
        user = await self.user_repo.get_by_id(user_id)
        if not user or user.blocked_at is None:
            return False
        from datetime import datetime, timezone
        if user.block_expires_at is not None:
            if user.block_expires_at <= datetime.now(timezone.utc):
                return False
        return True

    # ========== Роли ==========

    async def assign_role(self, user_id: int, role_id: int) -> bool:
        user = await self.user_repo.get_by_id(user_id)
        if not user:
            return False
        return await self.role_repo.assign_role_to_user(user_id, role_id)

    async def assign_role_by_code(self, user_id: int, role_code: str) -> bool:
        role = await self.role_repo.get_by_code(role_code)
        if not role:
            return False
        return await self.assign_role(user_id, role.role_id)

    async def remove_role(self, user_id: int, role_id: int) -> bool:
        user = await self.user_repo.get_by_id(user_id)
        if not user:
            return False
        return await self.role_repo.remove_role_from_user(user_id, role_id)

    async def get_user_roles(self, user_id: int) -> List[str]:
        user = await self.user_repo.get_by_id(user_id)
        if not user:
            return []
        return user.get_role_codes()

    async def get_user_role_objects(self, user_id: int) -> List:
        return await self.role_repo.get_user_roles(user_id)

    # ========== Фильтры ==========

    async def get_users_by_dept(self, dept_id) -> List[UserResponse]:
        users = await self.user_repo.get_by_dept(dept_id)
        return [user_to_response(u) for u in users]

    async def get_blocked_users(self, skip: int = 0, limit: int = 100) -> List[UserResponse]:
        users = await self.user_repo.get_blocked_users(skip, limit)
        return [user_to_response(u) for u in users]

    async def get_deleted_users(self, skip: int = 0, limit: int = 100) -> List[UserResponse]:
        users = await self.user_repo.get_deleted_users(skip, limit)
        return [user_to_response(u) for u in users]
