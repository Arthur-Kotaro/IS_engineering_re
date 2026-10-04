# ADR-0006: Имперсонация («войти как»)

## Статус
Accepted

## Дата
2026-09-26

## Контекст

Начальник должен иметь возможность действовать от имени подчинённого (больничный, отпуск, командировка). Также — от имени пользователя, явно делегировавшего полномочия.

## Решение

### Плитка «Войти как …»

Показывается в Navigation Service **всегда** (независимо от наличия доноров).
Navigation Service **не знает** о делегированиях — обеспечивается слабая связность.

### Страница `/page/impersonate`

UI Composer формирует два плоских списка:
1. **Временные делегации** — источник: Delegation Service, `GET /api/v1/delegations/active/me`.
2. **Прямые подчинённые** — источник: User Service, `GET /api/v1/users/{me}/subordinates`.

Списки плоские. Удалённые и заблокированные — не показываются. Истёкшие делегации — не показываются.
Если список пуст — надпись «нет …».

### Клик по донору

1. Клиент → `POST /internal/auth/impersonate {donor_id}` через gateway.
2. User Service:
   - Проверяет, что `donor.head_id == recipient.user_id` (обратное), ИЛИ
   - Спрашивает Delegation Service, есть ли активная делегация `delegator_id=donor_id, delegate_id=recipient.user_id`.
3. Если да — выпускает временный токен:
   - `user_id` = donor_id
   - `impersonated_by` = recipient_id
   - `impersonation_id` = uuid
   - `jti`, `type: access`, `exp` = +2 часа
4. Токен + `impersonation_id` → клиент.
5. Клиент открывает `DelegatedWindow` (новое окно, без фокуса).
6. Верхняя панель окна — индикатор режима, имя донора, кнопка выхода.

### DelegatedWindow

- Новое окно, отдельное от MainWindow.
- Все запросы идут с токеном донора.
- Nginx при валидации кладёт `X-User-ID = donor_id`, `X-Impersonated-By = recipient_id`.
- Сервисы читают `X-User-ID` как идентичность донора, `X-Impersonated-By` — для аудита.

### Блокировки

- Рекурсия: если `X-Impersonated-By` уже установлен, повторная имперсонация запрещена.
- Задвоение: нельзя открыть второе `DelegatedWindow` под того же донора.

### Закрытие

- Клиент → `POST /internal/auth/impersonation/close {impersonation_id}` через gateway.
- User Service удаляет ключ `impersonation:{id}` из Redis.
- Токен при следующем запросе инвалидируется через `/verify` (проверка `impersonation_id` в Redis).

### TTL

1–2 часа. Cleanup — Redis TTL.

### Аудит

Каждый сервис, куда приходит запрос с `X-Impersonated-By`, при записи в аудит сохраняет:
- `actor_id` = `X-Impersonated-By`.
- `acting_as_id` = `X-User-ID`.
- `delegation_id` (если применимо).
- `delegation_type` (если применимо).

## Прямые запросы UI Composer

UI Composer пробрасывает `X-User-ID` и `X-Impersonated-By` в сервисы при запросах sources. Сервисы читают эти заголовки для аудита.
