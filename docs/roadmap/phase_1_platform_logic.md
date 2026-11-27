# Фаза 1. Бизнес-логика платформы

**Версия:** 6.1
**Дата обновления:** 2026-11-27

## 1. Создание пользователей
Две страницы: создание, генерация пароля.

## 2. Телефонный справочник
(см. `phase_1_plan.md`, п. 3).
Кэш: Redis DB 2, защита от stampede.

## 3. Треды обсуждения задач МГ (Спираль 8)
- Одна задача = один тред.
- Создаётся по необходимости.
- Участники, только текст, короткие сообщения.
- Привязка: project_id + task_id.
- Для AI Manager.

## 4. Wizard инициализации МГ (Спираль 7)
- Автоматически при пустом МГ (CVE/IPP).
- Название, типология, вехи, дата первой вехи, чекбокс групп.
- После заполнения — не открывается.

## 5. Согласование МГ (Спираль 7)
- Кнопка «Черновик/Чистовик» → workflow.
- Черновик: CVE/IPP → отправить на согласование (проверка: вехи, группы, задачи, ответственные, сроки).
- Согласование: команда согласует/отклоняет (таблица). CVE/IPP — редактирование недоступно, кнопка «Отозвать».
- Утверждение: если все согласовали — CVE утверждает.
- Чистовик: редактирование разблокировано, причины переноса, серые полосы.

## 6. TimelineController (Спираль 7)
- Единый displayStart/End/dayWidth.
- Учитывает target_start/end.
- Zoom: Day/Week/Month.
- Month: без дней.

## 7. Целевой период (Спираль 7)
- Project → MG → Client.
- displayStart/End не меньше target.

## 8. Явное завершение МГ-задачи из ЛГ (Спираль 7)
- Кнопка «Завершить» в ToolBar ЛГ.
- **Условия активности:**
  - Пользователь — ответственный (право редактирования ЛГ).
  - В ЛГ есть задачи со сроками.
  - Все задачи ЛГ — `status = 1` (Completed).
  - Нет задач без сроков.
  - МГ-задача ещё не завершена (`task.status != 1`).
- **При клике** — диалог подтверждения.
- **При подтверждении** — `POST /{lg_id}/complete` в BUS → `_compute_forecast` + `status = 1` → `POST /internal/forecast-update` в MG → WS snapshot → перерисовка.
- **Автоматическое завершение** (когда все ЛГ-задачи Completed) — **отменено**: BUS `_compute_forecast` всегда `status = 0`. Завершение МГ-задачи — **только явное действие пользователя**.
- **Статус:** правка дана (Порция B), не применена/не проверена.

## 9. Инициализация ERP (Спираль 11)
- BDERE, телефонный справочник, workflow.

## 10. Refresh токена через IPC (Спираль 7)
- **`production_client`:** `TokenRefreshServer` (QLocalServer `engineering_re_token_refresh`). При `{"action":"refresh"}` → `AuthService::refreshToken(currentRefreshToken)`. При `tokenRefreshed` → broadcast `{"action":"token","token":"..."}`. При `sessionExpired` → broadcast `{"action":"error","message":"Session expired"}`.
- **`gantt_editor`:** `TokenRefreshClient` (QLocalSocket). При `401` / `close code 1008` / WS «Token expired» → `requestRefresh()`. При `{"action":"token"}` → `WebSocketClient::onTokenRefreshed(newToken)` → переподключение.
- **`ApiClient` (common):** `401 Token expired` → `m_onTokenExpired()` (callback от `AuthService`) → запрос в очередь → повтор после `tokenRefreshed`.
- **`AuthService`:** `handleTokenExpired` → `refreshToken`. При успехе — `ApiClient::onTokenRefreshed()`. При неудаче — `onTokenRefreshFailed()` → `sessionExpired`.
- **`WebSocketClient`:** `setOnTokenExpired`, `onTokenRefreshed`, `onTokenRefreshFailed`.
- **`ProjectController`:** создание `TokenRefreshClient`, связь `WebSocketClient` ↔ `TokenRefreshClient`.
- **Статус:** собрано, не проверено в рантайме (ПК не актуален, БД не актуальны). Требуется проверка на чистой среде (Сценарий 1: production_client refresh после 15 мин; Сценарий 2: gantt_editor IPC refresh + переподключение WS).

## Требования к разработке
(см. `spirals.md`)
