# Notification Service — контракты

**Порт:** 8012.

## Публичные (защищённые JWT)

### GET /api/v1/notifications

**Query:** `limit`, `offset`, `status_filter` (unread).

**Ответ 200:** `NotificationListResponse`.

### GET /api/v1/notifications/unread

**Ответ 200:** `List[NotificationResponse]`.

### GET /api/v1/notifications/unread/count

**Ответ 200:** `{"unread_count": int}`.

### POST /api/v1/notifications/read

**Body:** `{"notification_ids": [int]}`.

### POST /api/v1/notifications/read/all

### GET /api/v1/notifications/stream

SSE-поток. Требует JWT. `Content-Type: text/event-stream`.

## Internal

### POST /api/v1/notifications/internal

**Body:**
```json
{
  "user_id": int,
  "user_email": "string | null",
  "user_name": "string | null",
  "notification_type": "string",
  "title": "string",
  "message": "string",
  "reference_id": "int | null",
  "reference_type": "string | null",
  "data": {},
  "send_email": false
}
```

Заголовки: опционально X-Internal-Key.

Ответ 200: NotificationResponse.

Типы уведомлений

· login_new_device, password_changed, password_expiry_warning, password_expired
· account_blocked, account_unblocked, login_failed
· delegation_created, delegation_revoked, delegation_expired, delegation_temporary
· project_assigned, project_unassigned
· system

Схемы

NotificationResponse

```json
{
  "notification_id": int,
  "user_id": int,
  "user_email": "string | null",
  "user_name": "string | null",
  "notification_type": "string",
  "status": "unread | read",
  "title": "string",
  "message": "string",
  "reference_id": "int | null",
  "reference_type": "string | null",
  "data": {},
  "created_at": "datetime",
  "read_at": "datetime | null"
}
```

