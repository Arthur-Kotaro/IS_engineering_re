# Прогресс: фазы и спирали

**Версия:** 6.1
**Дата обновления:** 2026-11-24
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
| 2026-11-16 | Спираль 7: исправлены критические баги (WS-авторизация, миграция, delete_project, PATCH/status, TokenManager, ApiClient, refreshToken, changePassword, submitForm, токен через env). Добавлены требования: TimelineController, Wizard, Согласование, Целевой период, Zoom Month. | 7 | 1 |
| 2026-11-24 | Спираль 7, сессия 2: Баг 8, 8b, единый паттерн ЛГ (WS snapshot), ЛГ-зависимости, индикатор `local_graph_state`, прогноз vs целевой (убрано приравнивание при Completed). | 7 | 1 |

## Детали спиралей

### Спираль 6.6 (завершена)
- PRE, BDERE, GDERE.
- BaseWidgets 1.0.
- MG Service (каркас).

### Спираль 7 (в работе)

**Завершено (сессия 1):**
- миграция 001, WS-авторизация (MG, BUS, Nginx), `create_mastergraph` (гонка), `TokenManager::isTokenValid`, `ApiClient::buildUrl`, `delete_project`, `PATCH /status`, `ProjectRepository.update`, `refreshToken`, `changePassword`, `submitForm`, токен через env, `onWsSnapshot` (гибрид), `onWsDiff` (все сущности), WS-реконнект, индикатор.

**Завершено (сессия 2):**
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

**Открыто (сессия 2 + ранее):**
- Кнопка «Завершить» в ToolBar ЛГ (BUS endpoint `POST /{lg_id}/complete` + клиент + диалог + MG) — **Порция B**.
- Outbox (полный), N+1, `aiohttp`, `create_project` транзакция, `mg_creation_worker`, события → Notification, `event_log`, SSL VerifyNone, рефакторинг (Шаг 7), CMAKE 20, QML из qrc, чистка qrc, Threads, `GlobalSettings.fontSize`, `DataManager::m_cache`, мёртвый код, Н4–Н6, `localhost:8080`, `TopBar`, `MainWindow`, `NotificationsWindow`, `WidgetLoader`, диалоги (индикатор), `qmlRegisterType`, Баг 1 (веха), Баг 2 (масштаб → вехи), Баг 3 (GridArea/CalendarHeader), Баг 4 (окно групп), Баг 5 (группы), Баг 6 (прогноз/ЛГ), Баг 7 (токен), TimelineController, Zoom Month, Целевой период, Wizard, Согласование, `DefaultGroupsDialog` binding loop, 3d (4 состояния `local_graph_state`), автосоздание ЛГ при `init_local_graph` (архитектурное).

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
