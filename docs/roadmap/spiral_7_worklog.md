# Спираль 7. Макет MG → ERP. Worklog

**Дата начала:** 2026-11-14
**Дата обновления:** 2026-11-27
**Статус:** В работе. Сессия 2 (ЛГ/МГ + IPC refresh) — собрано, не проверено в рантайме.
**Связанные документы:** `phase_1_plan.md`, `spirals.md`, `progress.md`

## 1. Цель спирали

Завершить интеграцию редактора диаграмм Ганта (GDERE) с ERP. Обеспечить безупречную работу МГ и ЛГ: авторизация WS, синхронизация snapshot/diff, outbox для надёжной доставки событий, рефакторинг клиента.

## 2. Архитектурные решения (приняты)

| № | Решение | Обоснование |
|---|---|---|
| 1 | **WS через API Gateway.** `auth_request` в `location /ws/`. | Единая точка входа. |
| 2 | **Активность (Вариант B).** `QLocalSocket`/`QLocalServer`. | Ответственность клиента. |
| 3 | **Координатор активности.** Первый `listen()`. Перехват при аварии. | Надёжность. |
| 4 | **Activity в `CLIENT/common/`.** | Переиспользование. |
| 5 | **`coordinator_shutdown`** → дочерние продолжают, перехватывают. | Работа без MainWindow. |
| 6 | **timeout (30 мин)** → logout, AuthWindow. | Предсказуемость. |
| 7 | **Outbox:** таблица в БД каждого сервиса. Удаляется после ack. | Транзакционность. |
| 8 | **Ack (C):** получатель подтверждает. | Надёжность. |
| 9 | **Event Log (C, гибрид):** `mg_events`/`lg_events` + централизованный `event_log`. | AI Manager. |
| 10 | **N+1:** batch, `LEFT JOIN`, `GROUP BY`. | Производительность. |
| 11 | **WS-реконнект:** backoff 1→2→4→8→30s. `last_version` в URL. Индикатор в окне. | Надёжность. |
| 12 | **`onWsSnapshot` (C, гибрид):** пусто → `fromJson`, иначе — обновить. | UI. |
| 13 | **`onWsDiff`:** task, milestone, dependency, group. | Интерактивность. |
| 14 | **Токен:** env + удаление файла. `refreshToken` → обновление сессии. `changePassword` → очистка. | Безопасность. |
| 15 | **Заголовок вкладки:** две `MouseArea` (фокус/закрытие). | Надёжность. |
| 16 | **`MainWindowBridge` — источник `unreadNotifications`.** | Слабая связность. |
| 17 | **QML из qrc (Вариант A).** | Согласованность. |
| 18 | **`ImpersonateWindow` → вкладка** (Спираль 8). | Меньше кода. |
| 19 | **`DelegatedWindow`** — Спираль 8. | Контекст. |
| 20 | **CI/CD (Спираль 12).** Персональный `.env`. Мобильный → Спираль 13. | Инфраструктура. |
| 21 | **TimelineController** — единый источник `displayStart`/`displayEnd`/`dayWidth` для `CalendarHeader`, `GridArea`, `MilestoneBar`. | 100% синхронизация. |
| 22 | **Целевой период проекта** (`target_start_date`/`target_end_date`) в Project/MG. `displayStart/End` не могут быть меньше. | Предсказуемость. |
| 23 | **Zoom level «Месяц»** (3px, без дней). | Обзор хронологии. |
| 24 | **Wizard инициализации МГ** (авто при пустом МГ, CVE/IPP). | Ускорение. |
| 25 | **Согласование МГ:** кнопка «Черновик/Чистовик» → workflow (см. раздел 13). | Управляемость. |
| 26 | **Единый паттерн C для ЛГ:** клиент шлёт команду → сервер применяет → WS snapshot всем → клиент перерисовывает. Никаких optimistic update / локальных изменений `TaskModel` в CRUD. | Сервер — единственный источник истины. Будущее: бизнес-сервисы автоматически пересчитывают ЛГ всех проектов, клиент — тонкий терминал. |
| 27 | **Явное завершение МГ-задачи из ЛГ** (кнопка «Завершить»). Автоматическое завершение (когда все ЛГ-задачи Completed) — отменено. | Целевой срок задаёт только человек; «псевдо-фактический» срок (forecast = target при Completed) — баг. |
| 28 | **Refresh токена через IPC** (production_client ↔ gantt_editor). `TokenRefreshServer` (QLocalServer `engineering_re_token_refresh`) + `TokenRefreshClient` (QLocalSocket). | gantt_editor — отдельный процесс без AuthService. Refresh делает production_client, передаёт новый токен через IPC. |
| 29 | **`ApiClient` — очередь запросов после refresh.** При `401 Token expired` → callback → refresh → повтор из `m_pendingAfterRefresh`. | Прозрачный refresh для пользователя. |

