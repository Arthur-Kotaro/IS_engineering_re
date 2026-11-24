# Прогресс: фазы и спирали

**Версия:** 6.1
**Дата обновления:** 2026-11-16
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

## Детали спиралей

### Спираль 6.6 (завершена)
- PRE, BDERE, GDERE.
- BaseWidgets 1.0.
- MG Service (каркас).

### Спираль 7 (в работе)
- **Завершено:** миграция 001, WS-авторизация (MG, BUS, Nginx), `create_mastergraph` (гонка), `TokenManager::isTokenValid`, `ApiClient::buildUrl`, `delete_project`, `PATCH /status`, `ProjectRepository.update`, `refreshToken`, `changePassword`, `submitForm`, токен через env, `onWsSnapshot` (гибрид), `onWsDiff` (все сущности), WS-реконнект, индикатор.
- **Открыто:** outbox (полный), N+1, `aiohttp`, `create_project` транзакция, `mg_creation_worker`, события → Notification, `event_log`, SSL VerifyNone, рефакторинг, CMAKE 20, QML из qrc, чистка qrc, Threads, `GlobalSettings.fontSize`, `DataManager::m_cache`, мёртвый код, Н4–Н6, `localhost:8080`, `TopBar`, `MainWindow` (вкладки), `NotificationsWindow`, `WidgetLoader`, диалоги (индикатор), `qmlRegisterType`, баг 1 (веха), баг 2 (масштаб), баг 3 (GridArea/CalendarHeader), баг 4 (окно групп), баг 5 (группы), баг 6 (прогноз/ЛГ), баг 7 (токен), TimelineController, Zoom Month, Целевой период, Wizard, Согласование.

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
