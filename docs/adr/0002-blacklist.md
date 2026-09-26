# ADR-0002: Blacklist отозванных токенов в Redis

## Статус
Accepted

## Дата
2026-09-26

## Контекст

JWT access-токены имеют TTL 15–30 минут. При logout или revoke-сессии токен должен быть немедленно недействителен, пока не истёк его exp.

До этого решения:
- User Service вёл blacklist в Postgres (таблица `token_blacklist`).
- Auth Service вёл blacklist в Redis (ключ `blacklist:{jti}`).
- Два хранилища не синхронизированы.
- Logout в User писал в Postgres, Auth об этом не знал.

## Решение

Единый blacklist — Redis. Один инстанс, системный.

### Формат
- Ключ: blacklist:{jti}
- Значение: 1
- TTL: равен остатку срока жизни токена (exp - now)

### Writer
- User Service при POST /api/v1/auth/logout.
- User Service при revoke_all_user_refresh_tokens.
- User Service при change_password.

### Reader
- Auth Service при POST /verify — EXISTS blacklist:{jti}
- Nginx не читает (кэш ответа /verify)

### Cleanup
- Не нужен. Redis удаляет ключи по TTL.
- Задача cleanup_expired_tokens в User Service — удаляется.
- Таблица token_blacklist в Postgres — удаляется.
- Модель TokenBlacklist — удаляется.
- BlacklistRepository в User — удаляется.

### Redis DB
- DB 0 — blacklist
- DB 1 — сессии workflow (в будущем)
- DB 2 — кэш прав (резерв)

## Последствия

### Положительные
- Единый источник правды.
- TTL — бесплатный cleanup.
- Единый компонент для blacklist и workflow-сессий.
- Отзыв токена работает мгновенно.
- Auth читает blacklist при каждом /verify — но с nginx-кэшем это редко.

### Отрицательные
- Redis — ещё один компонент в системе.
- При падении Redis logout не работает (fail-open или fail-closed).
- В nginx-кэше /verify TTL 60 сек. Отозванный токен ещё 60 сек работает.

## Поведение при отказе Redis

Решение: fail-open для MVP. Логируется warning.

## Ссылки
- ADR-0001: Gateway на nginx + auth_request
