# Прогресс: фазы и спирали

**Версия:** 6.1
**Дата обновления:** 2026-11-27
**Статус:** Спираль 7 (Макет MG → ERP) в работе.

## Проекция «Фазы × Спирали»

| Фаза | Спирали | Статус фазы | Дата начала | Дата завершения |
|------|---------|-------------|-------------|-----------------|
| Фаза 0 | 1–6.6, 9 | Завершена (9 отложена) | 2026-09-26 | — |
| Фаза 1 | 7, 8, 10, 11, 12, 13 | В работе (7) | 2026-10-25 | — |
| Фаза 2 | — | Не начата | — | — |
| Фаза 3 | — | Возможное будущее | — | — |

## Хронология

| Дата | Событие | Спираль | Фаза |
|------|---------|---------|------|
| 2026-09-26 | Спирали 1–6 завершены. | 1–6 | 0 |
| 2026-10-04 | Спираль 6.5 завершена. | 6.5 | 0 |
| 2026-10-12 | Спираль 6.6 завершена. | 6.6 | 0 |
| 2026-10-25 | Спираль 7 начата. | 7 | 1 |
| 2026-11-14 | Спираль 7 переопределена: «Макет MG → ERP». Добавлены 8, 11, 12, 13. | 7, 8, 11, 12, 13 | 1 |
| 2026-11-16 | Спираль 7, сессия 1: критические баги (WS-авторизация, миграция, delete_project, PATCH/status, TokenManager, ApiClient, refreshToken, changePassword, submitForm, токен через env). Добавлены требования: TimelineController, Wizard, Согласование, Целевой период, Zoom Month. | 7 | 1 |
| 2026-11-24 | Спираль 7, сессия 2: Баг 8, 8b, единый паттерн ЛГ (WS snapshot), ЛГ-зависимости, индикатор `local_graph_state`, прогноз vs целевой. Правки Баг 2, 4, 6, 3d. | 7 | 1 |
| 2026-11-26 | Спираль 7, сессия 2 (продолжение): refresh токена через IPC (`TokenRefreshServer` ↔ `TokenRefreshClient`). Собрано, не проверено в рантайме. | 7 | 1 |
| 2026-11-27 | Спираль 7: обновление документации. Порция B (кнопка «Завершить») — правка дана. Баг 1 — нужен лог. | 7 | 1 |

## Детали спиралей

### Спираль 6.6 (завершена)
- PRE, BDERE, GDERE.
- BaseWidgets 1.0.
- MG Service (каркас).

### Спираль 7 (в работе)

**Завершено (сессия 1, проверено):**
- миграция 001, WS-авторизация (MG, BUS, Nginx), `create_mastergraph` (гонка), `TokenManager::isTokenValid`, `ApiClient::buildUrl`, `delete_project`, `PATCH /status`, `ProjectRepository.update`, `refreshToken`, `changePassword`, `submitForm`, токен через env, `onWsSnapshot` (гибрид), `onWsDiff` (все сущности), WS-реконнект, индикатор.

**Завершено (сессия 2, проверено):**
- Баг 8: `GroupWizardDialog` в `GanttEditorWindow.qml` (+`qml.qrc`); в ЛГ пункты «сверху/снизу» скрыты (`projectController.isLocalGraph`).
- Баг 8b: позиционирование групп — `after_group_id` + `shift_sort_orders_after`; fallback при not found; `DefaultGroupsDialog` → `"top"`.
- WS snapshot (МГ): `groups` из `get_groups_for_mg` (единый источник с REST, `sort_order`); fallback `bus_url` на `other_bus`.
- Атомарность `create_custom_group` (rollback при ошибке MgGroup); проверка привязки группы к МГ в `create_task`/`batch`.
- Комментарии в каскаде (`BatchTaskItem.comment`).
- Прогноз в МГ: причина не запрашивается для forecast.
- ЛГ: единый паттерн C (сервер — истина, WS snapshot → перерисовка). CRUD без локальных изменений `TaskModel`.
- ЛГ-зависимости: BUS endpoints + `DependencyModel::addDependencyWithId` + `GanttApiClient` методы + `LocalGraphController` методы.
- `GanttApiClient::setUserId` + X-User-ID только в BUS-методах (фикс 401).
- `local_graph_state`: чтение из BUS (не всегда 2), кэш в MG, индикатор только при `=== 2`.
- Прогноз vs целевой: убрано приравнивание forecast к target при `Completed` (в `setTaskStatus`, `updateTask`, `addTaskWithId`, `updateTaskDates`); `_compute_forecast` не ставит `status = 1` автоматически.
- **Баг 5** (создание групп не перерисовывает) — `ProjectController::createGroupsWithPosition` → `reloadMastergraph()` (в коде есть).

