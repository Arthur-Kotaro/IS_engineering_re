# ADR-0005: Модель прав на проект

## Статус
Accepted

## Дата
2026-09-26

## Контекст

Project Service — источник прав на проект. Права проверяются при действиях над проектом и его мастерграфиком.

## Модель

### Роли в проекте

- `owner` — владелец (создатель).
- `manager` — управляющий.
- `editor` — редактор.
- `viewer` — наблюдатель.

Роль в проекте — **одна** для пользователя. Не может быть одновременно editor и viewer.

### Права (permissions)

- `view_project`
- `edit_project`
- `edit_mastergraphic`
- `manage_members`
- `delete_project`

### Соответствие ролей и прав

| Роль | view_project | edit_project | edit_mastergraphic | manage_members | delete_project |
|---|---|---|---|---|---|
| owner | ✅ | ✅ | ✅ | ✅ | ✅ |
| manager | ✅ | ✅ | ✅ | ✅ | ❌ |
| editor | ✅ | ❌ | ✅ | ❌ | ❌ |
| viewer | ✅ | ❌ | ❌ | ❌ | ❌ |

## Композиция прав

effective_permission(user, project, permission) =
direct_permission(user, project, permission)
OR subordinate_permission(user, project, permission)
OR delegated_permission(user, project, permission)


- **Direct:** `project_members(user_id, project_id, role)` и `permission in role.permissions`.
- **Subordinate:** начальник действует от имени подчинённого (1 уровень). Для каждого подчинённого `S` проверяется `direct_permission(S, project, permission)`.
- **Delegated:** для каждой активной делегации, где user = `delegate_id`, проверяется `direct_permission(delegator_id, project, permission)`.

## Ответ на проверку

json
{
  "has_access": true,
  "reason": "direct" | "subordinate_access" | "delegation" | "none",
  "role": "owner",
  "via_user_id": 101,
  "project_status": "active"
}

via_user_id — подчинённый или делегатор, от имени которого действует пользователь. При direct — null.

Эндпоинты

Публичные (через X-User-ID):

· GET /api/v1/projects/list-with-access?permission=
· GET /api/v1/projects/{id}/check-access?user_id=&permission=

Внутренние:

· GET /internal/projects/list-with-access?user_id=&permission=
· GET /internal/projects/{id}/check-access?user_id=&permission=

Источник иерархии

User Service, эндпоинт GET /api/v1/users/{id}/subordinates.
Кэш в Project Service — TTL 30 сек (в будущем).

Источник делегаций

Delegation Service, эндпоинт GET /api/v1/delegations/active/{delegate_id}.
Запрос выполняется Project Service'ом напрямую (не через nginx, не через gateway).

Аудит

При действии через подчинённого или делегацию, сервисы, использующие права (MG, Project), сохраняют:

· actor_id — реальный действующий пользователь (X-Impersonated-By или X-User-ID).
· acting_as_id — донор (X-User-ID).
· access_reason — direct | subordinate_access | delegation.
· via_user_id — подчинённый или делегатор.

