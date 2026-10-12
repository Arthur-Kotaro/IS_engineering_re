# Документация ERP «Engineering:re»

**Версия:** 2.0
**Дата обновления:** 2026-09-26
**Статус:** активная разработка

## О системе

**Engineering:re** — ERP-система для управления инженерными проектами.
Ключевой артефакт — **мастерграфик проекта** (диаграмма Ганта).
Управление — через workflow-страницы, генерируемые UI Composer.

## Стек

**Микросервисы:**
- Python 3.12+
- FastAPI, uvicorn
- SQLAlchemy 2.0 (async), asyncpg
- PostgreSQL (SQLite в Navigation Service)
- Redis (blacklist, workflow-сессии, события)
- JWT (HS256), bcrypt
- Nginx (API Gateway, auth_request)

**Клиент:**
- C++20, Qt6 QML
- CMake, Ninja

## Архитектура (кратко)

```

Client (Qt6) → Nginx :8080 → Auth /verify + сервисы
├─ USER         :8000
├─ PROJECT      :8001
├─ MASTERGRAPH  :8003 (план)
├─ NAVIGATION   :8009
├─ AUTH         :8010
├─ DELEGATION   :8011
├─ NOTIFICATION :8012
└─ UI COMPOSER  :8020

```

UI Composer ходит в бизнес-сервисы **напрямую** (минуя nginx) для sources шаблонов.

## Структура документации

```

docs/
├── README.md                    — этот файл
├── adr/                         — Architecture Decision Records
│   ├── 0001-gateway.md
│   ├── 0002-blacklist.md
│   ├── 0003-navigation-storage.md
│   ├── 0004-delegation.md
│   ├── 0005-access-model.md
│   ├── 0006-impersonation.md
│   ├── 0007-workflow.md
│   └── 0008-auth-model.md
├── domain/
│   └── glossary.md              — термины
├── architecture/
│   ├── overview.md              — компоненты, стек
│   └── request-flows.md         — карта хопов
├── contracts/                   — контракты сервисов
│   ├── README.md
│   ├── auth.md
│   ├── user.md
│   ├── project.md
│   ├── delegation.md
│   ├── notification.md
│   └── ui_composer.md
├── roadmap/
│   └── spirals.md               — план и статус спиралей
└── audit/
└── 2026-09-26-erp-audit.md  — сквозной аудит

```

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

## Порты

| Сервис | Порт | Хранилище |
|---|---|---|
| User | 8000 | PostgreSQL |
| Project | 8001 | PostgreSQL |
| PJP (заглушка) | 8002 | — |
| Mastergraphics | 8003 | PostgreSQL |
| PROTO (заглушка) | 8004 | — |
| Navigation | 8009 | SQLite |
| Auth | 8010 | Redis |
| Delegation | 8011 | PostgreSQL |
| Notification | 8012 | PostgreSQL |
| UI Composer | 8020 | Redis (сессии) |
| Nginx Gateway | 8080 | — |

## Redis DB

| DB | Назначение |
|---|---|
| 0 | blacklist (отозванные JWT) |
| 1 | workflow-сессии |
| 2 | кэш (резерв) |
| 3 | user.events (pub/sub) |

## Запуск

```bash
cd /home/kotaro/code/IS_RE_engineering
./start_all.sh -d      # инфраструктура
./start_all.sh -b      # + бизнес-сервисы (PJP, MG, PROTO)
./start_all.sh -f      # + клиент
./stop_all.sh
./restart_all.sh
./status.sh
```

Связанные проекты

· mastergraph-re (/home/kotaro/code/mastergraph-re) — макет редактора мастерграфиков.
  После завершения рефакторинга — интеграция с ERP.

## Спирали

См. `docs/roadmap/spirals.md`.

Текущий статус: Спирали 1–6 завершены. Следующая — Спираль 7 (MG Service).

### Git: дисциплина коммитов

- **Коммит после каждой проверенной порции работы.**
- Минимум — после каждого блока, который запущен и работает.
- Максимум — 3 файла или один шаг спирали на коммит.
- Если работа идёт в 4+ файлах — всё равно коммитить после проверки.
- Тег `spiral-N-start` — перед началом.
- Тег `spiral-N-done` — после завершения.
- **Не накапливать изменения на несколько спиралей.** Это ломает историю.
