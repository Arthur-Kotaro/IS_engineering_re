# Обзор архитектуры ERP «Engineering:re»

## Схема

```

┌─────────────────────────────────────────────────────────┐
│  CLIENT (Qt6 QML, C++)                                  │
│  MainWindow + DelegatedWindow + AuthWindow              │
└──────────────────────┬──────────────────────────────────┘
│ HTTP :8080
▼
┌─────────────────────────────────────────────────────────┐
│  NGINX (API Gateway)                                    │
│  - auth_request → Auth                                  │
│  - proxy_cache (60s/10s)                                │
│  - маршрутизация по location                            │
│  - streaming (files, SSE)                               │
└──┬────────────────────────────────────────────────┬─────┘
│                                                │
│ subrequest                                     │ main request
▼                                                ▼
┌──────────────┐                          ┌──────────────────────┐
│ AUTH :8010   │                          │ Бизнес-сервисы       │
│ /verify      │                          │                      │
│ - JWT        │                          │ USER         :8000   │
│ - blacklist  │                          │ PROJECT      :8001   │
│ - imperson.  │                          │ MASTERGRAPH  :8003   │
└──────┬───────┘                          │ NAVIGATION   :8009   │
│                                  │ DELEGATION   :8011   │
▼                                  │ NOTIFICATION :8012   │
┌─────────┐                             │ UI COMPOSER  :8020   │
│ REDIS   │                             └──────────┬───────────┘
│ DB0 bl  │                                        │
│ DB1 wf  │◄───────────────────────────────────────┤
│ DB3 ev  │                                        │
└─────────┘                                        │
▼
┌─────────────────────┐
│ PostgreSQL          │
│ (per-service БД)    │
│ + SQLite Navigation │
└─────────────────────┘

```

## Сервисы

### AUTH_SERVICE :8010

**Назначение:** валидатор JWT.
**Эндпоинты:**
- `GET /health`
- `POST/GET/... /verify` — принимает `Authorization`, возвращает `X-User-ID`, `X-User-Role`, `X-Impersonated-By`, `X-Impersonation-Id` или 401.

**Читает:**
- Redis DB 0: `blacklist:{jti}`, `impersonation:{id}`.
**Не делает:** проксирование, маршрутизацию, issue токенов.

### USER_service :8000

**Назначение:** пользователи, роли, аутентификация, issue JWT.
**Отдельный репозиторий:** `git@github.com:Arthur-Kotaro/USER_service.git`.

**Публичные эндпоинты:**
- `POST /api/v1/auth/login`
- `POST /api/v1/auth/refresh`
- `POST /api/v1/auth/reset-password`
- `POST /api/v1/auth/register`

**Защищённые:**
- `POST /api/v1/auth/logout` — revokes access (Redis) + refresh (Postgres).
- `POST /api/v1/auth/change-password`
- `GET /api/v1/auth/password-expiry`
- `GET /api/v1/auth/me`
- `GET/PUT /api/v1/users/me`
- `GET /api/v1/users/`
- `GET /api/v1/users/{id}`
- `GET /api/v1/users/{id}/manager`
- `GET /api/v1/users/{id}/subordinates`
- `GET/POST /api/v1/hr/departments`, `/roles`, `/users/search`, `/users`
- `PUT /api/v1/hr/users/{id}`
- `POST /api/v1/hr/users/{id}/roles`, `/block`, `/unblock`
- `DELETE /api/v1/hr/users/{id}/roles/{role_id}`
- `GET /api/v1/admin/users`, `/users/{id}`, `/stats`, `/departments`, `/roles`, `/users/deleted`
- `PUT /api/v1/admin/users/{id}`
- `POST /api/v1/admin/users/{id}/block`, `/unblock`, `/restore`, `/reset-password`, `/roles`
- `DELETE /api/v1/admin/users/{id}`, `/users/{id}/roles/{role_id}`

**Внутренние:**
- `GET /internal/users` (X-Internal-Key)
- `GET /internal/users/{id}` (X-Internal-Key)
- `POST /internal/auth/impersonate`
- `POST /internal/auth/impersonation/close`

**Публикует события:** Redis DB 3, канал `user.events`.
- `user.deleted`, `user.blocked`.

### PROJECT_service :8001

