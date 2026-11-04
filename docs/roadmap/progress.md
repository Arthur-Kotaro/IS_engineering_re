# Прогресс: фазы и спирали

> Живой документ. Даты появляются по мере работы.

## Проекция «Фазы × Спирали»

| Фаза | Спирали | Статус фазы | Дата начала | Дата завершения |
|------|---------|-------------|-------------|-----------------|
| Фаза 0 | 1, 2, 3, 4, 5, 6, 6.5, 6.6, 9 | В работе (6.6, 7) | 2026-09-26 | — |
| Фаза 1 | 7, 8, 10 | Начата | 2026-10-25 | — |
| Фаза 2 | — | Не начата | — | — |

## Хронология

| Дата | Событие | Спираль | Фаза |
|------|---------|---------|------|
| 2026-09-26 | Завершена спираль 1. Gateway + Blacklist. Тег `spiral-1-done` | 1 | 0 |
| 2026-09-26 | Завершена спираль 2. Синхронизация слоёв — User, Auth. Тег `spiral-2-done` | 2 | 0 |
| 2026-09-26 | Завершена спираль 3. Delegation + Project. Отменена в Спирали 6.6 | 3 | 0 |
| 2026-09-26 | Завершена спираль 4. Имперсонация. Тег `spiral-4-done`. Требует доработки после 6.6 | 4 | 0 |
| 2026-09-26 | Завершена спираль 5. Workflow (UI Composer engine, HR-сценарий). Тег `spiral-5-done` | 5 | 0 |
| 2026-09-26 | Завершена спираль 6. Документация | 6 | 0 |
| 2026-10-04 | Завершена спираль 6.5. Секреты + env. Тег `spiral-6.5-done` | 6.5 | 0 |
| 2026-10-12 | Спираль 6.6: клиент — BaseWidgets 1.0, кнопка «Назад», скроллбар вкладки, ограничение ширины полей. | 6.6 | 0 |
| 2026-10-25 | **Спираль 7 (MG Service + GDERE) начата.** Создан каркас `MG_service` (БД, API, права). Начата интеграция GDERE. | 7 | 1 |

## Детали спиралей

### Спираль 6.6. Roles & Positions & Departments (завершена)
- **Клиент (порция 2026-10-12):**
  - Компонентный подход к рендерингу — библиотека `BaseWidgets 1.0`.
  - Семантический слой цветов `action*` в `Colors.qml` (ADR 0012).
  - Кнопка «Назад» в `TabContent.qml` с `context.can_go_back` (ADR 0011).
  - Скроллбар вкладки, ограничение ширины полей (ADR 0013).
- **Архитектурные решения:**
  - `Styles` и `Utils` стали полноценными QML-модулями.
  - PRE (Page Rendering Engine) выделен как отдельный движок.
  - Определены термины BDERE (Block Diagram Editor Rendering Engine) и GDERE (Gantt Diagram Editor Rendering Engine).
- **Реализовано:**
  - **MG Service (каркас):** БД, API, права через `project_service`.
  - **PRE:** виджет `ProjectList` (кастомный генератор), интеграция с `WidgetBridge`.
  - **Клиент:** `GanttEditorWindow` (каркас), открытие по сигналу `ganttEditorRequested`.

### Спираль 7. MG Service (мастерграфики) + GDERE (в работе)
**Цель:** Реализовать сервис МГ и интегрировать полноценный редактор диаграмм Ганта в ERP.

- **MG Service (бэкенд):**
  - [x] Каркас `MG_service` (порт 8003, БД PostgreSQL).
  - [x] Таблицы: `mastergraphs`, `task_groups`, `mg_tasks`, `mg_dependencies`.
  - [x] Проверка прав через `project_service`.
  - [x] Тестовые данные (6 групп, 1 МГ, 3 задачи, 2 зависимости).
  - [x] API: `GET /api/v1/mastergraphs`, `GET /api/v1/mastergraphs/{mg_id}`, `GET /api/v1/mastergraphs/{mg_id}/access`.
  - [ ] API: `POST /api/v1/mastergraphs` (создание).
  - [ ] API: `PUT /api/v1/mastergraphs/{mg_id}` (diff).
  - [ ] WebSocket: `/ws/mastergraphs/{mg_id}`.

- **GDERE (Клиент):**
  - [x] Создана независимая структура `gantt_editor/` (include, src, qml).
  - [x] Перенесены модели (`TaskModel`, `GroupModel`, `MilestoneModel`, `DependencyModel`, `HistoryEntry`) в `namespace GanttEditor`.
  - [x] Создан `ProjectData` (сетевой, без файлов).
  - [x] Создан `GanttApiClient` (REST).
  - [x] Создан `ProjectController` (заготовка для сети).
  - [x] Перенесены все QML-файлы (GridArea, TaskRow, AppToolBar и т.д.).
  - [x] `GanttEditorWindow.qml` адаптирован (убраны файловые диалоги).
  - [x] **Сборка клиента:** ошибок нет.
  - [ ] **Исправление путей QML:** Устранить ошибку `"../components": no such directory` в `GanttEditorWindow.qml` (заменить относительные импорты на прямые).
  - [ ] **Реализация `ProjectController::loadFromServer`:** реальный запрос к MG Service, парсинг JSON, заполнение `ProjectData`.
  - [ ] **Интеграция `WidgetBridge` → `GanttEditorWindow`:** передача `projectId`, `mgId`, `token` из UI Composer.
  - [ ] **WebSocket-клиент** для real-time.

### Спираль 8. Макет MG → ERP
Откладывается (зависит от Спирали 7).

### Спираль 9. Тесты
Откладывается.

### Спираль 10. Прод-готовность
Откладывается.

## Правило спирали

**Спираль 6.6 — фундаментальная переработка.** После неё:
- User Service: роли + должности + подразделения.
- Project Service: справочник проектных ролей.
- Navigation Service: 3 таблицы плиток.
- Auth Service: `X-User-Position` + `X-User-Role`.
- Client: обновить под новую модель.

**Только после этого — MG Service.** (Спираль 7 начата, так как 6.6 стабилизирована для MVP).

## Возможное будущее

**Фаза 3** — реализация бизнес-логики конкретного предприятия (например, ВАЗ/УПП) поверх платформы.
