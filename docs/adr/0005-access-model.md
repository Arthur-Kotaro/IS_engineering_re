# ADR-0005: Модель прав на проект (переписан)

## Статус
Accepted

## Дата
2026-10-09

## Контекст

**Предыдущая версия этого ADR (2026-09-26) содержала выдуманную модель:**
- Роли `owner`, `manager`, `editor`, `viewer` — **не существуют** в реальной системе.
- Права `edit_project`, `delete_project` — **не отражают** бизнес.
- Композиция `direct OR subordinate OR delegated` — **не реализована**.

Эта версия — правильная.

## Модель

### Справочник проектных ролей (`project_roles`)

10 ролей:
- `chief_engineer` — Главный инженер.
- `prototype_pm` — Руководитель проекта по прототипам (прото-РП).
- `industrialization_manager` — Менеджер по индустриализации.
- `architect` — Архитектор.
- `validator` — Специалист по валидации.
- `test_specialist` — Специалист по испытаниям.
- `planning_engineer` — Инженер по планированию продукта (IPP).
- `quality_engineer` — Инженер по качеству.
- `project_economist` — Экономист проекта.
- `project_member` — Плейсхолдер.

### Участники проекта (`project_members`)

- `member_id` (surrogate PK).
- `project_id` (FK).
- `role_code` (FK на `project_roles`).
- `user_id`.
- `UNIQUE (project_id, user_id)`.

**Один человек — одна роль в проекте.**

### Права

| Право | Кто |
|---|---|
| `view_project` | Все члены команды |
| `edit_mastergraphic` | `chief_engineer` и `planning_engineer` проекта |
| `manage_members` | `chief_engineer` и `planning_engineer` |
| `approve_members` | `chief_engineer` |
| `edit_project` | `chief_engineer` |
| `delete_project` | Не реализовано |

### Особый случай: `admin`

Пользователь с ролью `admin` (или `is_super_admin`) имеет все права на все проекты.

### Проверка

```python
async def check_access(project_id, user_id, permission):
    project = get_project(project_id)
    user = get_user(user_id)  # из User Service

    if "admin" in user.roles or user.is_super_admin:
        return allowed

    is_ce = project.chief_engineer_id == user_id
    is_pe = project.planning_engineer_id == user_id
    is_member = member_exists(project_id, user_id)

    if permission == "view_project":
        return is_member or is_ce or is_pe
    if permission == "edit_mastergraphic":
        return is_ce or is_pe
    if permission == "manage_members":
        return is_ce or is_pe
    if permission == "approve_members":
        return is_ce
    if permission == "edit_project":
        return is_ce
    return False

Что удалено

    Роли owner, manager, editor, viewer.

    Композиция direct OR subordinate OR delegated.

    Поля access_via, via_user_id в ответах.

    Поддержка subordinate_access и delegation_access.

Что осталось

    chief_engineer_id и planning_engineer_id в таблице projects — для быстрого доступа к ролям главы команды.

Ссылки

    ADR-0010: Роли, должности, подразделения.

    docs/domain/enterprise/administrative-structure.md.
