# Обзор архитектуры ERP «Engineering:re»

## Компоненты

Client (Qt6 QML) → Nginx :8080 → Auth :8010 /verify + сервисы
├─ User :8000
├─ Project :8001
├─ Mastergraph :8003 (план)
├─ Navigation :8009
├─ Auth :8010
├─ Delegation :8011
├─ Notification :8012
└─ UI Composer :8020

UI Composer ходит в бизнес-сервисы напрямую для sources шаблонов.


## Порты

| Сервис | Порт | Хранилище |
|---|---|---|
| User | 8000 | PostgreSQL |
| Project | 8001 | PostgreSQL |
| PJP | 8002 | заглушка |
| Mastergraphics | 8003 | план |
| PROTO | 8004 | заглушка |
| Navigation | 8009 | SQLite |
| Auth | 8010 | Redis |
| Delegation | 8011 | PostgreSQL |
| Notification | 8012 | PostgreSQL |
| UI Composer | 8020 | Redis (сессии) |
| Nginx | 8080 | — |

## Redis DB

| DB | Назначение |
|---|---|
| 0 | blacklist + impersonation |
| 1 | workflow-сессии |
| 3 | события `user.events` |

## Модель данных

### User Service

departments (UUID, 6 уровней, 39 записей)
positions (19 записей)
roles (57 записей)
users (95 записей)
users_roles (many-to-many)
refresh_tokens
login_history


### Project Service

projects (3 записи, тест)
project_roles (10 ролей)
project_members (21 запись, тест)


### Navigation Service

tiles (7 плиток)
tiles_by_role (5 связей)
tiles_by_position (пусто)
tiles_universal (2)


### Delegation Service

delegations
delegation_history
delegation_rules (10 правил)


### Notification Service

notifications

## Ключевые решения

- Роль — производная от `(position, dept)`.
- Иерархия — через `departments.parent_dept_id`.
- Один пользователь — одна должность, много ролей.
- JWT содержит: `user_id`, `position`, `roles[]`, `is_super_admin`.
- Плитки: 3 источника.
- Права на проект: через `project_members` + `projects`.

## Ссылки

- ADR-0010 — роли, должности, подразделения.
- ADR-0005 — модель прав на проект.
- ADR-0001 — gateway.
- `docs/domain/enterprise/` — админ-структура.
