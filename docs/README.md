# Документация платформы «Engineering:re»

**Версия:** 5.0
**Дата:** 2026-10-12
**Статус:** Спираль 6.6 (Roles & Positions & Departments) в работе.

## О чём этот проект

«Engineering:re» — **платформа** для управления инженерными проектами.
Не ERP для конкретного предприятия. Не отраслевое решение.

Платформа предоставляет универсальные механизмы:
- workflow-страницы, генерируемые UI Composer;
- навигацию и плитки;
- роли, должности, подразделения;
- делегирование и имперсонацию;
- **базовый универсальный бизнес-микросервис**, разворачиваемый
  во множестве одинаковых экземпляров с возможностью
  специализации «на лету».

Бизнес-логика конкретного предприятия (например, автомобильного
инжиниринга ВАЗ/УПП) — **не является целью платформы**.
Она может быть реализована в самом конце, если проект дойдёт
до этого. См. `strategy.md`.

## Структура документации

docs/
├── README.md — этот файл
├── tech-stack.md — стек, архитектура, порты, запуск
├── strategy.md — стратегия: платформа, а не ERP
├── context.md — контекст проекта и автора
├── business-rules.md — 48 ключевых правил предприятия (ВАЗ/УПП)
├── test_users.md — тестовые пользователи
├── history/
│ └── domain-corrections.md — исправленные ошибки домена
├── domain/
│ ├── glossary.md — глоссарий платформы
│ └── enterprise/ — справочник предметной области ВАЗ/УПП
│ ├── README.md
│ ├── glossary.md
│ ├── diagrams.md
│ ├── hierarchy.md
│ ├── administrative-structure.md
│ ├── upp-structure.md
│ ├── positions.md
│ ├── project-team.md
│ ├── master-schedule.md
│ ├── validation-plan.md
│ ├── request-types.md
│ ├── lots.md
│ ├── main-process.md
│ ├── components-supply.md
│ ├── serial-production.md
│ ├── prototype-assembly.md
│ ├── economics.md
│ ├── reporting.md
│ ├── communications.md
│ ├── typical-errors.md
│ ├── prowork-and-techcheck.md
│ ├── project-phases.md
│ ├── project-types.md
│ ├── aves.md
│ └── mule.md
├── roadmap/
│ ├── progress.md — хронология + проекция «Фазы × Спирали»
│ ├── documentation-policy.md — правило ведения документации
│ ├── phase_0_plan.md
│ ├── phase_1_plan.md
│ ├── phase_1_platform_logic.md
│ └── phase_2_plan.md
├── adr/ — Architecture Decision Records
├── architecture/ — обзор и потоки запросов
├── contracts/ — контракты сервисов
└── archive/ — бэкап старых документов

## Ключевые модели платформы

**Модель пользователя:**
- Одна должность (`positions`).
- Несколько ролей (`roles` через `users_roles`).
- Одно подразделение (`departments.dept_id`).
- Иерархия сотрудников — через `departments.parent_dept_id`.

**Модель роли:**
- Роль — производная от `(position, dept)`.
- В JWT: `position` (одна) + `roles` (список).

**Модель проекта:**
- `project_roles` — справочник.
- `project_members.member_id` + `role_code` + `user_id`.
- Права через `project_members` + `projects`.

**Модель плиток:**
- `tiles` + `tiles_by_role` + `tiles_by_position` + `tiles_universal`.
- `admin` — все плитки.

**Базовый универсальный бизнес-МС:**
- Один и тот же код разворачивается во множестве экземпляров.
- Каждый экземпляр может быть доработан независимо.
- Специализация создаётся «на лету», после выхода в эксплуатацию.
- Discovery service находит экземпляры.
- Подробнее — в `roadmap/phase_1_plan.md` и `strategy.md`.

## Правила работы

### Git

- **Одна ветка `main`.** Ветки не используются.
- Коммиты — атомарные, с осмысленным сообщением.
- Перед крупной спиралью — `git tag spiral-N-start`.
- После завершения спирали — `git tag spiral-N-done`.
- **USER_service — отдельный репозиторий** (`git@github.com:Arthur-Kotaro/USER_service.git`).
  Основной репо отслеживает только указатель на коммит. При изменениях — обновлять через `git add USER_service && git commit -m "update USER_service pointer"`.

### Разработка

- Одна порция = одна проверка.
- Новые файлы и перезаписи — через `cat > файл <<'EOF'`.
- Точечные правки в больших файлах — вручную в редакторе.
- `sed`, `awk` — не использовать.

### Формат ADR

- Status: Proposed / Accepted / Deprecated / Superseded.
- Контекст → Решение → Последствия → Ссылки.

### Документация

- Без упрощений. Без выжимок. Без потерь.
- Открытые вопросы остаются там, где описан соответствующий домен.
- Timestamp при обновлении блоков — см. `roadmap/documentation-policy.md`.

## Запуск

См. `tech-stack.md`.

## Спирали и фазы

См. `roadmap/progress.md`.