**Назначение:** проекты, команды, права.
**Эндпоинты:**
- `GET /api/v1/projects/list`
- `POST /api/v1/projects/create`
- `GET /api/v1/projects/{id}`
- `PUT /api/v1/projects/{id}`
- `PATCH /api/v1/projects/{id}/status`
- `DELETE /api/v1/projects/{id}`
- `POST /api/v1/projects/{id}/members`
- `DELETE /api/v1/projects/{id}/members/{user_id}`
- `GET /api/v1/projects/list-with-access?permission=`
- `GET /api/v1/projects/{id}/check-access?user_id=&permission=`

**Внутренние:**
- `GET /internal/projects/list-with-access`
- `GET /internal/projects/{id}/check-access`

**Композиция прав:** `direct OR subordinate OR delegated`.
**Спрашивает:** User Service (иерархия), Delegation Service (активные делегации).

### NAVIGATION_SERVICE :8009

**Назначение:** плитки рабочего стола.
**Хранилище:** SQLite (`navigation.db`).
**Эндпоинты:**
- `GET /api/v1/navigation/dashboard`
- `POST /api/v1/navigation/tiles`

**Плитки:** по ролям. Плитка «Войти как …» — для всех ролей.

### DELEGATION_SERVICE :8011

**Назначение:** управление делегированиями.
**Типы:** `direct`, `temporary`. Обратное — не хранится.

**Эндпоинты:**
- `POST /api/v1/delegations` — создать.
- `GET /api/v1/delegations/active/me` — активные для текущего как реципиента.
- `GET /api/v1/delegations/active/as-delegator` — активные, где текущий — донор.
- `GET /api/v1/delegations/active/{user_id}` — для указанного (super_admin или self).
- `GET /api/v1/delegations/check?delegate_id=&delegator_id=`
- `GET /api/v1/delegations/` — все (super_admin).
- `POST /api/v1/delegations/{id}/revoke`
- `GET /api/v1/delegations/history/{user_id}`
- `GET/POST/PUT /api/v1/delegation-rules/`

**Подписан на:** Redis DB 3, канал `user.events`.
- `user.deleted`, `user.blocked` → revoke все активные делегации донора.

### NOTIFICATION_SERVICE :8012

**Назначение:** уведомления (in-app, email).
**Эндпоинты:**
- `GET /api/v1/notifications`
- `GET /api/v1/notifications/unread`
- `GET /api/v1/notifications/unread/count`
- `POST /api/v1/notifications/read`
- `POST /api/v1/notifications/read/all`
- `GET /api/v1/notifications/stream` — SSE.
- `POST /api/v1/notifications/internal` — создание из других сервисов.

### UI_COMPOSER_SERVICE :8020

**Назначение:** server-driven UI. Генерация JSON-описаний страниц.

**Компоненты:**
- `TemplateLoader` — читает YAML-шаблоны.
- `WorkflowLoader` — читает workflow YAML, валидирует при старте.
- `SessionService` — Redis DB 1, сессии workflow.
- `DataAggregator` — запросы к бизнес-сервисам.
- `UIBuilder` — рендер `{{placeholders}}`.

**Эндпоинты:**
- `GET /health`
- `GET /pages`
- `GET /page/{path}` — открыть страницу. Workflow или шаблон.
- `POST /workflow/event` — событие от клиента.
- `POST /workflow/back` — назад.
- `POST /workflow/close` — завершить сессию.

**Принцип:** слеп к правам. Агрегирует данные от имени пользователя.
**Пробрасывает:** `X-User-ID`, `X-Impersonated-By` в бизнес-сервисы.

### MASTERGRAPHICS_SERVICE :8003

**Назначение:** хранение мастерграфиков.
**Статус:** в разработке (Спираль 7).

## Клиент (Qt6 QML)

**Окна:**
- `AuthWindow` — логин.
- `MainWindow` — основное окно.
- `DelegatedWindow` — режим имперсонации.
- `ImpersonateWindow` — выбор донора.
- `SettingsWindow`, `NotificationsWindow`.

**Компоненты:**
- `WidgetBridge` — мост QML ↔ C++.
- `JsonUiRenderer` — рендер JSON в QML.
- `QmlObjectFactory` — создание QML-объектов из спецификаций.
- `DataManager` — загрузка данных для виджетов.
- `ApiClient`, `AuthService`, `TokenManager`, `UserProfile` — сетевой слой.

## Стек и версии

| Компонент | Версия |
|---|---|
| Python | 3.12+ |
| FastAPI | 0.115+ |
| SQLAlchemy | 2.0.36+ |
| PostgreSQL | 15+ |
| Redis | 7+ |
| Nginx | 1.24+ |
| Qt | 6.5+ |
| C++ | 20 |
| CMake | 3.16+ |
