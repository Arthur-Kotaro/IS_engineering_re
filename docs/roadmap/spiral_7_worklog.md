# Спираль 7. Макет MG → ERP. Worklog

**Дата начала:** 2026-11-14
**Статус:** В работе (сессия вопросов-ответов завершена, готовимся к правкам).
**Связанные документы:** `phase_1_plan.md`, `CURRENT_STATE.md`, `spirals.md`

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

## 5. Найденные проблемы и баги

### 🔴 Критические
| № | Проблема | Файл | Статус |
|---|---|---|---|
| 1 | Миграция 001 (pgcrypto) | MG_service | ✅ исправлен |
| 2 | `update_task` UnboundLocalError | MG_service | ❌ отменён (не баг) |
| 3 | WS-авторизация MG | MG_service | ✅ исправлен |
| 4 | WS-авторизация BUS | BUS_service | ✅ исправлен |
| 5 | `create_mastergraph` гонка | MG_service | ✅ исправлен |
| 6 | `TokenManager::isTokenValid` | CLIENT | ✅ исправлен |
| 7 | `ApiClient::buildUrl` | CLIENT | ✅ исправлен |
| 8 | `delete_project` не работает | PROJECT_service | ✅ исправлен |
| 9 | `PATCH /status` без валидации | PROJECT_service | ✅ исправлен |
| 10 | `ProjectRepository.update` не очищает | PROJECT_service | ✅ исправлен |
| 11 | **Н1:** ЛГ не открывается | WS/клиент | ✅ исправлен (X-User-ID) |
| 12 | **Н2:** Пересортировка задач | TaskModel | ❌ открыт |
| 13 | **Н3:** Ширина GridArea ≠ CalendarHeader | QML | ❌ открыт (решается в TimelineController) |

### 🟡 Важные
| № | Проблема | Файл | Статус |
|---|---|---|---|
| 14 | Outbox отсутствует | сервисы | ❌ открыт |
| 15 | N+1 | сервисы | ❌ открыт |
| 16 | `aiohttp` на каждый вызов | сервисы | ❌ открыт |
| 17 | `create_project` без транзакции | PROJECT_service | ❌ открыт |
| 18 | `mg_creation_worker` без `FOR UPDATE` | PROJECT_service | ❌ открыт |
| 19 | WS без реконнекта | WebSocketClient | ✅ исправлен |
| 20 | `onWsSnapshot` пустой | ProjectController | ✅ исправлен |
| 21 | `onWsDiff` только task/update | ProjectController | ✅ исправлен (частично) |
| 22 | Токен через файл | main.cpp | ✅ исправлен (env) |
| 23 | `refreshToken` не обновляет сессию | AuthService | ✅ исправлен |
| 24 | `changePassword` не очищает токен | AuthService | ✅ исправлен |
| 25 | `submitForm` DELETE → POST | WidgetBridge | ✅ исправлен |
| 26 | SSL VerifyNone | ApiClient | ❌ открыт |
| 27 | **Н4:** Нет подсветки просроченных в МГ | QML | ❌ открыт |
| 28 | **Н5:** Нельзя выбрать ответственного в ЛГ | BUS_service | ❌ открыт |
| 29 | **Н6:** Пиктограмма пароля | AuthWindow.qml | ❌ открыт |
| 30 | `localhost:8080` хардкод | QML/C++ | ❌ открыт |
| 31 | Двойное переключение темы | TopBar | ❌ открыт |
| 32 | Двойная логика закрытия вкладки | MainWindow.qml | ❌ открыт |
| 33 | `topBar` не в области видимости | NotificationsWindow | ❌ открыт |
| 34 | WidgetLoader бесконечный цикл | WidgetLoader.qml | ❌ открыт |
| 35 | CMAKE_CXX_STANDARD 17 | CMakeLists | ❌ открыт |
| 36 | QML с диска, не из qrc | main.cpp | ❌ открыт |
| 37 | resources.qrc мёртвые записи | resources.qrc | ❌ открыт |
| 38 | gantt qml.qrc артефакты | qml.qrc | ❌ открыт |
| 39 | Threads::Threads без find_package | common/CMakeLists | ❌ открыт |
| 40 | GlobalSettings.fontSize не обновляется | GlobalSettings.qml | ❌ открыт |
| 41 | DataManager::m_cache растёт | DataManager.cpp | ❌ открыт |
| 42 | Диалоги без индикатора загрузки | QML | ❌ открыт |
| 43 | **Баг 1:** После вехи GridArea пустеет | ProjectController | ❌ открыт |
| 44 | **Баг 2:** Масштаб не пересчитывает вехи | MilestoneBar | ❌ открыт (решается в TimelineController) |
| 45 | **Баг 5:** Создание групп не перерисовывает | ProjectController | ❌ открыт (A1) |
| 46 | **Баг 6:** Блокировка прогноза при пустом ЛГ | TaskBar/BUS | ❌ открыт |
| 47 | **Баг 7:** Токен истёк → нет соединения | ApiClient/AuthService | ❌ открыт (G) |
| 48 | **Баг 4:** Окно групп мало | DefaultGroupsDialog | ❌ открыт (A2) |

