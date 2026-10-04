# Delegation Service — контракты

**Порт:** 8011.

## Типы делегирования

| Тип | initiator_id | delegator_id | delegate_id | main_delegate_id |
|---|---|---|---|---|
| `direct` | = delegator_id (начальник) | начальник | подчинённый | null |
| `temporary` | начальник | подчинённый А | подчинённый Б | = delegator_id (А) |

Обратное делегирование **не хранится**. Реализуется через `head_id` в User Service.

## Публичные эндпоинты

### POST /api/v1/delegations

**Body:**
```json
{
  "delegator_id": int,
  "delegate_id": int,
  "delegation_type": "direct | temporary",
  "main_delegate_id": int | null,
  "starts_at": "datetime",
  "expires_at": "datetime",
  "reason": "string | null"
}
```

Правила:

· direct: delegator_id = initiator_id = текущий пользователь; delegate_id — прямой подчинённый.
· temporary: main_delegate_id обязателен; оба подчинённых — прямые у инициатора.
· Один реципиент на одно делегирование.
· Рекурсия запрещена.

Ответ 200: DelegationResponse.

GET /api/v1/delegations/active/me

Активные делегации, где текущий пользователь — реципиент.

Ответ 200: List[DelegationResponse].

GET /api/v1/delegations/active/as-delegator

Активные делегации, где текущий — донор.

GET /api/v1/delegations/active/{user_id}

Для указанного user_id. Только self или super_admin.

GET /api/v1/delegations/check?delegate_id=&delegator_id=

Ответ 200:

```json
{
  "has_delegation": true,
  "delegation_id": int,
  "delegator_id": int,
  "delegation_type": "direct | temporary",
  "expires_at": "datetime"
}
```

GET /api/v1/delegations/

Все делегации. Только super_admin.

Ответ 200: DelegationListResponse.

POST /api/v1/delegations/{id}/revoke

Body: {"reason": "string | null"}.

Права: initiator_id или super_admin.

Ответ 200: DelegationResponse.

GET /api/v1/delegations/history/{user_id}

Только self или super_admin.

Эндпоинты правил

GET /api/v1/delegation-rules/

Все правила. Только super_admin.

GET /api/v1/delegation-rules/{role}

Правило для роли. Только super_admin.

POST /api/v1/delegation-rules/

Body: DelegationRuleCreate. Только super_admin.

PUT /api/v1/delegation-rules/{role}

Body: DelegationRuleCreate. Только super_admin.

События

Подписан на Redis DB 3, канал user.events:

· user.deleted → revoke все активные делегации донора.
· user.blocked → revoke все активные делегации донора.

Схемы

DelegationResponse

```json
{
  "delegation_id": int,
  "initiator_id": int,
  "delegator_id": int,
  "delegator_name": "string | null",
  "delegate_id": int,
  "delegate_name": "string | null",
  "main_delegate_id": "int | null",
  "delegation_type": "direct | temporary",
  "starts_at": "datetime",
  "expires_at": "datetime",
  "status": "active | expired | revoked | pending",
  "reason": "string | null",
  "created_by": int,
  "created_at": "datetime",
  "updated_at": "datetime | null",
  "revoked_at": "datetime | null",
  "revoked_by": "int | null",
  "revoke_reason": "string | null"
}
```

