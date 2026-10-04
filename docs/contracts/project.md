# Project Service — контракты

**Порт:** 8001.

## Публичные (защищённые JWT через nginx)

### GET /api/v1/projects/list

**Query:** `skip`, `limit`, `status_filter`.

**Ответ 200:** `List[ProjectResponse]`.

### POST /api/v1/projects/create

**Body:** `ProjectCreate`.

**Ответ 201:** `ProjectResponse`. Создатель автоматически добавляется как `owner`.

### GET /api/v1/projects/list-with-access

**Query:** `permission` (по умолчанию `view_project`).

**Ответ 200:** `List[ProjectResponse]` — только проекты, где у пользователя есть permission. Поля `access_via`, `role`, `via_user_id` заполнены.

### GET /api/v1/projects/{id}

**Ответ 200:** `ProjectDetailResponse` (с участниками).

### PUT /api/v1/projects/{id}

**Body:** `ProjectUpdate`. Требует `edit_project`.

### PATCH /api/v1/projects/{id}/status?status=

Требует `edit_project`. Валидные: `draft`, `active`, `suspended`, `completed`, `cancelled`.

### DELETE /api/v1/projects/{id}

Требует `delete_project` (только owner).

**Ответ 204.**

### POST /api/v1/projects/{id}/members

**Body:** `{"user_id": int, "role": "owner|manager|editor|viewer"}`.

Требует `manage_members`.

### DELETE /api/v1/projects/{id}/members/{user_id}

Требует `manage_members`.

### GET /api/v1/projects/{id}/check-access?user_id=&permission=

**Ответ 200:**
```json
{
  "has_access": true,
  "reason": "direct | subordinate_access | delegation | none",
  "role": "owner | manager | editor | viewer | null",
  "via_user_id": int | null,
  "project_status": "active"
}
```

Внутренние (без JWT, доверяют локальной сети)

GET /internal/projects/list-with-access?user_id=&permission=

Ответ 200: List[ProjectResponse].

GET /internal/projects/{id}/check-access?user_id=&permission=

Ответ 200: CheckAccessResponse.

Модель прав

Роли: owner, manager, editor, viewer.

Права:

· view_project — все роли.
· edit_project — owner, manager.
· edit_mastergraphic — owner, manager, editor.
· manage_members — owner, manager.
· delete_project — owner.

Композиция:

```
effective_permission(user, project, permission) =
    direct_permission(user, project, permission)         # project_members
    OR subordinate_permission(user, project, permission) # подчинённые user'а
    OR delegated_permission(user, project, permission)   # активные делегации
```

Источники:

· Иерархия — User Service, GET /api/v1/users/{id}/subordinates.
· Делегации — Delegation Service, GET /api/v1/delegations/active/{id}.

Схемы

ProjectResponse

```json
{
  "project_id": int,
  "title": "string",
  "description": "string | null",
  "status": "draft | active | suspended | completed | cancelled",
  "created_by": int,
  "created_at": "datetime",
  "updated_at": "datetime",
  "members_count": int,
  "access_via": "direct | subordinate_access | delegation | null",
  "role": "string | null",
  "via_user_id": "int | null"
}
```

CheckAccessResponse

```json
{
  "has_access": bool,
  "reason": "direct | subordinate_access | delegation | none",
  "role": "string | null",
  "via_user_id": "int | null",
  "project_status": "string | null"
}
```