## 3. События и уведомления (PROJECT_service → другие)

| Событие | Получатель | Когда | Что делает получатель |
|---|---|---|---|
| `project.created` | `MG_service` | Создание проекта | Создать МГ (черновик) |
| `project.updated` | `NOTIFICATION_service` | Изменение реквизита | Уведомить команду |
| `project.member_added` | `NOTIFICATION_service` | Добавление | Уведомить включённого |
| `project.member_removed` | `NOTIFICATION_service` | Удаление | Уведомить исключённого |
| `project.status_changed` | `MG_service` + `NOTIFICATION_service` | Смена статуса | MG: (открытый вопрос). |
| `project.deleted` | `MG_service` + `NOTIFICATION_service` | Удаление | MG: архивировать? |
| `mg.approval_requested` | `NOTIFICATION_service` | Отправка на согласование | Уведомить команду |
| `mg.approved` | `NOTIFICATION_service` | Утверждение | Уведомить команду |
| `mg.rejected` | `NOTIFICATION_service` | Отклонение | Уведомить CVE/IPP |
| `mg.withdrawn` | `NOTIFICATION_service` | Отзыв с согласования | Уведомить команду |
| `task.responsible_assigned` | `NOTIFICATION_service` | Назначение | Уведомить назначенного |
| `delegation.granted` | `NOTIFICATION_service` | Делегирование | Уведомить получателя |

## 4. Вопросы и ответы (сессия)

**Блок 1: WS-авторизация**
- WS через Gateway: ✅
- Активность: Вариант B (IPC).
- ЛГ: view на handshake.
- Outbox MG → BUS: ✅
- Н7: убрать запрос причины при изменении прогноза: ✅
- Н8: `local_graph_state` через outbox: ✅

**Блок 2: Outbox**
- Таблица в каждой БД: ✅
- Удалять после ack: ✅
- Ack: C.
- Backoff: [5, 15, 60, 300, 900].
- Идемпотентность по `project_id`: ✅
- Одна таблица `outbox` с `event_type`: ✅

**Блок 3: N+1**
- Batch + кэш: ✅
- `LEFT JOIN`: ✅
- `GROUP BY`: ✅

**Блок 4: WS-реконнект**
- Backoff: 1→2→4→8→30s.
- `last_version` в URL.
- Очередь: WS + REST fallback.
- Индикатор: в окне редактора.

**Блок 5: `onWsSnapshot` / `onWsDiff`**
- Гибрид (C).
- Все события.
- Поля: milestone, dependency, group.

**Блок 6: Токен**
- `DataManager` не передаёт Authorization — не проблема.
- Env + удаление файла.
- Обновлять `m_currentSession`.
- Очищать токен.

**Блок 7: QML и UI**
- Проверить пути `qmlRegisterType`.
- Убрать `qml/main.qml`, `interfaces/*.json`.
- Убрать `Colors.toggleTheme()` из `TopBar`.
- Две `MouseArea` (фокус/закрытие).
- `MainWindowBridge::loadUnreadCount()`.
- `DelegatedWindow` — Спираль 8.
- `ImpersonateWindow` — вкладка, Спираль 8.

**Блок 8: Рефакторинг**
- Схемы разбиения.
- Проверка: `renderer_preview`, `curl`.

**Блок 9: Тесты**
- Characterization, Smoke, Unit (Спираль 9).

**Блок 10: Документация**
- `SERVICES.md` — с нуля.
- `DECISIONS.md` — таблица.
- `CURRENT_STATE.md` — ссылки.
- `TEST_PLAN.md`.

**Новые пункты:**
- Скрипты локального запуска — задокументировать.
- CI/CD — Спираль 12.
- Страница выбора МГ с разделами по статусам (UI Composer).

