# User Service — контракты

**Порт:** 8000.
**Отдельный репозиторий:** `git@github.com:Arthur-Kotaro/USER_service.git`.

## Публичные эндпоинты

### POST /api/v1/auth/login

**Body:**
```json
{"email": "string", "password": "string"}
```

Ответ 200:

```json
{
  "access_token": "string",
  "refresh_token": "string",
  "token_type": "bearer",
  "requires_password_change": false
}
```

POST /api/v1/auth/refresh

Body: {"refresh_token": "string"}.

Ответ 200: то же.

POST /api/v1/auth/reset-password

Body: {"email": "string"}.

Ответ 200: {"message": "string"}.

POST /api/v1/auth/register

(Не реализован. Пользователи создаются через HR/Admin.)

Защищённые эндпоинты

POST /api/v1/auth/logout

Заголовки: Authorization.

Действия:

1. Отзывает access-токен — пишет blacklist:{jti} в Redis DB 0 с TTL = exp - now.
2. Отзывает все refresh-токены пользователя в Postgres.

Ответ 200: {"message": "Successfully logged out"}.

POST /api/v1/auth/change-password

Body: {"current_password": "...", "new_password": "...", "confirm_password": "..."}.

Ответ 200: {"message": "..."}.

GET /api/v1/auth/me

Ответ 200: UserResponse.

GET /api/v1/users/me

Ответ 200: UserResponse.

PUT /api/v1/users/me

Body: UserUpdate.

Ответ 200: UserResponse.

GET /api/v1/users/{id}

Ответ 200: UserResponse.

GET /api/v1/users/{id}/manager

Ответ 200: {"manager_id": int | null, "manager_name": string, "manager_email": string}.

GET /api/v1/users/{id}/subordinates

Ответ 200:

```json
[{"user_id": 1, "user_name": "string", "full_name": "string", "email": "string"}]
```

HR (защищённые, требуют роль hr или admin)

· GET /api/v1/hr/departments
· GET /api/v1/hr/roles
· GET /api/v1/hr/users/search?query=&limit=
· GET /api/v1/hr/users/{id}
· POST /api/v1/hr/users (создание)
· PUT /api/v1/hr/users/{id}
· POST /api/v1/hr/users/{id}/roles?role_id=
· DELETE /api/v1/hr/users/{id}/roles/{role_id}
· POST /api/v1/hr/users/{id}/block?reason=
· POST /api/v1/hr/users/{id}/unblock

Admin (защищённые, требуют роль admin или super_admin)

· GET /api/v1/admin/departments
· GET /api/v1/admin/roles
· GET /api/v1/admin/users?skip=&limit=&include_deleted=&only_active=
· GET /api/v1/admin/users/deleted
· GET /api/v1/admin/users/{id}
· PUT /api/v1/admin/users/{id}
· POST /api/v1/admin/users/{id}/block (body: BlockUserRequest)
· POST /api/v1/admin/users/{id}/unblock
· DELETE /api/v1/admin/users/{id}
· POST /api/v1/admin/users/{id}/restore
· POST /api/v1/admin/users/{id}/roles?role_id=
· DELETE /api/v1/admin/users/{id}/roles/{role_id}
· POST /api/v1/admin/users/{id}/reset-password?new_password=
· GET /api/v1/admin/stats

Internal (защищённые X-Internal-Key)

· GET /internal/users?search=&limit=
· GET /internal/users/{id}

Internal (защищённые JWT)

· POST /internal/auth/impersonate

Body: {"donor_id": int}.

Ответ 200:

```json
{
  "access_token": "string",
  "token_type": "bearer",
  "impersonation_id": "uuid",
  "recipient_id": int,
  "donor_id": int,
  "expires_in": 7200
}
```

Ошибки:

· 403 — нет права имперсонировать.
· 404 — донор не найден.
· 400 — донор удалён или заблокирован.
· POST /internal/auth/impersonation/close

Body: {"impersonation_id": "uuid"}.

Ответ 200: {"message": "Impersonation closed"}.

События

Публикует в Redis DB 3, канал user.events:

· user.deleted — {user_id, payload: {by}}.
· user.blocked — {user_id, payload: {reason}}.

Схемы

UserResponse

```json
{
  "user_id": int,
  "user_name": "string",
  "full_name": "string | null",
  "email": "string | null",
  "gender": "M | F | null",
  "birth_date": "datetime | null",
  "dept_code": "string | null",
  "phone_work": "string | null",
  "phone_mobile": "string | null",
  "head_id": "int | null",
  "is_super_admin": bool,
  "status": "active | blocked | locked | deleted",
  "roles": ["string"],
  "created_at": "datetime",
  "updated_at": "datetime",
  "last_login_at": "datetime | null"
}
```
