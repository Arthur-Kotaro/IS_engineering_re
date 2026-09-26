# ADR-0007: Workflow и UI Composer

## Статус
Proposed

## Дата
2026-09-26

## Решение

Server-driven UI: сервер отдаёт JSON-описание страницы, клиент рендерит.
Workflow — YAML. Шаблон — YAML. Сессия — Redis.

## Формат workflow (пример)

    name: hr_search_user
    initial_endpoint: /page/hr/search-user
    initial_step: search

    steps:
      search:
        type: navigation
        template: hr/search-user
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

## Формат шаблона (пример)

    name: hr_search_user
    title: "Поиск пользователей"
    sources: []
    ui:
      widgets:
        - type: Card
          widgets:
            - type: SearchField
              id: search_input
            - type: Button
              id: search_btn
              text: "Найти"

## Сессия

Redis, ключ session:{id}. Содержит:
- workflow_name
- current_step
- data
- history
- user_id

TTL: 1 час.

## Эндпоинты UI Composer

- GET /page/{path} — начать workflow.
- POST /workflow/event — обработать событие.
- POST /workflow/back — вернуться на предыдущий шаг.
- POST /workflow/close — завершить сессию.

## Типы виджетов

Контейнеры: Card, QGroupBox, ColumnLayout, RowLayout, QGridLayout.
Поля: TextField, EmailField, PasswordField, SearchField, SelectField, MultiSelectField.
Кнопки: Button.
Отображение: Label, Table, ProgressBar.
Диаграммы: ChartPie, ChartBar, ChartLine, ChartBarCompare.
Специализированные: GanttChartView (позже).

## Типы событий

| Виджет | event_type |
|---|---|
| Button | click |
| TextField, EmailField, PasswordField | input |
| SearchField | submit |
| SelectField, MultiSelectField | change |
| Table (строка) | row_select |
| ColumnLayout (форма) | submit |

## Валидация

- Клиентская: validation в шаблоне.
- Серверная: в workflow on_event.validation.

## Row-select

Клик по строке Table → событие row_select → next_step с {{selected_row.*}}.

## Принципы

- UI Composer слеп к правам.
- action в шаблонах убирается. Вместо — on_event в workflow.
- sources и data сессии — два источника данных для шаблона.