## 5. Баги и проблемы — единая таблица

**Статусы:** ✅ исправлен (проверено) / 🟡 исправлен (не проверено в рантайме) / ❌ открыт / ⏸ отложен / 🚫 отменён.

### 5.1. Сессия 1 (2026-11-16)

| № | Проблема | Файл | Статус |
|---|---|---|---|
| 1 | Миграция 001 (pgcrypto) | MG_service | ✅ |
| 2 | `update_task` UnboundLocalError | MG_service | 🚫 (не баг) |
| 3 | WS-авторизация MG | MG_service | ✅ |
| 4 | WS-авторизация BUS | BUS_service | ✅ |
| 5 | `create_mastergraph` гонка | MG_service | ✅ |
| 6 | `TokenManager::isTokenValid` | CLIENT | ✅ |
| 7 | `ApiClient::buildUrl` | CLIENT | ✅ |
| 8 | `delete_project` не работает | PROJECT_service | ✅ |
| 9 | `PATCH /status` без валидации | PROJECT_service | ✅ |
| 10 | `ProjectRepository.update` не очищает | PROJECT_service | ✅ |
| 11 | **Н1:** ЛГ не открывается | WS/клиент | ✅ (X-User-ID) |
| 12 | **Н2:** Пересортировка задач | TaskModel | ❌ |
| 13 | **Н3:** Ширина GridArea ≠ CalendarHeader | QML | ❌ (TimelineController) |
| 14 | Outbox отсутствует | сервисы | ❌ |
| 15 | N+1 | сервисы | ❌ |
| 16 | `aiohttp` на каждый вызов | сервисы | ❌ |
| 17 | `create_project` без транзакции | PROJECT_service | ❌ |
| 18 | `mg_creation_worker` без `FOR UPDATE` | PROJECT_service | ❌ |
| 19 | WS без реконнекта | WebSocketClient | ✅ |
| 20 | `onWsSnapshot` пустой | ProjectController | ✅ |
| 21 | `onWsDiff` только task/update | ProjectController | ✅ (частично) |
| 22 | Токен через файл | main.cpp | ✅ (env) |
| 23 | `refreshToken` не обновляет сессию | AuthService | ✅ |
| 24 | `changePassword` не очищает токен | AuthService | ✅ |
| 25 | `submitForm` DELETE → POST | WidgetBridge | ✅ |
| 26 | SSL VerifyNone | ApiClient | ❌ |
| 27 | **Н4:** Нет подсветки просроченных в МГ | QML | ❌ |
| 28 | **Н5:** Нельзя выбрать ответственного в ЛГ | BUS_service | ❌ |
| 29 | **Н6:** Пиктограмма пароля | AuthWindow.qml | ❌ |
| 30 | `localhost:8080` хардкод | QML/C++ | ❌ |
| 31 | Двойное переключение темы | TopBar | ❌ |
| 32 | Двойная логика закрытия вкладки | MainWindow.qml | ❌ |
| 33 | `topBar` не в области видимости | NotificationsWindow | ❌ |
| 34 | WidgetLoader бесконечный цикл | WidgetLoader.qml | ❌ |
| 35 | CMAKE_CXX_STANDARD 17 | CMakeLists | ❌ |
| 36 | QML с диска, не из qrc | main.cpp | ❌ |
| 37 | resources.qrc мёртвые записи | resources.qrc | ❌ |
| 38 | gantt qml.qrc артефакты | qml.qrc | ❌ |
| 39 | Threads::Threads без find_package | common/CMakeLists | ❌ |
| 40 | GlobalSettings.fontSize не обновляется | GlobalSettings.qml | ❌ |
| 41 | DataManager::m_cache растёт | DataManager.cpp | ❌ |
| 42 | Диалоги без индикатора загрузки | QML | ❌ |

### 5.2. Сессия 2 (2026-11-24): ЛГ/МГ

