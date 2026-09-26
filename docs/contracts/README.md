# Контракты сервисов

## Формат обмена
- HTTP/1.1, JSON.
- Заголовки — см. ADR-0008.

## Аутентификация
- POST /api/v1/auth/login → {access_token, refresh_token, token_type, requires_password_change}
- POST /api/v1/auth/refresh → {access_token, refresh_token}
- POST /api/v1/auth/logout → {message}
- POST /internal/auth/verify — заголовки или 401

## Пользователи
- GET /api/v1/users/me → UserResponse
- GET /api/v1/users/{id} → UserResponse
- GET /api/v1/hr/users/search?query= → List[UserResponse]
- GET /internal/users/{id}/subordinates → List[{user_id, user_name}]

## Проекты
- GET /api/v1/projects/list → List[ProjectResponse]
- GET /api/v1/projects/{id} → ProjectDetailResponse
- GET /internal/projects/list-with-access?user_id=&permission= → List[ProjectResponse]

## Делегации
- POST /api/v1/delegations/direct → DelegationResponse
- POST /api/v1/delegations/temporary → DelegationResponse
- GET /api/v1/delegations/for-user/{user_id} → List[DelegationResponse]
- GET /internal/delegations/check?delegate_id=&donor_id= → {has_delegation: bool}

## UI Composer
- GET /page/{path} → {title, widgets, context: {session_id, current_step}}
- POST /workflow/event → тот же формат

## Ошибки
{
  "detail": "string",
  "error_type": "string | null",
  "context": {}
}

HTTP-коды: 200, 400, 401, 403, 404, 409, 500.
