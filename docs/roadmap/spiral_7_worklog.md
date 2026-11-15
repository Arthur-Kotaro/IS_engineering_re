# Спираль 7. Макет MG → ERP. Worklog

**Дата начала:** 2026-11-14
**Статус:** В работе (сессия вопросов-ответов завершена, готовимся к правкам).
**Связанные документы:** `phase_1_plan.md`, `CURRENT_STATE.md`, `spirals.md`

## 1. Цель спирали

Завершить интеграцию редактора диаграмм Ганта (GDERE) с ERP. Обеспечить безупречную работу МГ и ЛГ: авторизация WS, синхронизация snapshot/diff, outbox для надёжной доставки событий, рефакторинг клиента.

## 2. Архитектурные решения (приняты)

| № | Решение | Обоснование |
|---|---|---|
| 1 | **WS через API Gateway.** Добавить `auth_request` в `location /ws/`. Проверка токена на handshake. | Единая точка входа, как для REST. Снимает нагрузку с сервисов. |
| 2 | **Активность пользователя (Вариант B).** IPC через `QLocalSocket`/`QLocalServer`. | Чистое архитектурное решение. Ответственность клиента. |
| 3 | **Координатор активности.** Первый процесс, успешно вызвавший `listen()`. При аварии — первый, обнаруживший обрыв, перехватывает роль. | Надёжность (безупречность), нет единой точки отказа. |
| 4 | **Расположение Activity.** В `CLIENT/common/` (общий код для всех клиентов). | Переиспользование в `production_client`, `gantt_editor`, будущем мессенджере. |
| 5 | **При `coordinator_shutdown`** дочерние процессы продолжают работу, перехватывают роль. | Пользователь может работать в редакторе без MainWindow. |
| 6 | **При timeout (30 мин)** координатор рассылает `logout`, закрывает всё, показывает `AuthWindow`. Дочерние — закрываются. | Предсказуемое завершение сессии. |
| 7 | **Outbox:** таблица `outbox` в БД каждого сервиса. Удаляется после ack. | Транзакционность, изоляция, простота. |
| 8 | **Ack (Вариант C):** получатель сам подтверждает обработку (отдельный endpoint / идемпотентный ответ). | Надёжность. |
| 9 | **Event Log (Вариант C, гибрид):** существующие таблицы (`mg_events`, `lg_events`) остаются. Дополнительно — централизованный `event_log` (через outbox). | AI Manager читает только централизованный лог. |
| 10 | **N+1:** batch-чтение (`get_user_name`, `get_local_graph_state`), `LEFT JOIN` для `list_projects_with_access`, `GROUP BY` для `is_pure`. | Производительность. |
| 11 | **WS-реконнект:** backoff `1s → 2s → 4s → 8s → 30s`. `last_version` в URL. Очередь исходящих (WS + REST fallback). Индикатор в окне редактора. | Надёжность, предсказуемость. |
| 12 | **`onWsSnapshot` (C, гибрид):** если модель пуста — `fromJson`, иначе — обновить модель. | Сохраняет состояние UI. |
| 13 | **`onWsDiff`:** все сущности и события, влияющие на визуализацию (task, milestone, dependency, group). | Интерактивная панель. |
| 14 | **Токен:** передавать через переменную окружения + удалять временный файл. Обновлять `m_currentSession` при refresh. Очищать токен при changePassword. | Безопасность. |
| 15 | **Заголовок вкладки:** две смежные `MouseArea` (фокус / закрытие). Правая краснеет при наведении. Скругления верхних углов. | Надёжность, простота. |
| 16 | **`MainWindowBridge` — единый источник `unreadNotifications`.** | Слабая связность, один источник правды. |
| 17 | **QML из qrc (Вариант A).** Загрузка `qrc:/qt/qml/ProductionClient/qml/main.qml`. | Согласованность с `gantt_editor`. |
| 18 | **`ImpersonateWindow` → вкладка через `UI_COMPOSER_SERVICE`** (Спираль 8). | Меньше кода в клиенте. |
| 19 | **`DelegatedWindow`** — отложить до Спирали 8. | Не хватает контекста. |
| 20 | **CI/CD (новая Спираль 12).** Перед мобильным клиентом. Персональный `.env` для каждого сервиса. | Инфраструктура. Мобильный клиент → Спираль 13. |

