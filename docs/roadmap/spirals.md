# Дорожная карта спиралей

## Спираль 1. Gateway + Blacklist
- Установить Redis.
- User Service: blacklist в Redis. Удалить TokenBlacklist, cleanup-задачу.
- Auth Service: переписать под /verify.
- Nginx: auth_request, кэш, публичные эндпоинты, /internal/, /page/.
- Проверка end-to-end.
- Удалить репозиторный API_Gateway/.
- Обновить start_all.sh.

## Спираль 2. Синхронизация слоёв — User, Auth
- User: убрать awaitable_attrs.projects, починить hr.py::create_user, подключить admin.py.
- Удалить мёртвый код.
- Auth: убрать прокси-логику.
- Проверить все сценарии.

## Спираль 3. Синхронизация слоёв — Delegation, Project
- Delegation: убрать REVERSE, два типа (DIRECT, TEMPORARY), initiator_id.
- Project: модель прав, check-access, list-with-access.
- Композиция direct OR subordinate OR delegated.

## Спираль 4. Имперсонация
- User: POST /internal/auth/impersonate, GET /internal/users/{id}/subordinates.
- UI Composer: страница /page/impersonate.
- Клиент: DelegatedWindow, плитка «Войти как …».
- Nginx: проброс X-Impersonated-By.

## Спираль 5. Workflow
- Redis DB 1 для сессий.
- UI Composer: WorkflowLoader, WorkflowValidator, WorkflowEngine, SessionService.
- Эндпоинты /workflow/event, /workflow/back, /workflow/close.
- Клиент: WidgetBridge.sendWorkflowEvent, TabContent.qml, DataManager.
- Сценарий HR-поиск — end-to-end.

## Спираль 6. Документация
- Финализировать ADR.
- Дополнить глоссарий.
- Контракты в docs/contracts/.

## Спираль 7. MG Service
- Модель мастерграфика, CRUD.
- Интеграция с Project (права).
- Интеграция с workflow.
- GanttChartView в клиенте.

## Спираль 8. Макет MG → ERP
- Аудит макета.
- Синтез с ERP.
- Перенос виджетов и сценариев.

## Спираль 9. Тесты
- Покрытие критичных сценариев.

## Спираль 10. Прод-готовность
- HTTPS, rate-limit, observability, CI/CD, бэкапы.

---

## Статус

- [x] Спираль 1. Gateway + Blacklist. Завершена 2026-09-26.
- [x] Спираль 2. Синхронизация слоёв — User, Auth. Завершена 2026-09-26. Тег `spiral-2-done`.
- [ ] Спираль 3. Синхронизация слоёв — Delegation, Project.
- [ ] Спираль 4. Имперсонация.
- [ ] Спираль 5. Workflow.
- [ ] Спираль 6. Документация.
- [ ] Спираль 7. MG Service.
- [ ] Спираль 8. Макет MG → ERP.
- [ ] Спираль 9. Тесты.
- [ ] Спираль 10. Прод-готовность.
