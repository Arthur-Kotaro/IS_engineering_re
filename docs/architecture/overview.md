# Обзор архитектуры ERP «Engineering:re»

## Компоненты

Client (Qt6 QML) → Nginx :8080 → Auth :8010 /verify + сервисы
├─ User :8000
├─ Project :8001
├─ Mastergraph :8003
├─ Navigation :8009
├─ Auth :8010
├─ Delegation :8011
├─ Notification :8012
└─ UI Composer :8020

UI Composer ходит в бизнес-сервисы напрямую для sources шаблонов.
MG Service ходит в Project Service для проверки прав.

## Клиентские рендеринг-движки

В клиенте выделены три независимых рендеринг-движка (ADR 0014):

1. **PRE (Page Rendering Engine)** — рендеринг страниц. Генерирует QML-компоненты из JSON (форм, таблиц, кнопок). Работает внутри вкладок (`TabContent`).
2. **BDERE (Block Diagram Editor Rendering Engine)** — рендеринг блочных диаграмм. Отдельный движок (отложен). Работает в отдельном окне (`BlockEditorWindow`).
3. **GDERE (Gantt Diagram Editor Rendering Engine)** — рендеринг диаграмм Ганта. Отдельный движок (в работе). Работает в отдельном окне (`GanttEditorWindow`). Использует WebSocket для real-time синхронизации.

Открытие окон редакторов реализуется через сигнал `ganttEditorRequested` (от `WidgetBridge` к `MainWindow`).

## Порты

| Сервис | Порт | Хранилище |
|---|---|---|
| User | 8000 | PostgreSQL |
| Project | 8001 | PostgreSQL |
| PJP | 8002 | заглушка |
| Mastergraphics (MG) | 8003 | PostgreSQL |
| PROTO | 8004 | заглушка |
| BUS (Base Universal Service) | 8005+ | PostgreSQL (по экземпляру на группу задач) |
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
- departments (UUID, 6 уровней, 39 записей)
- positions (19 записей)
- roles (57 записей)
- users (95 записей)
- users_roles (many-to-many)

### Project Service
- projects (3 записи, тест)
- project_roles (10 ролей)
- project_members (21 запись, тест)

### Mastergraphics Service (MG)
- mastergraphs (1 тестовый)
- task_groups (6 для MVP)
- mg_tasks
- mg_dependencies

### BUS (Base Universal Service)
- local_graphs (ЛГ)
- lg_tasks
- lg_dependencies

### Navigation Service
- tiles (8 плиток)
- tiles_by_role (8 связей)
- tiles_universal (2)

## Ключевые решения

- **Роль** — производная от `(position, dept)`.
- **Иерархия** — через `departments.parent_dept_id`.
- **Один пользователь** — одна должность, много ролей.
- **JWT** содержит: `user_id`, `position`, `roles[]`, `is_super_admin`.
- **Плитки:** 3 источника (по роли, по должности, универсальные).
- **Права на проект:** через `project_members` + `projects`.
- **MG ↔ BUS:** MG запрашивает у BUS прогнозные сроки для задач; BUS хранит локальные графики. При изменении ЛГ BUS уведомляет MG.
- **Версионирование:** два уровня (сущность + диаграмма). Для optimistic locking.
- **Diff:** передаётся по WebSocket (планируется).

## Ссылки

- ADR-0010 — роли, должности, подразделения.
- ADR-0014 — клиентские рендеринг-движки (PRE, BDERE, GDERE).
- ADR-0005 — модель прав на проект.
- ADR-0001 — gateway.
- `docs/domain/enterprise/` — админ-структура.