### 🟢 Удалить (мёртвый код)
| № | Что | Файл |
|---|---|---|
| 49 | `CacheManager` | production_client/src/core/ |
| 50 | `ErrorDialog`, `UpdateNotification`, `ConfirmDialog` | qml/dialogs/ |
| 51 | `LoadingSpinner`, `MastergraphStatusCard`, `OfflineIndicator`, `ToastNotification` | qml/components/ |
| 52 | `ProjectsView`, `MastergraphView`, `DashboardView`, `SettingsView` | qml/views/ |
| 53 | `SearchableSelect` (решить после изучения) | qml/widgets/ |
| 54 | `DateUtils`, `StringUtils` | qml/utils/ |
| 55 | `AppStyle` | qml/styles/ |
| 56 | `HistoryEntry` | gantt_editor/include/, src/ |

### 📌 Не проблема (проверено)
- **П.17** (`DataManager` не передаёт Authorization) — `UI Composer` резолвит `sources` сам.
- **П.26** (заглушки данных) — механизм `renderer_preview`.

### ✅ Завершено (сессия 2, 2026-11-24)

| № | Что | Файлы |
|---|---|---|
| 1 | Баг 8: GroupWizardDialog не объявлен в GanttEditorWindow.qml → окно МГ не грузилось. В ЛГ пункты «сверху/снизу» скрыты. | `GanttEditorWindow.qml`, `qml.qrc`, `GroupContextMenu.qml`, `GroupDelegate.qml` |
| 2 | Баг 8b: позиционирование групп (стандартные/кастомные) — `after_group_id` + `shift_sort_orders_after`; fallback при not found. `DefaultGroupsDialog` → `"top"`. | `ProjectController.h/.cpp`, `GroupWizardDialog.qml`, `DefaultGroupsDialog.qml`, `mastergraphs.py`, `mg_group_repo.py` |
| 3 | WS snapshot (МГ): `groups` из `get_groups_for_mg` (единый источник с REST, `sort_order`). Fallback `bus_url` на `other_bus`. | `mastergraphs_ws.py`, `mastergraph_repo.py` |
| 4 | Атомарность `create_custom_group` (rollback при ошибке MgGroup). Проверка привязки группы к МГ в `create_task`/`batch`. | `mastergraphs.py`, `mastergraph_service.py` |
| 5 | Комментарии в каскаде (`BatchTaskItem.comment`). | `schemas/mastergraph.py`, `mastergraphs.py`, `CascadeDialog.qml` |
| 6 | Прогноз в МГ: причина не запрашивается для forecast. | `TaskBar.qml`, `EditForecastDatesDialog.qml` |
| 7 | ЛГ: единый паттерн C (сервер — истина, WS snapshot → перерисовка). CRUD без локальных изменений `TaskModel`. | `LocalGraphController.cpp`, `ProjectController.cpp`, `local_graphs.py`, `local_graphs_ws.py` |
| 8 | ЛГ-зависимости: BUS endpoints + `DependencyModel::addDependencyWithId` + `GanttApiClient` + `LocalGraphController`. | `local_graphs.py`, `schemas/local_graph.py`, `DependencyModel.h/.cpp`, `GanttApiClient.h/.cpp`, `LocalGraphController.h/.cpp`, `DependencyDialog.qml`, `TaskContextMenu.qml` |
| 9 | `GanttApiClient::setUserId` + X-User-ID только в BUS-методах (фикс 401). | `GanttApiClient.h/.cpp`, `LocalGraphController.cpp` |
| 10 | `local_graph_state`: чтение из BUS (не всегда 2), кэш в MG, индикатор только при `=== 2`. | `bus_client.py`, `mastergraphs.py`, `GroupDelegate.qml` |
| 11 | Прогноз vs целевой: убрано приравнивание forecast к target при `Completed`; `_compute_forecast` не ставит `status = 1` автоматически. | `TaskModel.cpp`, `local_graphs.py` |
### Сессия 2 (продолжение, 2026-11-26): refresh токена через IPC

