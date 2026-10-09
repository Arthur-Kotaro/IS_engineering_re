# ADR-0010: Роли, должности, подразделения

## Статус
Accepted

## Дата
2026-10-09

## Контекст

До этой спирали модель ролей была смешанной:
- Таблица `roles` содержала и роли, и должности.
- Пользователь имел `dept_code` (строку), `head_id`, `projects`.
- В Project Service были выдуманные роли `owner/manager/editor/viewer`.
- Navigation Service фильтровал плитки по глобальным ролям.

Это не соответствует реальной матричной структуре предприятия.

## Решение

### Три справочника в User Service

**`departments`** — организационная структура.
- 6 уровней: `division → directorate → department → section/shop → bureau/group/area/store`.
- `dept_id UUID`, `dept_code` (читаемый), `dept_name_en`, `dept_name_ru`, `parent_dept_id`, `type`, `level`.
- 39 подразделений.

**`positions`** — обобщённые должности.
- `position_code`, `position_name_en`, `position_name_ru`.
- 19 записей. Примеры: `director`, `section_head`, `engineer`, `operator`.
- Одна должность на пользователя.

**`roles`** — разрешённые роли.
- `role_code`, `role_name_en`, `role_name_ru`.
- 57 записей. Включают административные (`admin`, `hr`), руководящие по подразделениям (`division_head`, `director`), специализированные (`welder`, `planner`, `cost_engineer`).
- Может быть несколько на пользователя. Связь через `users_roles`.

### Отменено

- `users.head_id` — иерархия вытекает из `departments.parent_dept_id`.
- `users.dept_code` (строка) — заменено на `users.dept_id` (UUID).
- `users.projects` — проектные связи только в Project Service.
- Должность = роль — **не всегда**. Роль — производная от `(position, dept)`.

### Правила

**Роль — производная от должности и подразделения.**
- Пример: `section_head` в КТО → роль `kto_head` (`section_head` + `UPP_KTO`).
- Пример: `section_head` в Отделе экономистов → роль `econ_head`.
- Причина: разный функционал, разные плитки, разный GUI.

**Один пользователь — одна должность, может быть несколько ролей.**

**В проде:** должность + роли задаёт HR при создании пользователя.
**Сейчас:** задаём через SQL для тестов.

## Project Service

**`project_roles`** — справочник проектных ролей (10 записей):
`chief_engineer`, `prototype_pm`, `industrialization_manager`, `architect`,
`validator`, `test_specialist`, `planning_engineer`, `quality_engineer`,
`project_economist`, `project_member`.

**`project_members`** — участники проекта:
- `member_id` (surrogate), `project_id`, `role_code`, `user_id`.
- `UNIQUE (project_id, user_id)` — один человек, одна роль в проекте.

**`projects.chief_engineer_id` и `planning_engineer_id`** — оставлены для быстрого доступа.

**Права на проект:**
- `view_project` — все члены команды.
- `edit_mastergraphic` — CVE и IPP проекта.
- `manage_members` — CVE и IPP.
- `approve_members` — CVE.
- `edit_project` — CVE.
- `delete_project` — не реализовано.

**Права проверяются через `project_members`** (роль + user_id) и `projects` (CE/IPP).

**Отменены:** `owner/manager/editor/viewer`, `subordinate_access`, `delegation_access`, `via_user_id`, `access_via`.

## Navigation Service

**Три таблицы плиток:**
- `tiles` — справочник.
- `tiles_by_role` — привязка к разрешённым ролям.
- `tiles_by_position` — привязка к должностям (пока пусто).
- `tiles_universal` — для всех (`profile`, `impersonate`).

**Алгоритм:**

result = tiles_by_role(user.roles) ∪ tiles_by_position(user.position) ∪ tiles_universal
if "admin" in user.roles: result = все tiles


## Auth Service

**`/verify` возвращает:**
- `X-User-ID`
- `X-User-Position` (одна должность)
- `X-User-Role` (разрешённые роли через запятую или `super_admin`)
- `X-Impersonated-By` (опционально)

**JWT payload:**
```json
{
  "user_id": 9,
  "position": "bureau_head",
  "roles": ["proto_purchaser"],
  "is_super_admin": false,
  "jti": "...",
  "type": "access"
}
Последствия
Положительные

    Модель соответствует реальной структуре предприятия.

    Разделение должности и роли даёт гибкость.

    Роль учитывает подразделение → разный GUI и функционал.

    Navigation Service — три источника плиток.

    Project Service — справочник реальных проектных ролей.

Отрицательные

    Миграция существующей БД (на ноутбуке) невозможна — данные пересоздаются с нуля.

    Код сервисов переписывается полностью.

    Документация переписывается.

Ссылки

    ADR-0005: Модель прав на проект (переписан).

    ADR-0008: Модель аутентификации.

    docs/domain/enterprise/ — административная структура.
    EOF
