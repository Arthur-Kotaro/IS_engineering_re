-- ============================================================
-- MG_service: Спираль 7.5 — mg_groups (привязка групп к МГ)
-- ============================================================

CREATE TABLE IF NOT EXISTS mg_groups (
    mg_group_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    mg_id UUID NOT NULL REFERENCES mastergraphs(mg_id) ON DELETE CASCADE,
    group_id VARCHAR(50) NOT NULL REFERENCES task_groups(group_id),
    sort_order INTEGER NOT NULL DEFAULT 0,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    is_deleted BOOLEAN NOT NULL DEFAULT FALSE,
    deleted_at TIMESTAMPTZ NULL,
    deleted_by BIGINT NULL
);

CREATE INDEX IF NOT EXISTS ix_mg_groups_mg_id ON mg_groups(mg_id);
CREATE INDEX IF NOT EXISTS ix_mg_groups_mg_group ON mg_groups(mg_id, group_id);

-- Частичный уникальный индекс: одна активная привязка mg_id+group_id
CREATE UNIQUE INDEX IF NOT EXISTS ux_mg_groups_active
    ON mg_groups(mg_id, group_id)
    WHERE is_deleted = FALSE;