**Проблема:** токен истекает через 15 мин, WS рвётся (401 / close code 1008), refresh не работает. `ApiClient` / `AuthService` / `WebSocketClient` не связаны с refresh.

**Решение (единая схема IPC):**
- `TokenRefreshServer` (production_client, `QLocalServer` имя `engineering_re_token_refresh`) — слушает дочерние процессы. При `{"action":"refresh"}` → `AuthService::refreshToken(currentRefreshToken)`. При `tokenRefreshed(newAccess, newRefresh)` → broadcast `{"action":"token","token":"<newAccess>"}` всем подключённым. При `sessionExpired` → broadcast `{"action":"error","message":"Session expired"}`.
- `TokenRefreshClient` (gantt_editor, `QLocalSocket`) — при `requestRefresh()` отправляет `{"action":"refresh"}`. При `{"action":"token"}` → `emit tokenRefreshed(newToken)`. При `{"action":"error"}` → `emit refreshFailed`.
- `ApiClient` (common) — при `401` + `detail == "Token expired"` → `m_onTokenExpired()` (callback от `AuthService`). Если `true` — запрос в `m_pendingAfterRefresh`. `AuthService::refreshToken` → `ApiClient::onTokenRefreshed()` → очередь повторяется. При неудаче — `ApiClient::onTokenRefreshFailed()` → `emit tokenExpired()`.
- `AuthService` (common) — в конструкторе `m_apiClient->setOnTokenExpired([this]{ return handleTokenExpired(); })`. `handleTokenExpired()` → `refreshToken(m_currentSession.refreshToken)`. `refreshToken` → `m_apiClient->onTokenRefreshed()` при успехе, `onTokenRefreshFailed()` при неудаче.
- `WebSocketClient` (gantt_editor) — `setOnTokenExpired(callback)`, `onTokenRefreshed(newToken)`, `onTokenRefreshFailed()`. При close code `1008` или WS-сообщении «Token expired» → `m_onTokenExpired()` (callback из `ProjectController` → `TokenRefreshClient::requestRefresh()`). При `onTokenRefreshed(newToken)` → `m_token = newToken` → переподключение `connectToServer(m_url, m_token)`.
- `ProjectController` (gantt_editor) — создаёт `TokenRefreshClient`, связывает: `TokenRefreshClient::tokenRefreshed` → `m_token = newToken`, `m_apiClient->setToken`, `m_webSocket->onTokenRefreshed(newToken)`. `m_webSocket->setOnTokenExpired([this]{ return m_tokenRefreshClient->requestRefresh(); })`.

**Новые файлы:**
- `CLIENT/production_client/gantt_editor/include/TokenRefreshClient.h`
- `CLIENT/production_client/gantt_editor/src/TokenRefreshClient.cpp`
- `CLIENT/production_client/include/TokenRefreshServer.h`
- `CLIENT/production_client/src/TokenRefreshServer.cpp`

**Изменены:**
- `CLIENT/common/include/userserviceclient/ApiClient.h` — `TokenExpiredHandler`, `PendingRequest`, `m_refreshing`, `m_pendingAfterRefresh`, `onTokenRefreshed`, `onTokenRefreshFailed`.
- `CLIENT/common/src/ApiClient.cpp` — обработка `401 Token expired`, `executeRequest` с сохранением `PendingRequest` в `reply` property, `enqueueAfterRefresh`, `onTokenRefreshed`, `onTokenRefreshFailed`.
- `CLIENT/common/include/userserviceclient/AuthService.h` — `handleTokenExpired`.
- `CLIENT/common/src/AuthService.cpp` — `setOnTokenExpired` в конструкторе, `handleTokenExpired`, `onTokenRefreshed` / `onTokenRefreshFailed` в `refreshToken`.
- `CLIENT/production_client/gantt_editor/include/WebSocketClient.h` — `setOnTokenExpired`, `onTokenRefreshed`, `onTokenRefreshFailed`, `m_refreshing`, `m_reconnectAfterRefresh`.
- `CLIENT/production_client/gantt_editor/src/WebSocketClient.cpp` — обработка `1008` / WS «Token expired» → refresh → переподключение.
- `CLIENT/production_client/gantt_editor/include/ProjectController.h` — `+TokenRefreshClient*`, `+Q_PROPERTY tokenRefreshClient`, `+include TokenRefreshClient.h`.
- `CLIENT/production_client/gantt_editor/src/ProjectController.cpp` — создание `TokenRefreshClient`, связывание с `WebSocketClient`.
- `CLIENT/production_client/gantt_editor/CMakeLists.txt` — `+TokenRefreshClient.cpp/.h`.
- `CLIENT/production_client/CMakeLists.txt` — `+TokenRefreshServer.cpp`.

