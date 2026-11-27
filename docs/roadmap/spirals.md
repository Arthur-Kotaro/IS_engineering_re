# Дорожная карта спиралей

**Версия:** 6.1
**Дата обновления:** 2026-11-24
**Статус:** Спираль 7 (Макет MG → ERP) в работе.

## Статус

- [x] **Спираль 1.** Gateway + Blacklist.
- [x] **Спираль 2.** Синхронизация слоёв — User, Auth.
- [x] **Спираль 3.** Delegation + Project (отменена в 6.6).
- [x] **Спираль 4.** Имперсонация (требует доработки после 6.6).
- [x] **Спираль 5.** Workflow (UI Composer engine, HR-сценарий).
- [x] **Спираль 6.** Документация.
- [x] **Спираль 6.5.** Секреты + env.
- [x] **Спираль 6.6.** Roles & Positions & Departments.

- [ ] **Спираль 7. Макет MG → ERP. (в работе)**
  - [x] Каркас `MG_service`, БД, API, права.
  - [x] Интеграция GDERE.
  - [x] WS-авторизация (Gateway + MG/BUS).
  - [x] Outbox (частично: `create_mastergraph`).
  - [x] Баг-фикс (сессия 1): миграция 001, `delete_project`, `PATCH /status`, `update`, `TokenManager`, `ApiClient`, `refreshToken`, `changePassword`, `submitForm`, Токен через env.
  - [x] **Баг 8** (GroupWizardDialog не открывался; в ЛГ скрыт).
  - [x] **Баг 8b** (позиционирование групп: стандартные/кастомные, top/bottom + `after_group_id`).
  - [x] **WS snapshot (МГ): `groups` из `get_groups_for_mg`** (единый источник с REST, `sort_order`).
  - [x] **Атомарность `create_custom_group`** (rollback) + проверка привязки группы к МГ в `create_task`/`batch`.
  - [x] **Комментарии в каскаде** (`BatchTaskItem.comment`).
  - [x] **Прогноз в МГ: причина не требуется** (для forecast).
  - [x] **ЛГ: единый паттерн C** (WS snapshot → перерисовка; CRUD без локальных изменений).
  - [x] **ЛГ-зависимости** (BUS endpoints + клиент + модель).
  - [x] **`GanttApiClient::setUserId`** + X-User-ID только в BUS-методах (фикс 401).
  - [x] **`local_graph_state`: чтение из BUS, кэш в MG, индикатор `=== 2`**.
  - [x] **Прогноз vs целевой**: убрано приравнивание forecast к target при `Completed`; `_compute_forecast` не ставит `status = 1` автоматически.
  - [ ] **Порция B: кнопка «Завершить» в ЛГ** (BUS endpoint `POST /{lg_id}/complete` + клиент + диалог + MG).
  - [ ] Outbox (полный: MG → BUS, BUS → MG).
  - [ ] N+1 (batch, join, group by).
  - [ ] Refactoring (разбиение больших файлов) — **Шаг 7**.
  - [ ] TimelineController (единый `displayStart/End/dayWidth`).
  - [ ] Zoom level «Месяц».
  - [ ] Целевой период проекта (Project → MG → Client).
  - [ ] Wizard инициализации МГ.
  - [ ] Согласование МГ (workflow).
  - [ ] Refresh токена / поддержка WS-соединений.
  - [ ] Баг 1 (веха → GridArea).
  - [ ] Баг 2 (масштаб → вехи) — решается в TimelineController.
  - [ ] Баг 3 (GridArea ↔ CalendarHeader) — решается в TimelineController.
  - [ ] Баг 4 (окно групп).
  - [ ] Баг 5 (группы не перерисовывают).
  - [ ] Баг 6 (блокировка прогноза при пустом ЛГ).
  - [ ] Баг 7 (токен истёк).
  - [ ] `DefaultGroupsDialog` binding loop (CheckBox.checked).
  - [ ] 3d: 4 состояния `local_graph_state` + «BUS недоступен».
  - [ ] Автосоздание ЛГ при `init_local_graph` (архитектурное).

- [ ] **Спираль 8. Треды + ревизия делегаций + ревизия уведомлений.**
- [ ] **Спираль 9. Тесты.** (отложена, Фаза 0)
- [ ] **Спираль 10. Прод-готовность.** (отложена, Фаза 1)
- [ ] **Спираль 11. Инициализация ERP.** (BDERE, справочник, workflow)
- [ ] **Спираль 12. CI/CD.** (персональный `.env` для каждого сервиса)
- [ ] **Спираль 13. Мобильный клиент (Android).**

## Отменено
- Модель `owner/manager/editor/viewer`.
- `head_id` у пользователя.
- Композиция `direct OR subordinate OR delegated`.
- Роль X («Директор по менеджменту проектов»).
- `upp_head`.

## Требования к разработке
1. **C/C++ строго в стиле Allman.**
2. **Файл > 450 строк → делим на несколько файлов.**
3. Код на Python — PEP 8.
4. Одна порция = одна проверка.
5. Новые файлы и перезаписи — через `cat > файл <<'EOF'`.
6. Точечные правки — вручную.
7. `sed`, `awk` — не использовать.