| № | Проблема | Файл | Статус |
|---|---|---|---|
| 43 | **Баг 8:** GroupWizardDialog не объявлен в GanttEditorWindow.qml → окно МГ не грузилось. В ЛГ пункты «сверху/снизу» скрыты. | `GanttEditorWindow.qml`, `qml.qrc`, `GroupContextMenu.qml`, `GroupDelegate.qml` | ✅ |
| 44 | **Баг 8b:** позиционирование групп (стандартные/кастомные) — `after_group_id` + `shift_sort_orders_after`; fallback при not found. `DefaultGroupsDialog` → `"top"`. | `ProjectController.h/.cpp`, `GroupWizardDialog.qml`, `DefaultGroupsDialog.qml`, `mastergraphs.py`, `mg_group_repo.py` | ✅ |
| 45 | WS snapshot (МГ): `groups` из `get_groups_for_mg` (единый источник с REST, `sort_order`). Fallback `bus_url` на `other_bus`. | `mastergraphs_ws.py`, `mastergraph_repo.py` | ✅ |
| 46 | Атомарность `create_custom_group` (rollback при ошибке MgGroup). Проверка привязки группы к МГ в `create_task`/`batch`. | `mastergraphs.py`, `mastergraph_service.py` | ✅ |
| 47 | Комментарии в каскаде (`BatchTaskItem.comment`). | `schemas/mastergraph.py`, `mastergraphs.py`, `CascadeDialog.qml` | ✅ |
| 48 | Прогноз в МГ: причина не запрашивается для forecast. | `TaskBar.qml`, `EditForecastDatesDialog.qml` | ✅ |
| 49 | ЛГ: единый паттерн C (сервер — истина, WS snapshot → перерисовка). CRUD без локальных изменений `TaskModel`. | `LocalGraphController.cpp`, `ProjectController.cpp`, `local_graphs.py`, `local_graphs_ws.py` | ✅ |
| 50 | ЛГ-зависимости: BUS endpoints + `DependencyModel::addDependencyWithId` + `GanttApiClient` + `LocalGraphController`. | `local_graphs.py`, `schemas/local_graph.py`, `DependencyModel.h/.cpp`, `GanttApiClient.h/.cpp`, `LocalGraphController.h/.cpp`, `DependencyDialog.qml`, `TaskContextMenu.qml` | ✅ |
| 51 | `GanttApiClient::setUserId` + X-User-ID только в BUS-методах (фикс 401). | `GanttApiClient.h/.cpp`, `LocalGraphController.cpp` | ✅ |
| 52 | `local_graph_state`: чтение из BUS (не всегда 2), кэш в MG, индикатор только при `=== 2`. | `bus_client.py`, `mastergraphs.py`, `GroupDelegate.qml` | ✅ |
| 53 | Прогноз vs целевой: убрано приравнивание forecast к target при `Completed`; `_compute_forecast` не ставит `status = 1` автоматически. | `TaskModel.cpp`, `local_graphs.py` | ✅ |
| 54 | **Баг 2:** Масштаб не пересчитывает вехи. | `MilestoneBar.qml`, `CalendarHeader.qml` (фрагмент) | 🟡 (правка дана, не проверено) |
| 55 | **Баг 6 + 3d:** Блокировка прогноза (`lgs === 2` вместо `2 || 3`); dateless (1) — редактируется. | `TaskBar.qml`, `TaskContextMenu.qml` | 🟡 (правка дана, не проверено) |
| 56 | **Баг 4:** `DefaultGroupsDialog` binding loop (CheckBox.checked). | `DefaultGroupsDialog.qml` | 🟡 (правка дана, не проверено) |
| 57 | **Баг 1:** После вехи GridArea пустеет. | `ProjectController.cpp`, `GridArea.qml`, `MilestoneModel.cpp` | ❌ (диагноз: `getEarliestDate`/`getLatestDate` могут вернуть today; нужен лог) |
| 58 | **Баг 3:** GridArea ↔ CalendarHeader рассинхрон (МГ). | `GanttEditorWindow.qml`, `GridArea.qml`, `CalendarHeader.qml` | ❌ (TimelineController) |
| 59 | **Баг 5:** Создание групп не перерисовывает. | `ProjectController::createGroupsWithPosition` → `reloadMastergraph()` | ✅ (в коде есть) |

### 5.3. Сессия 2 (продолжение, 2026-11-26): refresh токена через IPC

