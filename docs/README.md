# Документация ERP «Engineering:re»

**Версия:** 4.0
**Дата:** 2026-10-09
**Статус:** Спираль 6.6 (Roles & Positions & Departments) в работе.

## Структура
docs/
├── README.md
├── adr/
│ ├── 0001-gateway.md
│ ├── 0002-blacklist.md
│ ├── 0003-navigation-storage.md
│ ├── 0004-delegation.md
│ ├── 0005-access-model.md ← переписан
│ ├── 0006-impersonation.md
│ ├── 0007-workflow.md
│ ├── 0008-auth-model.md
│ ├── 0009-alembic.md
│ └── 0010-roles-positions-departments.md ← новый
├── domain/
│ ├── glossary.md ← обновлён
│ └── enterprise/
│ ├── README.md
│ ├── administrative-structure.md
│ └── upp-structure.md
├── architecture/
│ ├── overview.md
│ └── request-flows.md
├── contracts/
├── roadmap/
│ └── spirals.md ← обновлён
└── audit/


## Ключевое

**Модель пользователя:**
- Одна должность (`positions`).
- Несколько ролей (`roles` через `users_roles`).
- Одно подразделение (`departments.dept_id`).
- Иерархия сотрудников — через `departments.parent_dept_id`.

**Модель роли:**
- Роль — производная от `(position, dept)`.
- В JWT: `position` (одна) + `roles` (список).

**Модель проекта:**
- `project_roles` — справочник.
- `project_members.member_id` + `role_code` + `user_id`.
- Права через `project_members` + `projects`.

**Модель плиток:**
- `tiles` + `tiles_by_role` + `tiles_by_position` + `tiles_universal`.
- `admin` — все плитки.

## Правила работы с git

- Одна ветка `main`.
- Коммит после каждой проверенной порции.
- USER_service — отдельный репозиторий.

## Запуск

```bash
cd /home/kotaro/code/IS_engineering_re
./start_all.sh -d
./status.sh
