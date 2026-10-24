# ADR 0015. Mastergraphics Service (MG Service) и BUS

**Дата:** 2026-10-25
**Status:** Accepted

## Контекст

Нужен сервис для управления Мастерграфиками (МГ). Также нужен механизм для Локальных графиков (ЛГ).

## Решение

Создан **MG Service** (порт 8003) — отдельный сервис.

- **БД:** PostgreSQL (`mastergraph_service`).
- **Таблицы:** `mastergraphs`, `task_groups`, `mg_tasks`, `mg_dependencies`.
- **Права:** проверяются через `project_service /api/v1/projects/{id}/check-access`.
- **Группы задач:** 6 для MVP (`documentation`, `architecture`, `supplier_nomination`, `prototypes`, `validation`, `calibration`).
- **Связь с BUS:** через `bus_instances.yaml` (group_id → url).

**BUS (Base Universal Service)** — управляет ЛГ. По одному экземпляру на группу задач. Для MVP — заглушки.

**WebSocket** для real-time синхронизации (позже).

## Последствия

- MG Service — отдельный сервис с собственной БД.
- ЛГ хранятся в BUS, не в MG.
- MG и BUS общаются через REST (MVP) и WebSocket (позже).