| № | Проблема | Файл | Статус |
|---|---|---|---|
| 60 | **Баг 7:** Токен истёк → нет соединения. Refresh не работает. | `ApiClient.h/.cpp`, `AuthService.h/.cpp`, `WebSocketClient.h/.cpp`, `TokenRefreshServer.h/.cpp`, `TokenRefreshClient.h/.cpp`, `ProjectController.h/.cpp`, `AppCore.cpp`, `CMakeLists.txt` | 🟡 (собрано, `[TokenRefreshServer] listening` — есть; не проверено в рантайме) |
| 61 | **Порция B:** Кнопка «Завершить» в ЛГ (BUS endpoint `POST /{lg_id}/complete` + клиент + диалог + MG). | `local_graphs.py`, `schemas/local_graph.py`, `LocalGraphController.h/.cpp`, `CompleteLocalGraphDialog.qml`, `qml.qrc`, `GanttEditorWindow.qml`, `AppToolBar.qml` | 🟡 (правка дана, не применена/не проверена) |

### 5.4. Удалить (мёртвый код) — отложено

| № | Что | Файл |
|---|---|---|
| 62 | `CacheManager` | production_client/src/core/ |
| 63 | `ErrorDialog`, `UpdateNotification`, `ConfirmDialog` | qml/dialogs/ |
| 64 | `LoadingSpinner`, `MastergraphStatusCard`, `OfflineIndicator`, `ToastNotification` | qml/components/ |
| 65 | `ProjectsView`, `MastergraphView`, `DashboardView`, `SettingsView` | qml/views/ |
| 66 | `SearchableSelect` (решить после изучения) | qml/widgets/ |
| 67 | `DateUtils`, `StringUtils` | qml/utils/ |
| 68 | `AppStyle` | qml/styles/ |
| 69 | `HistoryEntry` | gantt_editor/include/, src/ |

### 5.5. Не проблема (проверено)
- **П.17** (`DataManager` не передаёт Authorization) — `UI Composer` резолвит `sources` сам.
- **П.26** (заглушки данных) — механизм `renderer_preview`.

## 6. План правок (актуальный)

### Этап 1: Бэкенд — критические баги (✅ завершён)
1–10 — исправлены.

### Этап 2: Бэкенд — безупречность
11. Outbox.
12. N+1.
13. `aiohttp` переиспользование.
14. `create_project` — транзакция.
15. `mg_creation_worker` — `FOR UPDATE SKIP LOCKED`.
16. События → Notification.
17. Centralized `event_log`.

### Этап 3: Клиент — критические баги (✅ завершён)
18–26, 43–53 — исправлены.

### Этап 4: Клиент — безупречность
27. Рефакторинг (разбиение файлов) — **Шаг 7**.
28. `CMAKE_CXX_STANDARD 20`.
29. QML из qrc (Вариант A).
30. Очистка `resources.qrc`, `gantt_editor/qml.qrc`.
31. `common/CMakeLists.txt` — `find_package(Threads)`.
32. `GlobalSettings.fontSize` — синхронизация.
33. `DataManager::m_cache` — LRU.
34. Удалить мёртвый код (62–69).
35. Н4 (подсветка просроченных в МГ).
36. Н5 (ответственный в ЛГ) — доработка BUS.
37. Н6 (пиктограмма пароля).
38. `localhost:8080` — вынести в конфиг.
39. `TopBar` — убрать двойное переключение.
40. `MainWindow` — заголовок вкладки (две MouseArea).
41. `NotificationsWindow` — `MainWindowBridge::loadUnreadCount()`.
42. `WidgetLoader` — исправить/удалить.
43. Диалоги — индикатор загрузки.
44. `qmlRegisterType` — проверить пути.
45. **Баг 1** — обновление после вехи (нужен лог).
46. **Баг 2** — правка дана, проверить.
47. **Баг 4** — правка дана, проверить.
48. **Баг 6 + 3d** — правка дана, проверить.
49. **TimelineController** + zoom «Месяц» — **Баг 3**.
50. **Целевой период** (Шаг 6).
51. **Wizard**.
52. **Согласование МГ**.
53. **Порция B: кнопка «Завершить» в ЛГ** — правка дана, применить/проверить.
54. **Баг 7 (refresh IPC)** — собрано, проверить Сценарий 1 / 2.

### Этап 5: Тесты
55. Characterization.
56. Smoke.

### Этап 6: Документация (второй проход)
57. Обновить все документы.
58. Финализировать worklog.

## 7. Чек-лист проверки
- Бэкенд: сборка, smoke, миграции, WS.
- Клиент: сборка, запуск, сценарий, `renderer_preview`.
- Документация: соответствие коду.

