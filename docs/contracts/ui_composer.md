# UI Composer Service — контракты

**Порт:** 8020.

## Публичные (защищённые JWT)

### GET /health

**Ответ 200:** `{"status": "ok", "redis": true}`.

### GET /pages

Список шаблонов.

**Ответ 200:** `{"pages": ["hr/search-user", "hr/search-results", ...]}`.

### GET /page/{path}

**Пример:** `GET /page/hr/search-user`.

**Действия:**
1. Если для `/page/{path}` есть workflow — создаётся сессия, отдаётся первый шаг.
2. Если workflow нет — рендерится шаблон напрямую.

**Ответ 200:**
```json
{
  "title": "string",
  "widgets": [...],
  "context": {
    "session_id": "uuid",
    "current_step": "search",
    "workflow_name": "hr_search_user"
  }
}
```

Если шаблон без workflow — без context.

POST /workflow/event

Body:

```json
{
  "session_id": "uuid",
  "widget_id": "search_btn",
  "event_type": "click",
  "data": {"search_input": "Иван"}
}
```

Ответ 200: то же, что /page/{path} — со следующим шагом.

POST /workflow/back

Body: {"session_id": "uuid"}.

Ответ 200: предыдущий шаг.

POST /workflow/close

Body: {"session_id": "uuid"}.

Ответ 200: {"message": "Session closed"}.

Формат workflow

См. docs/adr/0007-workflow.md.

Формат шаблона

```yaml
name: string
title: string
endpoint: string
description: string | optional
sources:
  - id: string
    service: string
    endpoint: string
    method: GET | POST | PUT | DELETE
    params:
      key: "{{placeholder}}"
ui:
  widgets:
    - type: string
      id: string
      <properties>
      widgets: [вложенные]
```

Типы виджетов

· Контейнеры: Card, QGroupBox, ColumnLayout, RowLayout, QGridLayout.
· Поля: TextField, EmailField, PasswordField, SearchField, SelectField, MultiSelectField.
· Кнопки: Button.
· Отображение: Label, Table, ProgressBar.
· Диаграммы: ChartPie, ChartBar, ChartLine, ChartBarCompare.
· Специализированные: GanttChartView (планируется).

Типы событий

Виджет event_type
Button click
TextField, EmailField, PasswordField input
SearchField submit
SelectField, MultiSelectField change
Table (строка) row_select
ColumnLayout (форма) submit

Sources

Обычный источник

```yaml
sources:
  - id: departments
    service: user_service
    endpoint: /api/v1/hr/departments
    method: GET
```

UI Composer делает запрос к сервису от имени пользователя.
Передаёт X-User-ID, X-Impersonated-By, Authorization.

Источник из сессии

```yaml
sources:
  - id: search_results
    service: session
    endpoint: "session:search_results"
```

Возвращает значение из session.data["search_results"].

Подстановка __me__

В endpoint: /api/v1/users/__me__/subordinates — __me__ заменяется на X-User-ID.

Сессия

Redis DB 1, ключ session:{uuid}, TTL 3600 сек.

Содержимое:

```json
{
  "session_id": "uuid",
  "workflow_name": "hr_search_user",
  "current_step": "search",
  "user_id": 100,
  "data": {"search_input": "Иван", "search_results": [...]},
  "history": ["search"]
}
```

Проброс заголовков

UI Composer передаёт в бизнес-сервисы:

· X-User-ID — из входного запроса.
· X-Impersonated-By — если есть.
· Authorization — если есть (для защищённых эндпоинтов).
· X-Internal-Key — для /internal/* (из переменной окружения INTERNAL_API_KEY).