## 3. События и уведомления (PROJECT_service → другие)

| Событие | Получатель | Когда | Что делает получатель |
|---|---|---|---|
| `project.created` | `MG_service` | Создание проекта | Создать МГ (черновик) |
| `project.updated` | `NOTIFICATION_service` | Изменение любого реквизита | Разослать уведомление всей команде |
| `project.member_added` | `NOTIFICATION_service` | Добавление участника | Уведомить включённого |
| `project.member_removed` | `NOTIFICATION_service` | Удаление участника | Уведомить исключённого |
| `project.status_changed` | `MG_service` + `NOTIFICATION_service` | Смена статуса | MG: (открытый вопрос). Notification: разослать команде |
| `project.deleted` | `MG_service` + `NOTIFICATION_service` | Удаление | MG: архивировать? Notification: уведомить |

**Уведомления (сводка):**
- `project.member_added` → Включённому.
- `project.member_removed` → Исключённому.
- `project.updated` → Всей команде.
- `task.responsible_assigned` (MG) → Назначенному.
- `delegation.granted` → Получателю.

## 4. Вопросы и ответы (сессия)

**Блок 1: WS-авторизация**
- 1.1 WS через Gateway: **согласны**.
- 1.3 Активность: **Вариант B** (IPC через QLocalSocket). Координатор — первый `listen()`. Перехват по закрытию/аварии.
- 1.4 Доступ к ЛГ: проверять **только view** на handshake. Редактирование — внутри.
- Outbox MG → BUS: **согласны**.
- Н7: убрать запрос причины при изменении прогноза: **согласны**.
- Н8: обновлять `local_graph_state` через outbox: **согласны**.

**Блок 2: Outbox**
- 2.1 Таблица `outbox` в каждой БД: **согласны**.
- 2.2 Удалять после ack, без архива. Event Log — отдельно (C).
- 2.3 Ack: **Вариант C** (получатель подтверждает).
- 2.4 Backoff: `[5, 15, 60, 300, 900]`.
- 2.5 Идемпотентность по `project_id`: **согласны**.
- 2.6–2.8 Одна таблица `outbox` с `event_type`.

**Блок 3: N+1**
- 3.1 Batch-чтение + кэш.
- 3.2 Batch-чтение + кэш.
- 3.3 `LEFT JOIN`.
- 3.4 `GROUP BY`.

**Блок 4: WS-реконнект**
- 4.1 Backoff: `1s → 2s → 4s → 8s → 30s`.
- 4.2 `last_version` в URL + `event_log` для diff.
- 4.3 Очередь: WS + REST fallback (C).
- 4.4 Индикатор: в окне редактора (connected / reconnecting / disconnected).

**Блок 5: `onWsSnapshot` / `onWsDiff`**
- 5.1 Гибрид (C).
- 5.2 Все события, влияющие на визуализацию.
- 5.3–5.5 Поля: milestone, dependency, group.

**Блок 6: Токен**
- 6.1 `DataManager` не передаёт Authorization — не проблема (UI Composer резолвит sources сам).
- 6.2 Переменная окружения + удаление файла.
- 6.3 Обновлять `m_currentSession`.
- 6.4 Очищать токен.

**Блок 7: QML и UI**
- 7.2 Проверить пути `qmlRegisterType`.
- 7.3 Убрать `qml/main.qml`, `interfaces/*.json`.
- 7.4 Убрать `Colors.toggleTheme()` из `TopBar`.
- 7.5 Две MouseArea (фокус / закрытие).
- 7.6 `MainWindowBridge::loadUnreadCount()`.
- 7.7 `DelegatedWindow` — Спираль 8.
- 7.8 `ImpersonateWindow` — вкладка, Спираль 8.

