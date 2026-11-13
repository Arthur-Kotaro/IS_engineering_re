-- MG_service: частичный уникальный индекс для активных задач по (mg_id, group_id, title)
CREATE UNIQUE INDEX IF NOT EXISTS ux_mg_tasks_active_title
    ON mg_tasks (mg_id, group_id, title)
    WHERE is_deleted = FALSE;