**Ещё не сделано (для полной схемы IPC):**
- `AppCore.cpp` / `main.cpp` (production_client) — создать `TokenRefreshServer`, связать с `AuthService` (`tokenRefreshed` → `onTokenRefreshed`, `sessionExpired` → `onSessionExpired`), `listen()`, `setCurrentRefreshToken(authService->currentSession().refreshToken)`.
- **Проверка** (Сценарий 1: production_client refresh после 15 мин; Сценарий 2: gantt_editor IPC refresh + переподключение WS).

**Статус:** правки **не проверены** (ПК не актуален, БД не актуальны). Требуется проверка на чистой среде.

## 6. План правок

### Этап 0: Документация
1. Аудит старой документации.
2. `SERVICES.md`, `DECISIONS.md`, `CURRENT_STATE.md`, `TEST_PLAN.md`.
3. Зафиксировать скрипты запуска.

### Этап 1: Бэкенд — критические баги (✅ завершён)
1-10 — исправлены.

### Этап 2: Бэкенд — безупречность
11. Outbox.
12. N+1.
13. `aiohttp` переиспользование.
14. `create_project` — транзакция.
15. `mg_creation_worker` — `FOR UPDATE SKIP LOCKED`.
16. События → Notification.
17. Centralized `event_log`.

### Этап 3: Клиент — критические баги (✅ завершён)
18-26 — исправлены.

### Этап 4: Клиент — безупречность
27. Рефакторинг (разбиение файлов).
28. `CMAKE_CXX_STANDARD 20`.
29. QML из qrc (Вариант A).
30. Очистка `resources.qrc`, `gantt_editor/qml.qrc`.
31. `common/CMakeLists.txt` — `find_package(Threads)`.
32. `GlobalSettings.fontSize` — синхронизация.
33. `DataManager::m_cache` — LRU.
34. Удалить мёртвый код (49–56).
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
45. **Баг 1** — обновление после вехи.
46. **Баг 5** — `createGroups` → `reloadMastergraph()`.
47. **Баг 4** — окно групп.
48. **TimelineController** (B) + zoom «Месяц» (C).
49. **Целевой период** (D).
50. **Wizard** (E).
51. **Согласование МГ** (F).
52. **Refresh токена / WS** (G).
53. **Баг 6** — блокировка прогноза при пустом ЛГ.

### Этап 5: Тесты
54. Characterization.
55. Smoke.

### Этап 6: Документация (второй проход)
56. Обновить все документы.
57. Финализировать worklog.

## 7. Чек-лист проверки
- Бэкенд: сборка, smoke, миграции, WS.
- Клиент: сборка, запуск, сценарий, `renderer_preview`.
- Документация: соответствие коду.

## 8. Текущее состояние
- Бэкенд: критические баги ✅, outbox/N+1 — ❌.
- Клиент: критические баги ✅, Timeline/Wizard/Согласование — ❌.
- Документация: устарела.

## 9. Что показать в следующем диалоге
- MG_service: `mastergraphs.py`, `mastergraph_service.py`, `mastergraphs_ws.py`.
- BUS_service: `local_graphs.py`, `local_graphs_ws.py`.
- PROJECT_service: `projects.py`, `project_service.py`, `project_repo.py`.
- CLIENT: `ProjectController.cpp`, `TaskBar.qml`, `GridArea.qml`, `CalendarHeader.qml`, `LocalGraphEditorWindow.qml`, `DefaultGroupsDialog.qml`, `MainWindow.qml`, `AuthService.cpp`, `ApiClient.cpp`, `WebSocketClient.cpp`.

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