## 8. Текущее состояние (2026-11-27)
- Бэкенд: критические баги ✅, outbox/N+1 — ❌.
- Клиент: критические баги ✅; refresh IPC — собрано, не проверено; Порция B — правка дана, не применена; Баг 1 — нужен лог; Баг 2/4/6 — правки даны, не проверены; Timeline/Wizard/Согласование/Целевой период — ❌.
- Документация: **обновлена 2026-11-27** (этот worklog, `progress.md`, `spirals.md`, `phase_1_plan.md`, `phase_1_platform_logic.md`).

## 9. Что показать в следующем диалоге
- MG_service: `mastergraphs.py`, `mastergraph_service.py`, `mastergraphs_ws.py`.
- BUS_service: `local_graphs.py`, `local_graphs_ws.py`.
- PROJECT_service: `projects.py`, `project_service.py`, `project_repo.py`.
- CLIENT: `ProjectController.cpp`, `TaskBar.qml`, `GridArea.qml`, `CalendarHeader.qml`, `MilestoneBar.qml`, `LocalGraphEditorWindow.qml`, `DefaultGroupsDialog.qml`, `MainWindow.qml`, `AuthService.cpp`, `ApiClient.cpp`, `WebSocketClient.cpp`, `TokenRefreshServer.cpp`, `TokenRefreshClient.cpp`, `AppCore.cpp`.

## 10. Идеи на будущее
- CI/CD (Спираль 12).
- Страница выбора МГ с разделами по статусам.
- `ImpersonateWindow` → вкладка.
- `DelegatedWindow` — доработка.
- Н5 (ответственный в ЛГ).
- Просмотр динамики МГ (event_log).

## 13. Архитектура МГ: Wizard, Согласование, TimelineController

### 13.1. Wizard быстрой инициализации МГ
- Открывается автоматически, если МГ новый (нет вех, задач, не утверждён) и пользователь — CVE/IPP.
- Содержит: название, типологию, список вех (из типологии), поле ввода даты первой вехи, чекбокс «типовые группы задач».
- Если пользователь задал вехи (или вехи+группы) — wizard больше не открывается автоматически.

### 13.2. Согласование МГ (кнопка «Черновик/Чистовик»)

**Режим «Черновик»:**
- Обычный член — окно не открывается.
- CVE/IPP — окно «Отправить на согласование».
  - `MG_service` проверяет: вехи заданы, группы заданы, есть задачи, у всех — ответственный и целевые сроки.
  - Успех → режим «Согласование».
  - Отказ → список причин.

**Режим «Согласование»:**
- Обычный член — окно с таблицей (ФИО | дата | согласовано), кнопки «Согласовать»/«Отклонить».
  - Отозвать подпись нельзя. Отклонил → может согласовать позже (дата обновляется, крестик → галочка).
- CVE/IPP — редактирование недоступно (замок). Окно с кнопкой «Отозвать МГ с согласования».
  - Отзыв → режим «Черновик», все подписи отзываются.
  - CVE и IPP тоже должны согласовать.
- Если вся команда согласовала — CVE может «Утвердить МГ».
  - МГ → «Утверждён».
  - Редактор → «Чистовик»: редактирование разблокировано, каждое изменение сроков — с причиной и серыми полосами (история).

### 13.3. TimelineController
- Единый источник `displayStart`, `displayEnd`, `dayWidth`.
- Учитывает `project_target_start` / `project_target_end` (Project → MG → Client).
- Правило: `displayStart` / `displayEnd` не могут быть меньше целевых сроков проекта. Больше — могут.
- Zoom levels: Day (30px), Week (10px), Month (3px).
- На Month: `CalendarHeader` — годы и месяцы; `GridArea` — только месяцы; `MilestoneBar` — вехи; `TaskBar` — только полосы.

### 13.4. Целевой период проекта
- `PROJECT_service`: `target_start_date`, `target_end_date` в `Project`.
- `MG_service`: те же в `Mastergraph`.
- Передаётся через `MgCreationQueue` → `create_mastergraph_from_typology`.
- Отдаётся в WS snapshot и REST.

### 13.5. Масштаб «Месяц»
- Третий zoom level в `SettingsManager`.
- Отдельная отрисовка (без дней).