**Блок 8: Рефакторинг**
- 8.1–8.6 Схемы разбиения (см. план правок).
- 8.7 Проверка: `renderer_preview` (клиент), `curl` (бэкенд).

**Блок 9: Тесты**
- 9.1 Characterization: форма, список, графики, таблица, поля.
- 9.2 Smoke: health, mastergraphs, projects, notifications, delegations.
- 9.3 Unit: Спираль 9.

**Блок 10: Документация**
- 10.1 `SERVICES.md` — с нуля.
- 10.2 `DECISIONS.md` — проще (таблица).
- 10.3 `CURRENT_STATE.md` — ссылки на progress/spirals.
- 10.4 `TEST_PLAN.md`.

**Новые пункты:**
- Скрипты локального запуска (BUS двумя способами) — задокументировать.
- CI/CD — Спираль 12.
- Идея: страница выбора МГ с разделами по статусам (UI Composer).

## 5. Найденные проблемы и баги

### 🔴 Критические (блокеры)
| № | Проблема | Файл | Статус |
|---|---|---|---|
| 1 | `001` миграция: `CREATE EXTENSION` после `gen_random_uuid()` | `MG_service/migrations/001_*.sql` | открыт |
| 2 | `update_task`: `UnboundLocalError` (`dates_new`) | `MG_service/app/services/mastergraph_service.py` | открыт |
| 3 | WS-авторизация: `/ws/mastergraphs/{mg_id}` без проверки | `MG_service/app/ws/mastergraphs_ws.py` | открыт |
| 4 | WS-авторизация: `/ws/local-graphs/{lg_id}` без проверки | `BUS_service/app/ws/local_graphs_ws.py` | открыт |
| 5 | `create_mastergraph`: гонка при идемпотентности | `MG_service/app/api/v1/internal.py` | открыт |
| 6 | `TokenManager::isTokenValid` не реализован | `CLIENT/common/src/TokenManager.cpp` | открыт |
| 7 | `ApiClient::buildUrl`: баг с `else` (двойной `/api/v1/`) | `CLIENT/common/src/ApiClient.cpp` | открыт |
| 8 | `delete_project` не работает (нет `PERM_DELETE_PROJECT` в check_access) | `PROJECT_service/app/services/access_service.py` | открыт |
| 9 | `PATCH /status` — нет валидации статуса | `PROJECT_service/app/api/v1/projects.py` | открыт |
| 10 | `ProjectRepository.update`: нельзя очистить поле | `PROJECT_service/app/repositories/project_repo.py` | открыт |
| 11 | **Н1:** ЛГ не открывается (ответственный за задачу) | WS/`BUS_service`/клиент | открыт |
| 12 | **Н2:** Пересортировка задач в группе после ввода прогресса | `TaskModel` / `ProjectController` | открыт |
| 13 | **Н3:** Ширина `GridArea` != `CalendarHeader` в ЛГ | `LocalGraphEditorWindow.qml` | открыт |