**Собрано, не проверено в рантайме (сессия 2, продолжение):**
- **Баг 7** (refresh токена через IPC): `TokenRefreshServer` (production_client) + `TokenRefreshClient` (gantt_editor) + `ApiClient` очередь + `AuthService::handleTokenExpired` + `WebSocketClient` refresh. `[TokenRefreshServer] listening on engineering_re_token_refresh` — поднялся. Требуется проверка Сценария 1 (production_client refresh после 15 мин) и Сценария 2 (gantt_editor IPC refresh + переподключение WS).

**Правка дана, не применена/не проверена:**
- **Порция B** (кнопка «Завершить» в ЛГ): BUS endpoint `POST /{lg_id}/complete` + `LgCompleteRequest` + `LocalGraphController::completeLocalGraph` + `CompleteLocalGraphDialog.qml` + `qml.qrc` + `GanttEditorWindow.qml` + `AppToolBar.qml` + `LocalGraphController.h` (с `canCompleteLocalGraph`).
- **Баг 2** (вехи при смене масштаба): `MilestoneBar.qml` (`Qt.callLater` + `requestPaint`) + `CalendarHeader.qml` (фрагмент `MilestoneBar { dayWidth: root.dayWidth; startDate: root.displayStart }`).
- **Баг 4** (`DefaultGroupsDialog` binding loop): `CheckBox` через `Binding` с `_updating`.
- **Баг 6 + 3d** (блокировка прогноза): `TaskBar.qml` (`isForecastLocked` — только `lgs === 2`), `TaskContextMenu.qml` (`canEditForecast` — `lgs !== 2`).

**Открыто (требует лог/диагностику):**
- **Баг 1** (после вехи GridArea пустеет): гипотеза — `getEarliestDate`/`getLatestDate` могут вернуть `today` (если `milestones` пуст в JSON или `hasAny = false`). Нужен лог `[GridArea] ... displayStart/End/totalRows` после вехи + `fromJson OK, milestones: N`.
- **Баг 3** (GridArea ↔ CalendarHeader рассинхрон в МГ): решается в TimelineController.

**Не начинали / отложено:**
- Outbox (полный), N+1, `aiohttp`, `create_project` транзакция, `mg_creation_worker`, события → Notification, `event_log`, SSL VerifyNone, рефакторинг (Шаг 7), CMAKE 20, QML из qrc, чистка qrc, Threads, `GlobalSettings.fontSize`, `DataManager::m_cache`, мёртвый код, Н4–Н6, `localhost:8080`, `TopBar`, `MainWindow`, `NotificationsWindow`, `WidgetLoader`, диалоги (индикатор), `qmlRegisterType`, TimelineController, Zoom Month, Целевой период, Wizard, Согласование, 3d (4 состояния `local_graph_state`), автосоздание ЛГ при `init_local_graph` (архитектурное).

### Спираль 8 (следующая)
- Discussion service (треды).
- Ревизия DELEGATION_SERVICE.
- Ревизия NOTIFICATION_SERVICE.

### Спираль 9 (отложена)
- Тесты.

### Спираль 10 (отложена)
- Прод-готовность.

### Спираль 11 (Фаза 1)
- BDERE.
- Телефонный справочник.
- Workflow (создание пользователей, CRUD проектов).

### Спираль 12 (Фаза 1)
- CI/CD. Персональный `.env` для каждого сервиса.

### Спираль 13 (Фаза 1)
- Mobile UI Composer.
- Android клиент.

## Требования к разработке
(см. `spirals.md`)
