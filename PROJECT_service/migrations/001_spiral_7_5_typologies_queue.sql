-- ============================================================
-- Project Service: Спираль 7.5 — project_type, typology, очередь MG
-- ============================================================

-- 1. projects: project_type, typology
ALTER TABLE projects
    ADD COLUMN IF NOT EXISTS project_type VARCHAR(50) NULL,
    ADD COLUMN IF NOT EXISTS typology VARCHAR(50) NULL;

-- Для существующих — default
UPDATE projects SET project_type = 'vehicle' WHERE project_type IS NULL;
UPDATE projects SET typology = 'brother' WHERE typology IS NULL;

ALTER TABLE projects ALTER COLUMN project_type SET NOT NULL;
ALTER TABLE projects ALTER COLUMN typology SET NOT NULL;

-- 2. project_types: справочник типов проектов
CREATE TABLE IF NOT EXISTS project_types (
    code VARCHAR(50) PRIMARY KEY,
    name_ru VARCHAR(100) NOT NULL,
    name_en VARCHAR(100) NOT NULL,
    description_ru TEXT NULL,
    sort_order INTEGER NOT NULL DEFAULT 0,
    is_active BOOLEAN NOT NULL DEFAULT TRUE
);

-- 3. typologies: справочник типологий (привязан к project_type)
CREATE TABLE IF NOT EXISTS typologies (
    code VARCHAR(50) PRIMARY KEY,
    project_type VARCHAR(50) NOT NULL REFERENCES project_types(code),
    name_ru VARCHAR(100) NOT NULL,
    name_en VARCHAR(100) NOT NULL,
    description_ru TEXT NULL,
    sort_order INTEGER NOT NULL DEFAULT 0,
    is_active BOOLEAN NOT NULL DEFAULT TRUE
);

CREATE INDEX IF NOT EXISTS ix_typologies_project_type ON typologies(project_type);

-- 4. mg_creation_queue: очередь уведомлений MG при создании проекта
CREATE TABLE IF NOT EXISTS mg_creation_queue (
    id BIGSERIAL PRIMARY KEY,
    project_id BIGINT NOT NULL REFERENCES projects(project_id) ON DELETE CASCADE,
    project_type VARCHAR(50) NOT NULL,
    typology_name VARCHAR(50) NOT NULL,
    cve_id BIGINT NOT NULL,
    ipp_id BIGINT NOT NULL,
    attempt INTEGER NOT NULL DEFAULT 0,
    max_attempts INTEGER NOT NULL DEFAULT 5,
    next_retry_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    status VARCHAR(20) NOT NULL DEFAULT 'pending',
    last_error TEXT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS ix_mg_queue_status_retry ON mg_creation_queue(status, next_retry_at);
CREATE INDEX IF NOT EXISTS ix_mg_queue_project ON mg_creation_queue(project_id);

-- 5. Seed: project_types
INSERT INTO project_types (code, name_ru, name_en, description_ru, sort_order)
VALUES
    ('vehicle', 'Автомобильный проект', 'Vehicle Project', 'Разработка автомобиля', 1),
    ('power_unit', 'Проект силового агрегата', 'Power Unit Project', 'Разработка ДВС/КПП', 2)
ON CONFLICT (code) DO NOTHING;

-- 6. Seed: typologies
INSERT INTO typologies (code, project_type, name_ru, name_en, description_ru, sort_order)
VALUES
    ('brother', 'vehicle', 'Автомобильный проект (братский)', 'Vehicle Project (Brother)', 'Братский проект', 1),
    ('mother', 'vehicle', 'Автомобильный проект (материнский)', 'Vehicle Project (Mother)', 'Материнский проект', 2),
    ('child', 'vehicle', 'Автомобильный подпроект (дочерний)', 'Vehicle Project (Child)', 'Дочерний проект', 3),
    ('power_unit', 'power_unit', 'Проект СА (ДВС/КПП)', 'Power Unit (Engine/Transmission)', 'Силовой агрегат', 1)
ON CONFLICT (code) DO NOTHING;