### 🟡 Важные (для безупречности)
| № | Проблема | Файл | Статус |
|---|---|---|---|
| 14 | Outbox отсутствует (MG, BUS, PROJECT) | сервисы | открыт |
| 15 | N+1 в `_build_detail_response`, `list_projects_with_access`, `list_mastergraphs` | сервисы | открыт |
| 16 | `aiohttp.ClientSession` на каждый вызов | сервисы | открыт |
| 17 | `create_project`: несколько коммитов, нет единой транзакции | `PROJECT_service` | открыт |
| 18 | `mg_creation_worker`: нет `FOR UPDATE SKIP LOCKED`, нет `try/except` | `PROJECT_service` | открыт |
| 19 | WS без реконнекта | `WebSocketClient.cpp` | открыт |
| 20 | `onWsSnapshot` пустой | `ProjectController.cpp` | открыт |
| 21 | `onWsDiff` только task/update | `ProjectController.cpp` | открыт |
| 22 | Токен через временный файл (не удаляется) | `main.cpp`, `WidgetBridge.cpp` | открыт |
| 23 | `AuthService::refreshToken` не обновляет `m_currentSession` | `AuthService.cpp` | открыт |
| 24 | `AuthService::changePassword` не очищает токен | `AuthService.cpp` | открыт |
| 25 | `WidgetBridge::submitForm`: DELETE отправляет POST | `WidgetBridge.cpp` | открыт |
| 26 | `QSslConfiguration::setPeerVerifyMode(VerifyNone)` | `ApiClient.cpp` | открыт |
| 27 | **Н4:** Нет подсветки просроченных задач в МГ | `GanttEditorWindow.qml` | открыт |
| 28 | **Н5:** В ЛГ нельзя выбрать ответственного | `BUS_service` | открыт |
| 29 | **Н6:** Кнопка пароля — обе пиктограммы «открытый глаз» | `AuthWindow.qml` | открыт |
| 30 | `MainWindowBridge`: `localhost:8080` хардкод | `MainWindowBridge.cpp` | открыт |
| 31 | `TopBar`: двойное переключение темы | `TopBar.qml` | открыт |
| 32 | `MainWindow`: двойная логика закрытия вкладки | `MainWindow.qml` | открыт |
| 33 | `NotificationsWindow`: `topBar` не в области видимости | `NotificationsWindow.qml` | открыт |
| 34 | `WidgetLoader`: бесконечный цикл, несуществующие файлы | `WidgetLoader.qml` | открыт |
| 35 | `CMAKE_CXX_STANDARD 17` (нужно 20) | CMakeLists.txt | открыт |
| 36 | `qml/main.qml` с диска, а не из qrc | `main.cpp` | открыт |
| 37 | `resources.qrc`: мёртвые записи (`main.qml`, `interfaces/*.json`) | `resources.qrc` | открыт |
| 38 | `gantt_editor/qml.qrc`: артефакты макета (`default_*.json`) | `gantt_editor/qml.qrc` | открыт |
| 39 | `common/CMakeLists.txt`: `Threads::Threads` без `find_package` | `common/CMakeLists.txt` | открыт |
| 40 | `GlobalSettings.fontSize` не обновляется из `AppCore` | `GlobalSettings.qml` | открыт |
| 41 | `DataManager::m_cache` растёт бесконечно | `DataManager.cpp` | открыт |
| 42 | `ValidationErrorDialog` / `ChangePasswordDialog`: нет индикатора загрузки | QML | открыт |

### 🟢 Удалить (мёртвый код)
| № | Что | Файл |
|---|---|---|
| 43 | `CacheManager` | `production_client/src/core/` |
| 44 | `ErrorDialog`, `UpdateNotification`, `ConfirmDialog` | `qml/dialogs/` |
| 45 | `LoadingSpinner`, `MastergraphStatusCard`, `OfflineIndicator`, `ToastNotification` | `qml/components/` |
| 46 | `ProjectsView`, `MastergraphView`, `DashboardView`, `SettingsView` | `qml/views/` |
| 47 | `SearchableSelect` (решить после изучения) | `qml/widgets/` |
| 48 | `DateUtils`, `StringUtils` | `qml/utils/` |
| 49 | `AppStyle` | `qml/styles/` |
| 50 | `HistoryEntry` (дублирует `dateHistory`) | `gantt_editor/include/`, `src/` |

### 📌 Не проблема (проверено)
- **П.17** (`DataManager` не передаёт Authorization) — не проблема, `UI Composer` резолвит `sources` сам.
- **П.26** (заглушки данных в графиках/таблицах) — механизм `renderer_preview`, не баг.

## 6. План правок

### Этап 0: Документация (параллельно)
1. Аудит старой документации.
2. Создать `SERVICES.md` (с нуля, с разделами делегирование/уведомления).
3. Создать `DECISIONS.md` (таблица).
4. Создать `CURRENT_STATE.md` (ссылки на progress/spirals).
5. Создать `TEST_PLAN.md`.
6. Зафиксировать скрипты запуска (BUS двумя способами).

