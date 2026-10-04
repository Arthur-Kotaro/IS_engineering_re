# Auth Service — контракты

**Порт:** 8010.
**Роль:** валидатор JWT.

## GET /health

**Ответ 200:**
```json
{"status": "healthy", "redis": true}
```

ALL /verify

Принимает тот же HTTP-метод, что и основной запрос (через auth_request).

Заголовки:

· Authorization: Bearer <jwt> — обязателен.

Ответ 200:

· Заголовки:
  · X-User-ID: <int>
  · X-User-Role: <comma-separated | super_admin>
  · X-Impersonated-By: <int> (опционально)
  · X-Impersonation-Id: <uuid> (опционально)

Ответ 401:

```json
{"detail": "Missing or invalid Authorization header"}
{"detail": "Token expired"}
{"detail": "Invalid token"}
{"detail": "Token revoked"}
{"detail": "Impersonation session closed"}
```

Проверки в /verify

1. Заголовок Authorization присутствует и начинается с Bearer .
2. JWT валиден (подпись, exp).
3. Если есть jti — нет в Redis DB 0 blacklist:{jti}.
4. Если есть impersonation_id — есть в Redis DB 0 impersonation:{id}.
5. Поле user_id присутствует в payload.

Поведение при отказе Redis

Fail-open. Если Redis недоступен — логируется warning, запрос пропускается. Осознанный компромисс (см. ADR-0002).
