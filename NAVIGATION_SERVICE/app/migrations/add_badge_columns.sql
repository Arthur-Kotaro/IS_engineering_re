-- add_badge_columns.sql
-- Добавляем колонки для пузырьков

ALTER TABLE tiles ADD COLUMN badge_enabled BOOLEAN DEFAULT FALSE;
ALTER TABLE tiles ADD COLUMN badge_endpoint VARCHAR(200) DEFAULT NULL;