### Этап 1: Бэкенд — критические баги
1. `001` миграция — `CREATE EXTENSION` до `gen_random_uuid()`.
2. `update_task` — `UnboundLocalError`.
3. WS-авторизация (`MG_service`, `BUS_service`) — handshake + view.
4. `create_mastergraph` — идемпотентность.
5. `TokenManager::isTokenValid` — реализовать.
6. `ApiClient::buildUrl` — исправить.
7. `delete_project` — реализовать (или убрать).
8. `PATCH /status` — валидация.
9. `ProjectRepository.update` — очистка поля.

### Этап 2: Бэкенд — безупречность
10. Outbox (`PROJECT_service`, `MG_service`, `BUS_service`).
11. N+1 (batch, join, group by).
12. `aiohttp.ClientSession` — переиспользование.
13. `create_project` — одна транзакция.
14. `mg_creation_worker` — `FOR UPDATE SKIP LOCKED` + `try/except`.
15. События `project.updated`, `member_added/removed`, `status_changed`, `deleted` → Notification.
16. Centralized `event_log`.

### Этап 3: Клиент — критические баги
17. WS-реконнект (backoff, `last_version`, очередь, индикатор).
18. `onWsSnapshot` (C, гибрид).
19. `onWsDiff` (все сущности).
20. Токен через env + удаление файла.
21. `AuthService::refreshToken` — обновлять сессию.
22. `AuthService::changePassword` — очищать токен.
23. `WidgetBridge::submitForm` — DELETE.
24. `QSslConfiguration` — убрать VerifyNone.
25. ActivityCoordinator / ActivityClient (`common/`).
26. Н1 (ЛГ не открывается) — разобраться.
27. Н2 (пересортировка) — исправить.
28. Н3 (ширина GridArea) — исправить.

### Этап 4: Клиент — безупречность
29. Рефакторинг (8.1–8.6).
30. `CMAKE_CXX_STANDARD 20`.
31. QML из qrc (Вариант A).
32. `resources.qrc`, `gantt_editor/qml.qrc` — очистка.
33. `common/CMakeLists.txt` — `find_package(Threads)`.
34. `GlobalSettings.fontSize` — синхронизация.
35. `DataManager::m_cache` — LRU.
36. Удалить мёртвый код (П.43–50).
37. Н4 (подсветка просроченных в МГ).
38. Н5 (ответственный в ЛГ) — доработка BUS (большой, возможно Спираль 8).
39. Н6 (пиктограмма пароля).
40. `localhost:8080` — вынести в конфиг.
41. `TopBar` — убрать двойное переключение.
42. `MainWindow` — заголовок вкладки (две MouseArea).
43. `NotificationsWindow` — `MainWindowBridge::loadUnreadCount()`.
44. `WidgetLoader` — исправить/удалить.
45. `ValidationErrorDialog` / `ChangePasswordDialog` — индикатор загрузки.
46. `qmlRegisterType` — проверить пути.

### Этап 5: Тесты
47. Characterization (форма, список, графики, таблица, поля).
48. Smoke (health, mastergraphs, projects, notifications, delegations).

### Этап 6: Документация (второй проход)
49. Обновить `SERVICES.md`, `DECISIONS.md`, `CURRENT_STATE.md`, `TEST_PLAN.md`.
50. Финализировать `spiral_7_worklog.md`.

## 7. Чек-лист проверки

**После каждой правки:**
- **Бэкенд:** сборка (`py_compile`, `uvicorn`), smoke (`curl`), миграции (`init_db.py`), WS (`websocat`).
- **Клиент:** сборка (`cmake --build`), запуск, сценарий (логин → МГ → задача → ЛГ), `renderer_preview`.
- **Документация:** соответствие факту кода.

## 8. Текущее состояние кодовой базы

**Бэкенд:**
- `PROJECT_service`: работает, есть баги (delete, status, update).
- `MG_service`: работает, есть критические баги (WS, миграция, UnboundLocalError).
- `BUS_service`: работает, есть баги (WS, ответственный).

**Клиент:**
- `gantt_editor`: работает, есть баги (WS-реконнект, snapshot, diff, токен, Н1–Н3).
- `production_client`: работает, есть баги (QML, тема, вкладки, уведомления).
- `common`: `UsersServiceClient` работает, `TokenManager` неполный.

