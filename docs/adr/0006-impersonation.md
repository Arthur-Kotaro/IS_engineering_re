# ADR-0006: Имперсонация («войти как»)

## Статус
Proposed

## Дата
2026-09-26

## Решение

Плитка «Войти как …» — всегда видна в Navigation.

Страница /page/impersonate — два плоских списка:
- Верхний: временные делегации (Delegation Service).
- Нижний: прямые подчинённые (User Service).
- Если список пуст — надпись.

Клик по донору:
1. Клиент → POST /internal/auth/impersonate → User Service.
2. User Service проверяет: подчинённый или активная делегация.
3. User Service выпускает временный токен:
   - user_id: donor_id
   - impersonated_by: recipient_id
   - impersonation_id: uuid
   - exp: +1–2 часа
4. Клиент открывает DelegatedWindow — новое окно без фокуса.
5. Верхняя панель окна — индикатор режима, имя донора, кнопка выхода.

## Блокировки

- Рекурсия: нельзя войти как тот, кто сам сейчас действует как другой.
- Задвоение: нельзя открыть второе окно под того же донора.

## Заголовки

- X-User-ID = donor_id
- X-Impersonated-By = recipient_id

## Аудит

- acting_user_id = recipient_id
- acting_as_id = donor_id
- delegation_id (если применимо)
- delegation_type

## TTL токена имперсонации

1–2 часа. Cleanup — Redis TTL.

## Закрытие окна

- Клиент → POST /internal/auth/impersonation/close → User Service.
- Токен добавляется в blacklist Redis.
