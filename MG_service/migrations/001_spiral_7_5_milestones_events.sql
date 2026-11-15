-- ============================================================
-- MG_service: Спираль 7.5 — вехи, история, статус МГ, soft delete
-- ============================================================

-- 0. Генератор UUID (должен быть ДО первого использования gen_random_uuid())
CREATE EXTENSION IF NOT EXISTS pgcrypto;

-- 1. mastergraphs: статус МГ, типология, soft delete
ALTER TABLE mastergraphs
    ADD COLUMN IF NOT EXISTS is_approved BOOLEAN NOT NULL DEFAULT FALSE,
    ADD COLUMN IF NOT EXISTS approved_at TIMESTAMPTZ NULL,
    ADD COLUMN IF NOT EXISTS approved_by BIGINT NULL,
    ADD COLUMN IF NOT EXISTS typology_name VARCHAR(50) NULL,
    ADD COLUMN IF NOT EXISTS is_deleted BOOLEAN NOT NULL DEFAULT FALSE,
    ADD COLUMN IF NOT EXISTS deleted_at TIMESTAMPTZ NULL,
    ADD COLUMN IF NOT EXISTS deleted_by BIGINT NULL;

-- 2. mg_tasks: soft delete
ALTER TABLE mg_tasks
    ADD COLUMN IF NOT EXISTS is_deleted BOOLEAN NOT NULL DEFAULT FALSE,
    ADD COLUMN IF NOT EXISTS deleted_at TIMESTAMPTZ NULL,
    ADD COLUMN IF NOT EXISTS deleted_by BIGINT NULL;

-- 3. mg_dependencies: soft delete
ALTER TABLE mg_dependencies
    ADD COLUMN IF NOT EXISTS is_deleted BOOLEAN NOT NULL DEFAULT FALSE,
    ADD COLUMN IF NOT EXISTS deleted_at TIMESTAMPTZ NULL,
    ADD COLUMN IF NOT EXISTS deleted_by BIGINT NULL;

-- 4. task_groups: soft delete
ALTER TABLE task_groups
    ADD COLUMN IF NOT EXISTS is_deleted BOOLEAN NOT NULL DEFAULT FALSE,
    ADD COLUMN IF NOT EXISTS deleted_at TIMESTAMPTZ NULL,
    ADD COLUMN IF NOT EXISTS deleted_by BIGINT NULL;

-- 5. holidays: soft delete
ALTER TABLE holidays
    ADD COLUMN IF NOT EXISTS is_deleted BOOLEAN NOT NULL DEFAULT FALSE,
    ADD COLUMN IF NOT EXISTS deleted_at TIMESTAMPTZ NULL,
    ADD COLUMN IF NOT EXISTS deleted_by BIGINT NULL;

-- 6. mg_milestones: вехи МГ
CREATE TABLE IF NOT EXISTS mg_milestones (
    milestone_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    mg_id UUID NOT NULL REFERENCES mastergraphs(mg_id) ON DELETE CASCADE,
    abbreviation VARCHAR(20) NOT NULL,
    full_name VARCHAR(200) NOT NULL,
    tooltip TEXT NULL,
    week_offset INTEGER NOT NULL DEFAULT 0,
    planned_date DATE NOT NULL,
    actual_date DATE NULL,
    status INTEGER NOT NULL DEFAULT 0,          -- 0=planned,1=completed,2=rescheduled
    sort_order INTEGER NOT NULL DEFAULT 0,
    version INTEGER NOT NULL DEFAULT 1,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    is_deleted BOOLEAN NOT NULL DEFAULT FALSE,
    deleted_at TIMESTAMPTZ NULL,
    deleted_by BIGINT NULL
);

CREATE INDEX IF NOT EXISTS ix_mg_milestones_mg_id ON mg_milestones(mg_id);
CREATE INDEX IF NOT EXISTS ix_mg_milestones_mg_sort ON mg_milestones(mg_id, sort_order);

-- 7. mg_events: универсальная история событий
CREATE TABLE IF NOT EXISTS mg_events (
    event_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    mg_id UUID NOT NULL REFERENCES mastergraphs(mg_id) ON DELETE CASCADE,
    entity_type VARCHAR(20) NOT NULL,           -- task | milestone | mastergraph | dependency
    entity_id UUID NOT NULL,
    event_type VARCHAR(30) NOT NULL,            -- create | update | delete | reschedule | status_change | complete | approve
    field VARCHAR(50) NULL,
    old_value JSONB NULL,
    new_value JSONB NULL,
    reason TEXT NULL,
    actor_id BIGINT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS ix_mg_events_mg_created ON mg_events(mg_id, created_at);
CREATE INDEX IF NOT EXISTS ix_mg_events_entity ON mg_events(entity_type, entity_id);

-- 8. mg_status_history: смены статуса МГ
CREATE TABLE IF NOT EXISTS mg_status_history (
    id BIGSERIAL PRIMARY KEY,
    mg_id UUID NOT NULL REFERENCES mastergraphs(mg_id) ON DELETE CASCADE,
    old_status VARCHAR(20) NOT NULL,
    new_status VARCHAR(20) NOT NULL,
    reason TEXT NULL,
    actor_id BIGINT NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS ix_mg_status_history_mg_created ON mg_status_history(mg_id, created_at);
