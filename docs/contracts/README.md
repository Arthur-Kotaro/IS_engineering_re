# Контракты сервисов

## Общие правила

- HTTP/1.1, JSON.
- Заголовки — см. ADR-0008 и request-flows.md.
- Аутентификация: `Authorization: Bearer <jwt>`.
- Внутренние эндпоинты: `X-Internal-Key`.
- Идентичность: `X-User-ID`, `X-User-Role` (ставит nginx).
- Имперсонация: `X-Impersonated-By`, `X-Impersonation-Id`.

## HTTP-коды

- 200 — успех.
- 201 — создано.
- 204 — удалено (без тела).
- 400 — неверные данные.
- 401 — не авторизован.
- 403 — нет прав.
- 404 — не найдено.
- 409 — конфликт.
- 500 — внутренняя ошибка.

## Формат ошибки

```json
{
  "detail": "string",
  "error_type": "string | null",
  "context": {}
}
```

Документы по сервисам

· auth.md — Auth Service.
· user.md — User Service.
· project.md — Project Service.
· delegation.md — Delegation Service.
· notification.md — Notification Service.
· ui_composer.md — UI Composer Service.
