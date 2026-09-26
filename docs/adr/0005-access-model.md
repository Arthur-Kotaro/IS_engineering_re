# ADR-0005: Модель прав на проект

## Статус
Proposed

## Дата
2026-09-26

## Контекст

Права на проект сейчас декоративны: ProjectMember.role — строка, _check_owner — только created_by.

## Решение

Project Service — единственный источник прав на проект.

Композиция:

    effective_permission(user, project, action) =
        direct_permission(user, project, action)
        OR subordinate_permission(user, project, action)
        OR delegated_permission(user, project, action)

- Direct: project_members с явными правами.
- Subordinate: начальник действует от имени подчинённого (через иерархию из User).
- Delegated: через активную делегацию из Delegation.

## Формат прав

Вариант 1 (проще): project_role = owner | manager | editor | viewer. Из роли выводятся права.

Вариант 2 (гибче): явные флаги can_view, can_edit_mastergraphic, can_edit_project, can_manage_members.

Решение: начать с варианта 1, перейти на 2 при необходимости.

## Источник иерархии

User Service, кэш в Project Service.

## Действие от имени

Аудит: acting_user_id + on_behalf_of.

## Что должен уметь Project Service

- GET /internal/projects/list-with-access?user_id=&permission=
- GET /internal/projects/{id}/check-access?user_id=&permission=
- Внутри — запросы к User (иерархия) и Delegation (делегации).
