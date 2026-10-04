# Карта потоков запросов

## Обозначения

- **Hop** — сетевой переход между процессами.
- **Subrequest** — внутренний подзапрос nginx к Auth для валидации JWT.
- **Кэш** — nginx `proxy_cache` для ответа `/verify`. TTL 60s (200), 10s (401).

## 1. Логин

```

1. Client → POST /api/v1/auth/login
2. Nginx → User:8000/api/v1/auth/login (публичный, без auth_request)
3. User: валидирует пароль, выпускает JWT, пишет refresh в Postgres
4. User → Nginx → Client

```

**Хопов:** 4. **К Auth:** 0.

## 2. Авторизованный запрос (плитки)

```

1. Client → GET /api/v1/navigation/dashboard (Authorization: Bearer ...)
2. Nginx: auth_request /auth-check
3. Nginx → Auth:8010/verify
4. Auth: валидирует JWT, проверяет blacklist в Redis
5. Auth → Nginx: 200 + X-User-ID, X-User-Role
   (или из кэша nginx — 0 хопов, если токен проверялся < 60 сек)
6. Nginx подставляет заголовки, проксирует в сервис
7. Nginx → Navigation:8009/api/v1/navigation/dashboard
8. Navigation: читает X-User-ID, читает плитки из SQLite
9. Navigation → Nginx → Client

```

**Хопов:** 6. **К Auth:** 2 (или 0 из кэша).

## 3. Открытие workflow-страницы

```

1. Client → GET /page/hr/search-user
2. Nginx: auth_request → Auth → Nginx
3. Nginx → UI Composer:8020/page/hr/search-user
4. UI Composer:
   4.1. Ищет workflow по initial_endpoint=/page/hr/search-user
   4.2. Создаёт сессию в Redis DB 1: session:{uuid}
   4.3. Загружает шаблон hr/search-user
   4.4. Обрабатывает sources
5. UI Composer → User:8000/api/v1/hr/users/search (напрямую)
6. User → UI Composer
7. UI Composer: рендерит, отдаёт JSON {title, widgets, context}
8. UI Composer → Nginx → Client

```

**Хопов:** 8 + 2 × N (N = количество sources). **К Auth:** 2.

**Важно:** UI Composer ходит в сервисы **напрямую**, минуя nginx. Не нужен повторный auth_request. Передаёт `X-User-ID` и `X-Impersonated-By` из входного запроса.

## 4. Workflow-событие

```

1. Client → POST /workflow/event {session_id, widget_id, event_type, data}
2. Nginx: auth_request → Auth → Nginx
3. Nginx → UI Composer:8020/workflow/event
4. UI Composer:
   4.1. Находит сессию в Redis
   4.2. Находит шаг, событие
   4.3. Выполняет action: запрос к сервису
5. UI Composer → User:8000/api/v1/hr/users/search?query=...
6. User → UI Composer
7. UI Composer: сохраняет результат в session.data, переходит на next_step
8. UI Composer: загружает следующий шаблон, рендерит
9. UI Composer → Nginx → Client

```

**Хопов:** 7 + 2 × M.

## 5. Имперсонация

### Открытие страницы доноров

```

1. Client → GET /page/impersonate
2. Nginx → UI Composer
3. UI Composer → Delegation:8011/api/v1/delegations/active/me (напрямую)
4. UI Composer → User:8000/api/v1/users/{me}/subordinates (напрямую)
5. UI Composer → Nginx → Client

```

### Клик по донору

```

1. Client → POST /internal/auth/impersonate {donor_id}
2. Nginx → User:8000 (проксирует /internal/)
3. User:
   3.1. Проверяет head_id донора = текущий user_id (обратное)
   ИЛИ
   3.2. Спрашивает Delegation, есть ли активная делегация
4. User выпускает токен: {user_id: donor_id, impersonated_by: recipient_id, impersonation_id}
5. User пишет в Redis: impersonation:{id} = recipient_id
6. User → Nginx → Client
7. Client открывает DelegatedWindow

```

### Запросы из DelegatedWindow

```

1. Client (DelegatedWindow) → GET /api/v1/navigation/dashboard с токеном донора
2. Nginx: auth_request → Auth
3. Auth: валидирует JWT, проверяет blacklist и impersonation_id
4. Auth → Nginx: X-User-ID=donor, X-Impersonated-By=recipient, X-Impersonation-Id
5. Nginx → Navigation
6. Navigation: читает X-User-ID как донора

```

### Закрытие

```

1. Client → POST /internal/auth/impersonation/close {impersonation_id}
2. Nginx → User
3. User удаляет Redis-ключ impersonation:{id}
4. Следующий запрос с этим токеном → Auth видит отсутствие ключа → 401

```

## 6. Роль заголовков

| Заголовок | Кто ставит | Значение |
|---|---|---|
| `Authorization` | Client | JWT. |
| `X-User-ID` | Nginx | Идентификатор пользователя (при имперсонации — донора). |
| `X-User-Role` | Nginx | Роли через запятую или `super_admin`. |
| `X-Impersonated-By` | Nginx | Реальный действующий пользователь (recipient). |
| `X-Impersonation-Id` | Nginx | ID сессии имперсонации. |
| `X-Internal-Key` | Service | Для `/internal/*`. |

## 7. Особенности

### Кэш `/verify`

Nginx кэширует ответ `/verify` по ключу `$http_authorization`.
TTL: 200 → 60s, 401 → 10s.

**Следствие:** отозванный токен работает до 60 секунд после logout. **Осознанный компромисс** для производительности.

### UI Composer ходит напрямую

UI Composer не использует nginx для sources. Прямые запросы к сервисам.
**Причина:** UI Composer уже прошёл auth_request. Лишний хоп не нужен.

### Streaming

Для файловых эндпоинтов в nginx: `proxy_buffering off`, `proxy_request_buffering off`.
Для SSE (`/api/v1/notifications/stream`): `proxy_buffering off`, `proxy_cache off`, `proxy_read_timeout 24h`.

### Событийная шина

User Service публикует события в Redis DB 3, канал `user.events`.
Delegation Service подписан. Получает `user.deleted`, `user.blocked` → revoke все делегации донора.