**Документация:** устарела (спирали 1–6.6), не отражает спираль 7.

## 9. Что показать в следующем диалоге

**Для бэкенда:**
- `MG_service/app/ws/mastergraphs_ws.py`
- `BUS_service/app/ws/local_graphs_ws.py`
- `MG_service/app/services/mastergraph_service.py`
- `PROJECT_service/app/services/access_service.py`
- `PROJECT_service/app/repositories/project_repo.py`

**Для клиента:**
- `CLIENT/production_client/gantt_editor/src/WebSocketClient.cpp`
- `CLIENT/production_client/gantt_editor/src/ProjectController.cpp`
- `CLIENT/common/src/AuthService.cpp`
- `CLIENT/common/src/TokenManager.cpp`
- `CLIENT/production_client/qml/windows/MainWindow.qml`
- `CLIENT/production_client/qml/components/TopBar.qml`
- `CLIENT/production_client/qml/windows/NotificationsWindow.qml`
- `CLIENT/production_client/resources.qrc`
- `CLIENT/production_client/main.cpp`

## 10. Идеи на будущее (не в Спирали 7)

- **CI/CD (Спираль 12):** персональный `.env` для каждого сервиса.
- **Страница выбора МГ:** разделы по статусам (UI Composer).
- **`ImpersonateWindow` → вкладка** (UI Composer, Спираль 8).
- **`DelegatedWindow`** — доработка (Спираль 8).
- **Н5 (ответственный в ЛГ)** — доработка BUS (Спираль 8).
- **Просмотр динамики МГ** (стрелки вперёд/назад) — на основе `event_log`.

---

## 11. Журнал выполненных правок (обновляется по ходу)

### ✅ Правка 1: Миграция `001` (MG_service)
- **Файл:** `MG_service/migrations/001_spiral_7_5_milestones_events.sql`
- **Что сделано:** `CREATE EXTENSION pgcrypto` перенесён в начало файла (до `gen_random_uuid()`).
- **Проверка:** `psql -f ...` — без ошибок, таблицы созданы.
- **Статус:** ✅ выполнено.

### ❌ Правка 2: `update_task` (`UnboundLocalError`)
- **Файл:** `MG_service/app/services/mastergraph_service.py`
- **Анализ:** `dates_new` используется только при `dates_old is not None` (short-circuit Python). `UnboundLocalError` **не возникает**. Ошибка в моём анализе.
- **Статус:** ❌ отменено (не баг).

### ✅ Правка 3: WS-авторизация
- **`gateway.conf`:** добавлен `auth_request` для WS, разделены маршруты `/ws/mastergraphs/` и `/ws/local-graphs/`, добавлен `upstream bus_service`.
- **`MG_service/app/ws/mastergraphs_ws.py`:** `accept()` до проверок, `X-User-ID` + `check_project_access`.
- **`MG_service/app/ws/manager.py`:** убран `accept`.
- **`BUS_service/app/ws/local_graphs_ws.py`:** `accept()` до проверок, `X-User-ID` + `check_project_access`, `UUID(lg_id)` для SQLAlchemy/SQLite.
- **`BUS_service/app/ws/manager.py`:** убран `accept`.
- **Проверка:**
  - WS МГ через Nginx (с токеном) → `snapshot` ✅
  - WS ЛГ через Nginx (с токеном) → `snapshot` ✅
  - Без токена → `401` ✅
- **Статус:** ✅ выполнено.

### 🟡 Правка 4: `create_mastergraph` (идемпотентность) — **следующая**
- **Файлы:** `MG_service/app/api/v1/internal.py`, `MG_service/app/services/mastergraph_creator.py`.
- **План:** обработать `IntegrityError` при гонке (вернуть существующий МГ).
- **Статус:** ⏳ ожидает.

### 🟡 Правки 5–9: (TokenManager, ApiClient, delete_project, status, update) — **ожидают**
- **Статус:** ⏳ ожидают.
