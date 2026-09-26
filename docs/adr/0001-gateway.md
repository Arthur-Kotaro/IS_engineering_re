# ADR-0001: API Gateway на nginx + Auth Service как verifier

## Статус
Accepted

## Дата
2026-09-26

## Контекст

ERP «Engineering:re» состоит из 8 микросервисов и Qt-клиента. Клиент обращается к системе через единую точку входа. До 500 активных пользователей. Локальная сеть. Прод — HTTPS позже.

Требования:
- Единая точка входа для клиента.
- Валидация JWT для защищённых эндпоинтов.
- Blacklist отозванных токенов.
- Имперсонация («войти как»).
- Streaming файлов (в будущем).
- Прозрачная карта маршрутов.
- Поддержка rate-limit, TLS, CORS при необходимости.

Ранее использовались:
- Системный nginx + Auth Service с проксированием.
- Мёртвый репозиторный API_Gateway на Docker.

Проблемы прежней схемы:
- Двойное проксирование: nginx → Auth → сервис.
- Streaming ломался (httpx читал тело в память).
- Две точки маршрутизации (nginx + Auth).
- Auth — точка отказа.
- Имперсонацию сложно вписать.

## Решение

Единый gateway на системном nginx с `auth_request`.

    Client → Nginx:8080 ─auth_request→ Auth:8010 (POST /verify)
                        │
                        └──────────────→ сервисы напрямую

Компоненты:
- Nginx — маршрутизация, терминация, auth_request, кэш, streaming, TLS/rate-limit позже.
- Auth Service — только `/verify`: валидация JWT, проверка blacklist в Redis, возврат заголовков.
- Сервисы — принимают `X-User-ID`, `X-User-Role`, `X-Impersonated-By` от nginx. Доверяют.
- UI Composer — ходит к сервисам напрямую, минуя nginx, для sources шаблонов.

## Ключевые решения

### 1. Auth Service упрощается до /verify
Auth не проксирует. Один эндпоинт принимает `Authorization`, возвращает 200 + заголовки или 401.
Удаляются: SERVICE_ROUTES, PAGE_ROUTES, проксирование, httpx, config.py.

### 2. Nginx — единственная точка маршрутизации
Все маршруты клиента описаны в `nginx.conf`.
`nginx -T` показывает полную карту.
Новый сервис = новый `location`.

### 3. Кэш токенов в nginx — сразу
Nginx кэширует ответ `/verify` по ключу `$http_authorization`.
TTL: 200 → 60s, 401 → 10s.
`proxy_cache_use_stale error timeout` — если Auth упал, отдаётся закэшированный ответ.
Обоснование: под нагрузкой до 500 пользователей Auth не должен дёргаться на каждый запрос.

### 4. Публичные эндпоинты — список в nginx
Без `auth_request`:
- /api/v1/auth/login
- /api/v1/auth/refresh
- /api/v1/auth/reset-password
- /api/v1/auth/register
Идут напрямую в User Service.

### 5. /internal/* — закрыт по IP и ключу
allow 127.0.0.1; allow 10.0.0.0/8; deny all;
Плюс X-Internal-Key в заголовке.

### 6. UI Composer ходит в сервисы напрямую
UI Composer, обрабатывая `sources` шаблона, делает запросы к User/Project/Navigation/Delegation минуя nginx.
Обоснование:
- Не нужен повторный auth_request (UI Composer уже прошёл проверку).
- Не нужен лишний хоп через nginx.
- UI Composer передаёт `X-User-ID` и `X-Impersonated-By` из входного запроса.

### 7. X-Impersonated-By пробрасывается
При имперсонации nginx получает `X-Impersonated-By` из Auth и подставляет в основной запрос.
UI Composer пробрасывает его в сервисы.
Сервисы читают его опционально для аудита. Не используют для прав.

### 8. Заголовки
- X-User-ID — идентичность.
- X-User-Role — роли через запятую, либо super_admin.
- X-Impersonated-By — реальный действующий пользователь (при имперсонации).
- Authorization — оригинальный JWT, пробрасывается в сервисы.

### 9. Streaming файлов
В `location` для файловых эндпоинтов:
    proxy_buffering off;
    proxy_request_buffering off;
    client_max_body_size 500M;

### 10. Отказ от репозиторного API_Gateway
API_Gateway/ (Docker, nginx/, docker-compose) — мёртвый. Удаляется.
Системный nginx управляется через /etc/nginx/.
Конфиг версионируется в репозитории как `infra/nginx/gateway.conf` + `deploy.sh`.

## Последствия

### Положительные
- Один язык для Auth (Python), nginx — стандартная инфраструктура.
- Прозрачная карта маршрутов.
- Streaming работает.
- Rate-limit, TLS, CORS — бесплатно при необходимости.
- Имперсонация вписана в один слой.
- Auth проще в 10 раз.
- UI Composer и клиент не меняются.

### Отрицательные
- Subrequest к Auth на каждый уникальный токен (с кэшем — реже).
- Дублирование `auth_request_set` в каждом location (лечится include).
- Отладка auth_request сложнее прямого кода.
- Окно 60 сек, в котором отозванный токен ещё работает.
- Два процесса: nginx + Auth.

## Ссылки
- ADR-0002: Blacklist в Redis
- docs/architecture/request-flows.md
