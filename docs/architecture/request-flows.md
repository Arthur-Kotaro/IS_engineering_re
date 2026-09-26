# Карта потоков запросов

## 1. Логин

    1. Client → POST /api/v1/auth/login
    2. Nginx → User:8000/api/v1/auth/login (публичный, без auth_request)
    3. User: проверяет пароль, выпускает JWT, пишет refresh в Postgres
    4. User → Nginx → Client

Хопов: 4. К Auth: 0.

## 2. Авторизованный запрос (плитки)

    1. Client → GET /api/v1/navigation/dashboard
    2. Nginx: auth_request /auth-check
    3. Nginx → Auth:8010/verify
    4. Auth: валидирует JWT, проверяет blacklist в Redis
    5. Auth → Nginx: 200 + X-User-ID, X-User-Role, X-Impersonated-By
       (или из кэша nginx, если токен уже проверялся <60 сек назад)
    6. Nginx: подставляет заголовки, проксирует в сервис
    7. Nginx → Navigation:8009/api/v1/navigation/dashboard
    8. Navigation: читает X-User-ID, читает плитки из SQLite
    9. Navigation → Nginx → Client

Хопов: 6. К Auth: 2 (subrequest), из них 0 при попадании в кэш.

## 3. Открытие workflow-страницы

    1. Client → GET /page/hr/search-user
    2. Nginx: auth_request → Auth → Nginx (или из кэша)
    3. Nginx → UI Composer:8020/page/hr/search-user
    4. UI Composer:
       - находит workflow
       - создаёт сессию в Redis
       - загружает шаблон
       - обрабатывает sources
    5. UI Composer → User:8000/api/v1/hr/departments
    6. User → UI Composer
    7. UI Composer: рендерит, отдаёт JSON
    8. UI Composer → Nginx → Client

Хопов: 8 + 2 × (количество sources). К Auth: 2.

## 4. Workflow-событие

    1. Client → POST /workflow/event {session_id, widget_id, event_type, data}
    2. Nginx: auth_request → Auth → Nginx (или из кэша)
    3. Nginx → UI Composer:8020/workflow/event
    4. UI Composer: находит сессию, шаг, событие; выполняет action
    5. UI Composer → User:8000/api/v1/hr/users/search?query=...
    6. User → UI Composer
    7. UI Composer: сохраняет результат, переходит на next_step
    8. UI Composer → Nginx → Client

Хопов: 7 + 2 × M.

## 5. Имперсонация (планируется)

    1. Client → GET /page/impersonate
    2. Nginx → UI Composer → два запроса:
       - GET /internal/delegations/for-user/{user_id} → Delegation
       - GET /internal/users/{user_id}/subordinates → User
    3. UI Composer отдаёт страницу с двумя списками
    4. Client: пользователь кликает по донору
    5. Client → POST /internal/auth/impersonate → User
       {recipient_id, donor_id}
    6. User: проверяет, выпускает токен с impersonated_by
    7. Client: сохраняет токен, открывает DelegatedWindow
    8. Дальнейшие запросы из DelegatedWindow идут с токеном донора
    9. Nginx валидирует, кладёт X-User-ID=donor, X-Impersonated-By=recipient
    10. Сервисы пишут в аудит

## Роль заголовков

| Заголовок | Кто ставит | Что значит |
|---|---|---|
| Authorization | Client | JWT. |
| X-User-ID | Nginx | Кто в системе (по токену). |
| X-User-Role | Nginx | Роли через запятую, либо super_admin. |
| X-Impersonated-By | Nginx | Кто действует на самом деле. |
| X-Internal-Key | Сервис | Для /internal/*. |

## Прямые запросы UI Composer

UI Composer не ходит через nginx для sources. Идёт напрямую в сервисы.

Причина:
- UI Composer уже прошёл auth_request.
- Не нужен повторный subrequest.
- Не нужен лишний хоп.

Что передаёт:
- X-User-ID из входного запроса.
- X-Impersonated-By из входного запроса.
- Authorization (опционально).
