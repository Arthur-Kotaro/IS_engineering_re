# ADR-0007: Workflow и UI Composer

## Статус
Accepted

## Дата
2026-09-26

## Контекст

Server-driven UI: сервер отдаёт JSON-описание страницы, клиент рендерит через генераторы QML. Workflow описывает шаги и переходы между ними.

## Решение

### Компоненты

- **Workflow YAML** — `workflows/*.yaml`. Описывает шаги, события, переходы.
- **Шаблон YAML** — `templates/*.yaml`. Описывает UI страницы.
- **Сессия** — Redis DB 1. Ключ `session:{uuid}`, TTL 1 час.
- **UI Composer** — движок, оркестратор.

### Сессия

- Создаётся при первом `GET /page/{endpoint}`, если для endpoint есть workflow.
- Хранит: `session_id`, `workflow_name`, `current_step`, `user_id`, `data`, `history`.
- `data` — накопленные значения (результаты поиска, выбранные строки, введённые поля).
- `history` — стек предыдущих шагов (для `/workflow/back`).

### Эндпоинты

- `GET /page/{path}` — открыть страницу. Если для path есть workflow — создаётся сессия, отдаётся первый шаг. Если нет — рендер шаблона без workflow.
- `POST /workflow/event {session_id, widget_id, event_type, data}` — обработать событие.
- `POST /workflow/back {session_id}` — вернуться на предыдущий шаг.
- `POST /workflow/close {session_id}` — завершить сессию.

### Формат workflow

yaml
name: hr_search_user
initial_endpoint: /page/hr/search-user
initial_step: search

steps:
  search:
    type: navigation
    template: hr/search-user
    title: "Поиск пользователей"
    on_event:
      - widget: search_btn
        event_type: click
        service:
          name: user_service
          endpoint: /api/v1/hr/users/search
          method: GET
          params:
            query: "{{search_input}}"
        on_success:
          next_step: results
          data:
            search_results: "{{result}}"

  results:
    type: navigation
    template: hr/search-results
    on_event:
      - widget: back_btn
        event_type: click
        on_success:
          next_step: search

Формат шаблона

yaml
name: hr_search_user
title: "Поиск пользователей"
endpoint: /page/hr/search-user

sources:
  - id: departments
    service: user_service
    endpoint: /api/v1/hr/departments
    method: GET

ui:
  widgets:
    - type: Card
      title: "Поиск"
      widgets:
        - type: SearchField
          id: search_input
        - type: Button
          id: search_btn
          text: "Найти"

Sources

Два источника данных для шаблона:

· sources — запросы к сервисам при рендеринге. Возвращают данные.
· session.data — данные, накопленные в workflow. Доступны через {{key}}.
· session:key — специальный формат endpoint: "session:key" для ссылки на данные сессии.

Рендер

UI Composer:

1. Читает шаблон.
2. Резолвит sources (запросы к бизнес-сервисам).
3. Резолвит {{...}} из data + результатов sources.
4. Отдаёт JSON: {title, widgets, context}.

context содержит session_id, current_step, workflow_name.

События

Виджет event_type
Button click
TextField, EmailField, PasswordField input
SearchField submit
SelectField, MultiSelectField change
Table (строка) row_select
ColumnLayout (форма) submit

Валидация

· Клиентская — в шаблоне: validation: {min_length, max_length, error_message}.
· Серверная — в workflow on_event.validation.

Row-select

Клик по строке Table → событие row_select. В data события передаётся selected_row — объект строки. В workflow: next_step с сохранением {{selected_row.project_id}} в session.data.

Принципы

· UI Composer слеп к правам. Просто агрегирует данные от имени пользователя.
· action в шаблонах отсутствует. Вся логика — в on_event workflow.
· UI Composer пробрасывает X-User-ID и X-Impersonated-By в сервисы при запросах sources.

Ограничения текущей реализации

· Валидация на сервере — заглушка (принимается, но не проверяется).
· row_select в Table — генератор клиента не отправляет событие. Требуется доработка TableGenerators.cpp.
· _get_rule_for (делегации) и _check_access (проекты) — не связаны с workflow.
