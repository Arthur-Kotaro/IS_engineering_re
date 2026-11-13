-- ============================================================
-- BUS_service: Спираль 7.5 — lg_events, soft delete, local_graph_state
-- ============================================================

-- 1. local_graphs: soft delete + local_graph_state
ALTER TABLE local_graphs ADD COLUMN is_deleted INTEGER NOT NULL DEFAULT 0;
ALTER TABLE local_graphs ADD COLUMN deleted_at DATETIME NULL;
ALTER TABLE local_graphs ADD COLUMN deleted_by BIGINT NULL;
ALTER TABLE local_graphs ADD COLUMN local_graph_state INTEGER NOT NULL DEFAULT 0;

-- 2. lg_tasks: soft delete + timestamps
ALTER TABLE lg_tasks ADD COLUMN is_deleted INTEGER NOT NULL DEFAULT 0;
ALTER TABLE lg_tasks ADD COLUMN deleted_at DATETIME NULL;
ALTER TABLE lg_tasks ADD COLUMN deleted_by BIGINT NULL;
ALTER TABLE lg_tasks ADD COLUMN created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP;
ALTER TABLE lg_tasks ADD COLUMN updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP;

-- 3. lg_dependencies: version + soft delete + created_at
ALTER TABLE lg_dependencies ADD COLUMN version INTEGER NOT NULL DEFAULT 1;
ALTER TABLE lg_dependencies ADD COLUMN is_deleted INTEGER NOT NULL DEFAULT 0;
ALTER TABLE lg_dependencies ADD COLUMN deleted_at DATETIME NULL;
ALTER TABLE lg_dependencies ADD COLUMN deleted_by BIGINT NULL;
ALTER TABLE lg_dependencies ADD COLUMN created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP;

-- 4. lg_events
CREATE TABLE IF NOT EXISTS lg_events (
    event_id TEXT PRIMARY KEY,
    lg_id TEXT NOT NULL,
    entity_type TEXT NOT NULL,
    entity_id TEXT NOT NULL,
    event_type TEXT NOT NULL,
    field TEXT NULL,
    old_value TEXT NULL,
    new_value TEXT NULL,
    reason TEXT NULL,
    actor_id BIGINT NULL,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(lg_id) REFERENCES local_graphs(lg_id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS ix_lg_events_lg_created ON lg_events(lg_id, created_at);
CREATE INDEX IF NOT EXISTS ix_lg_events_entity ON lg_events(entity_type, entity_id);

-- 5. Очистка мусорных задач "Новая задача"
DELETE FROM lg_tasks
WHERE title = 'Новая задача'
  AND start_date IS NULL
  AND end_date IS NULL
  AND (responsible_id IS NULL OR responsible_id = 0);

-- 6. Перенос local_graph_state
UPDATE local_graphs
SET local_graph_state = CASE
    WHEN responsible_id IS NULL THEN 0
    WHEN EXISTS (
        SELECT 1 FROM lg_tasks t
        WHERE t.lg_id = local_graphs.lg_id
          AND t.is_deleted = 0
          AND t.start_date IS NOT NULL
          AND t.end_date IS NOT NULL
    ) THEN 2
    ELSE 1
END;
