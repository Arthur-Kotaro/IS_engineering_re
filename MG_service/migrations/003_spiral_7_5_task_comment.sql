-- MG_service: task comment
ALTER TABLE mg_tasks ADD COLUMN IF NOT EXISTS comment VARCHAR(1000) NULL;
