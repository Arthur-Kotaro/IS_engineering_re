-- ============================================================
-- init_full_test_data.sql
-- Полное заполнение тестовыми данными (все роли + пользователи)
-- ============================================================

-- ============================================================
-- 1. РОЛИ (все из матрицы)
-- ============================================================
INSERT INTO roles (role_title, role_title_ru, role_code) VALUES
('admin', 'Администратор системы', 'ADMIN'),
('supervisor', 'Начальник УПП', 'SUPER'),
('chief_engineer', 'Главный инженер', 'CVE'),
('industrialization_manager', 'Менеджер по индустриализации', 'IM'),
('proto_pm', 'Руководитель проекта по прототипам', 'CPM'),
('architect', 'Архитектор', 'ARCH'),
('validator', 'Специалист по валидации', 'PSW'),
('test_specialist', 'Специалист по испытаниям', 'TEST'),
('planning_engineer', 'Инженер по планированию', 'IPP'),
('pfe_ist', 'PFE/IST', 'PFE'),
('proctech', 'Проработчик состава', 'PROC'),
('proto_technologist', 'Прото-технолог', 'TECH'),
('proto_planner', 'Прото-плановик', 'PLAN'),
('proto_master', 'Прото-мастер', 'MAST'),
('proto_controller', 'Прото-контролёр', 'CONT'),
('proto_logistician', 'Прото-логист', 'LOGI'),
('proto_purchaser', 'Прото-закупщик', 'PURC'),
('proto_economist', 'Прото-экономист', 'ECON'),
('shop_head', 'Прото-начальник цеха', 'SHOP'),
('kto_head', 'Прото-начальник КТО', 'KTO'),
('design_pm_senior', 'РП по разработке (старший)', 'IST'),
('design_pm_junior', 'РП по разработке (младший)', 'PFE')
ON CONFLICT (role_title) DO NOTHING;

-- ============================================================
-- 2. ПОДРАЗДЕЛЕНИЯ
-- ============================================================
INSERT INTO department (dept_code, dept_name, parent_dept_code) VALUES
('UPP', 'Управление производства прототипов', NULL),
('SHOP_ASSEMBLY', 'Цех сборки прототипов', 'UPP'),
('SHOP_BODY', 'Цех изготовления кузова и оснастки', 'UPP'),
('KTO', 'Конструкторско-технологический отдел', 'UPP'),
('BUREAU_MECH', 'Бюро механообработки', 'KTO'),
('BUREAU_WELD', 'Бюро сварки и сборки', 'KTO'),
('BUREAU_CONTROL', 'Бюро контроля (проработчики)', 'KTO'),
('DEP_QUALITY', 'Отдел качества', 'UPP'),
('DEP_ECONOMISTS', 'Отдел экономистов', 'UPP'),
('ZAKUP', 'Отдел закупок', NULL),
('LOG', 'Отдел логистики', NULL),
('SKLAD', 'Склады', NULL),
('HR', 'Отдел кадров', NULL)
ON CONFLICT (dept_code) DO NOTHING;

-- ============================================================
-- 3. ПОЛЬЗОВАТЕЛИ
-- ============================================================
-- Пароли:
--   Суперадмин: SuperAdminPass123!
--   Руководители: LeaderPass456!
--   Остальные: TestPassword123!

-- Хеш для SuperAdminPass123!:
-- $2b$12$abcdefghijklmnopqrstuvwxyz1234567890abcdefghijklmnopqrstuv
-- Хеш для LeaderPass456!:
-- $2b$12$KIXB4Qk6hGqNFPgK9SX3M.N7b1yNk9qyE2b2B6S7Y8hH9iJ0kL1M2N
-- Хеш для TestPassword123!:
-- $2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly

-- 3.1 СУПЕРАДМИН
INSERT INTO users (user_name, full_name, email, password_hash, is_super_admin, created_at, updated_at) VALUES
('super.admin', 'Системный администратор', 'super.admin@company.com', '$2b$12$abcdefghijklmnopqrstuvwxyz1234567890abcdefghijklmnopqrstuv', TRUE, NOW(), NOW())
ON CONFLICT (user_name) DO NOTHING;

-- 3.2 РУКОВОДИТЕЛИ (LeaderPass456!)
INSERT INTO users (user_name, full_name, email, password_hash, dept_code, is_super_admin, created_at, updated_at) VALUES
('supervisor.ivanov', 'Иванов Иван Петрович', 'ivanov@upp.com', '$2b$12$KIXB4Qk6hGqNFPgK9SX3M.N7b1yNk9qyE2b2B6S7Y8hH9iJ0kL1M2N', 'UPP', FALSE, NOW(), NOW()),
('chief_engineer.smirnov', 'Смирнов Александр Владимирович', 'smirnov@company.com', '$2b$12$KIXB4Qk6hGqNFPgK9SX3M.N7b1yNk9qyE2b2B6S7Y8hH9iJ0kL1M2N', NULL, FALSE, NOW(), NOW()),
('industrialization.kuznetsov', 'Кузнецов Дмитрий Сергеевич', 'kuznetsov@company.com', '$2b$12$KIXB4Qk6hGqNFPgK9SX3M.N7b1yNk9qyE2b2B6S7Y8hH9iJ0kL1M2N', NULL, FALSE, NOW(), NOW()),
('proto_pm.volkov', 'Волков Алексей Михайлович', 'volkov@upp.com', '$2b$12$KIXB4Qk6hGqNFPgK9SX3M.N7b1yNk9qyE2b2B6S7Y8hH9iJ0kL1M2N', 'UPP', FALSE, NOW(), NOW()),
('proto_pm.novikov', 'Новиков Андрей Николаевич', 'novikov@upp.com', '$2b$12$KIXB4Qk6hGqNFPgK9SX3M.N7b1yNk9qyE2b2B6S7Y8hH9iJ0kL1M2N', 'UPP', FALSE, NOW(), NOW()),
('proto_pm.orlov', 'Орлов Игорь Васильевич', 'orlov@upp.com', '$2b$12$KIXB4Qk6hGqNFPgK9SX3M.N7b1yNk9qyE2b2B6S7Y8hH9iJ0kL1M2N', 'UPP', FALSE, NOW(), NOW()),
('design_pm_senior.vasin', 'Васин Сергей Леонидович', 'vasin@design.com', '$2b$12$KIXB4Qk6hGqNFPgK9SX3M.N7b1yNk9qyE2b2B6S7Y8hH9iJ0kL1M2N', NULL, FALSE, NOW(), NOW()),
('design_pm_junior.belov', 'Белов Олег Евгеньевич', 'belov@design.com', '$2b$12$KIXB4Qk6hGqNFPgK9SX3M.N7b1yNk9qyE2b2B6S7Y8hH9iJ0kL1M2N', NULL, FALSE, NOW(), NOW()),
('shop_assembly.kovalchuk', 'Ковальчук Петр Григорьевич', 'kovalchuk@shop.com', '$2b$12$KIXB4Qk6hGqNFPgK9SX3M.N7b1yNk9qyE2b2B6S7Y8hH9iJ0kL1M2N', 'SHOP_ASSEMBLY', FALSE, NOW(), NOW()),
('shop_body.melnik', 'Мельник Иван Степанович', 'melnik@shop.com', '$2b$12$KIXB4Qk6hGqNFPgK9SX3M.N7b1yNk9qyE2b2B6S7Y8hH9iJ0kL1M2N', 'SHOP_BODY', FALSE, NOW(), NOW()),
('kto_head.belousov', 'Белоусов Павел Николаевич', 'belousov@kto.com', '$2b$12$KIXB4Qk6hGqNFPgK9SX3M.N7b1yNk9qyE2b2B6S7Y8hH9iJ0kL1M2N', 'KTO', FALSE, NOW(), NOW()),
('economist_head.smirnova', 'Смирнова Екатерина Васильевна', 'smirnova@ec.com', '$2b$12$KIXB4Qk6hGqNFPgK9SX3M.N7b1yNk9qyE2b2B6S7Y8hH9iJ0kL1M2N', 'DEP_ECONOMISTS', FALSE, NOW(), NOW()),
('quality_head.nikolaev', 'Николаев Юрий Сергеевич', 'nikolaev@qual.com', '$2b$12$KIXB4Qk6hGqNFPgK9SX3M.N7b1yNk9qyE2b2B6S7Y8hH9iJ0kL1M2N', 'DEP_QUALITY', FALSE, NOW(), NOW()),
('purchaser_head.petrov', 'Петров Алексей Владимирович', 'petrov@zakup.com', '$2b$12$KIXB4Qk6hGqNFPgK9SX3M.N7b1yNk9qyE2b2B6S7Y8hH9iJ0kL1M2N', 'ZAKUP', FALSE, NOW(), NOW()),
('logistician_head.ivanova', 'Иванова Мария Николаевна', 'ivanova@log.com', '$2b$12$KIXB4Qk6hGqNFPgK9SX3M.N7b1yNk9qyE2b2B6S7Y8hH9iJ0kL1M2N', 'LOG', FALSE, NOW(), NOW()),
('hr_head.sidorova', 'Сидорова Татьяна Ивановна', 'sidorova@hr.com', '$2b$12$KIXB4Qk6hGqNFPgK9SX3M.N7b1yNk9qyE2b2B6S7Y8hH9iJ0kL1M2N', 'HR', FALSE, NOW(), NOW())
ON CONFLICT (user_name) DO NOTHING;

-- 3.3 СПЕЦИАЛИСТЫ (TestPassword123!)
INSERT INTO users (user_name, full_name, email, password_hash, dept_code, is_super_admin, created_at, updated_at) VALUES
('architect.1', 'Константинов Андрей Валерьевич', 'konstantinov@company.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', NULL, FALSE, NOW(), NOW()),
('architect.2', 'Леонова Мария Геннадьевна', 'leonova@company.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', NULL, FALSE, NOW(), NOW()),
('architect.3', 'Медведев Сергей Васильевич', 'medvedev@company.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', NULL, FALSE, NOW(), NOW()),
('pfe.1', 'Пономарев Александр Игоревич', 'ponomarev@pfe.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', NULL, FALSE, NOW(), NOW()),
('pfe.2', 'Розова Оксана Сергеевна', 'rozova@pfe.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', NULL, FALSE, NOW(), NOW()),
('pfe.3', 'Соловьев Владислав Николаевич', 'solovyev@pfe.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', NULL, FALSE, NOW(), NOW()),
('validator.1', 'Крылова Елена Павловна', 'krylova@valid.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', NULL, FALSE, NOW(), NOW()),
('validator.2', 'Морозов Денис Викторович', 'morozov@valid.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', NULL, FALSE, NOW(), NOW()),
('test.1', 'Григорьев Денис Павлович', 'grigoriev@test.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', NULL, FALSE, NOW(), NOW()),
('test.2', 'Фролова Екатерина Игоревна', 'frolova@test.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', NULL, FALSE, NOW(), NOW()),
('planner.1', 'Никифорова Анна Владимировна', 'nikiforova@plan.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', NULL, FALSE, NOW(), NOW()),
('planner.2', 'Тарасов Илья Николаевич', 'tarasov@plan.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', NULL, FALSE, NOW(), NOW()),
('planner.proto.1', 'Фомина Татьяна Сергеевна', 'fomina@plan.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'UPP', FALSE, NOW(), NOW()),
('planner.proto.2', 'Шишкин Алексей Петрович', 'shishkin@plan.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'UPP', FALSE, NOW(), NOW()),
('proctech.1', 'Поляков Александр Сергеевич', 'polyakov@proctech.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'BUREAU_CONTROL', FALSE, NOW(), NOW()),
('proctech.2', 'Сорокина Ольга Владимировна', 'sorokina@proctech.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'BUREAU_CONTROL', FALSE, NOW(), NOW()),
('proctech.3', 'Беляев Роман Иванович', 'belyaev@proctech.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'BUREAU_CONTROL', FALSE, NOW(), NOW()),
('technologist.1', 'Андреев Антон Сергеевич', 'andreev@tech.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'BUREAU_MECH', FALSE, NOW(), NOW()),
('technologist.2', 'Максимов Денис Иванович', 'maximov@tech.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'BUREAU_WELD', FALSE, NOW(), NOW()),
('technologist.3', 'Орлова Елена Петровна', 'orlova@tech.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'BUREAU_MECH', FALSE, NOW(), NOW()),
('technologist.4', 'Крылов Михаил Викторович', 'krylov@tech.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'BUREAU_WELD', FALSE, NOW(), NOW()),
('technologist.5', 'Никитин Павел Геннадьевич', 'nikitin@tech.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'KTO', FALSE, NOW(), NOW()),
('controller.1', 'Алексеев Владимир Сергеевич', 'alekseev@cont.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'DEP_QUALITY', FALSE, NOW(), NOW()),
('controller.2', 'Борисова Екатерина Андреевна', 'borisova@cont.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'DEP_QUALITY', FALSE, NOW(), NOW()),
('controller.3', 'Васильев Игорь Петрович', 'vasiliev@cont.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'DEP_QUALITY', FALSE, NOW(), NOW()),
('economist.1', 'Захарова Ольга Сергеевна', 'zakharova@ec.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'DEP_ECONOMISTS', FALSE, NOW(), NOW()),
('economist.2', 'Ильин Алексей Викторович', 'ilin@ec.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'DEP_ECONOMISTS', FALSE, NOW(), NOW()),
('economist.3', 'Киселева Татьяна Владимировна', 'kiseleva@ec.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'DEP_ECONOMISTS', FALSE, NOW(), NOW()),
('master.assem.1', 'Воробьев Илья Сергеевич', 'vorobyev1@master.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'SHOP_ASSEMBLY', FALSE, NOW(), NOW()),
('master.assem.2', 'Ермолова Наталья Петровна', 'ermolova@master.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'SHOP_ASSEMBLY', FALSE, NOW(), NOW()),
('master.assem.3', 'Исаев Константин Викторович', 'isaev@master.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'SHOP_ASSEMBLY', FALSE, NOW(), NOW()),
('master.body.1', 'Савченко Виталий Николаевич', 'savchenko@master.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'SHOP_BODY', FALSE, NOW(), NOW()),
('master.body.2', 'Ткачева Ирина Владимировна', 'tkacheva@master.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'SHOP_BODY', FALSE, NOW(), NOW()),
('master.body.3', 'Федоров Роман Олегович', 'fedorov@master.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'SHOP_BODY', FALSE, NOW(), NOW()),
('logistician.1', 'Федосеев Александр Сергеевич', 'fedoseev@log.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'LOG', FALSE, NOW(), NOW()),
('logistician.2', 'Харитонова Елена Юрьевна', 'kharitonova@log.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'LOG', FALSE, NOW(), NOW()),
('logistician.3', 'Цветков Владимир Петрович', 'tsvetkov@log.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'LOG', FALSE, NOW(), NOW()),
('purchaser.1', 'Павлов Юрий Николаевич', 'pavlov@pur.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'ZAKUP', FALSE, NOW(), NOW()),
('purchaser.2', 'Романова Оксана Викторовна', 'romanova@pur.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'ZAKUP', FALSE, NOW(), NOW()),
('purchaser.3', 'Сергеев Иван Михайлович', 'sergeev@pur.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyKb6Q8wH6Ly', 'ZAKUP', FALSE, NOW(), NOW())
ON CONFLICT (user_name) DO NOTHING;

-- ============================================================
-- 4. НАЗНАЧЕНИЕ РОЛЕЙ
-- ============================================================
DO $$
DECLARE
    u RECORD;
    r RECORD;
BEGIN
    -- Суперадмин
    FOR u IN SELECT user_id FROM users WHERE user_name = 'super.admin'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'admin'
        LOOP INSERT INTO users_roles (user_id, role_id) VALUES (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Начальник УПП
    FOR u IN SELECT user_id FROM users WHERE user_name = 'supervisor.ivanov'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'supervisor'
        LOOP INSERT INTO users_roles (user_id, role_id) VALUES (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Главный инженер
    FOR u IN SELECT user_id FROM users WHERE user_name = 'chief_engineer.smirnov'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'chief_engineer'
        LOOP INSERT INTO users_roles (user_id, role_id) VALUES (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Менеджер по индустриализации
    FOR u IN SELECT user_id FROM users WHERE user_name = 'industrialization.kuznetsov'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'industrialization_manager'
        LOOP INSERT INTO users_roles (user_id, role_id) VALUES (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- РП по прототипам (все 3)
    FOR u IN SELECT user_id FROM users WHERE user_name LIKE 'proto_pm.%'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'proto_pm'
        LOOP INSERT INTO users_roles (user_id, role_id) VALUES (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- РП по разработке
    FOR u IN SELECT user_id FROM users WHERE user_name IN ('design_pm_senior.vasin', 'design_pm_junior.belov')
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = CASE
            WHEN u.user_name = 'design_pm_senior.vasin' THEN 'design_pm_senior'
            ELSE 'design_pm_junior'
        END
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Архитекторы
    FOR u IN SELECT user_id FROM users WHERE user_name LIKE 'architect.%'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'architect'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- PFE/IST
    FOR u IN SELECT user_id FROM users WHERE user_name LIKE 'pfe.%'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'pfe_ist'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Валидаторы
    FOR u IN SELECT user_id FROM users WHERE user_name LIKE 'validator.%'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'validator'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Специалисты по испытаниям
    FOR u IN SELECT user_id FROM users WHERE user_name LIKE 'test.%'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'test_specialist'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Инженеры по планированию
    FOR u IN SELECT user_id FROM users WHERE user_name IN ('planner.1', 'planner.2')
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'planning_engineer'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Прото-плановики
    FOR u IN SELECT user_id FROM users WHERE user_name LIKE 'planner.proto.%'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'proto_planner'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Проработчики
    FOR u IN SELECT user_id FROM users WHERE user_name LIKE 'proctech.%'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'proctech'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Технологи
    FOR u IN SELECT user_id FROM users WHERE user_name LIKE 'technologist.%'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'proto_technologist'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Контролеры
    FOR u IN SELECT user_id FROM users WHERE user_name LIKE 'controller.%'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'proto_controller'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Экономисты
    FOR u IN SELECT user_id FROM users WHERE user_name LIKE 'economist.%' AND user_name NOT LIKE 'economist_head%'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'proto_economist'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Начальник экономистов
    FOR u IN SELECT user_id FROM users WHERE user_name = 'economist_head.smirnova'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'proto_economist'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Мастера
    FOR u IN SELECT user_id FROM users WHERE user_name LIKE 'master.%'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'proto_master'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Логисты
    FOR u IN SELECT user_id FROM users WHERE user_name LIKE 'logistician.%' AND user_name NOT LIKE 'logistician_head%'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'proto_logistician'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Начальник логистики
    FOR u IN SELECT user_id FROM users WHERE user_name = 'logistician_head.ivanova'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'proto_logistician'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Закупщики
    FOR u IN SELECT user_id FROM users WHERE user_name LIKE 'purchaser.%' AND user_name NOT LIKE 'purchaser_head%'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'proto_purchaser'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Начальник закупок
    FOR u IN SELECT user_id FROM users WHERE user_name = 'purchaser_head.petrov'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'proto_purchaser'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Начальники цехов
    FOR u IN SELECT user_id FROM users WHERE user_name IN ('shop_assembly.kovalchuk', 'shop_body.melnik')
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'shop_head'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Начальник КТО
    FOR u IN SELECT user_id FROM users WHERE user_name = 'kto_head.belousov'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'kto_head'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- Начальник качества
    FOR u IN SELECT user_id FROM users WHERE user_name = 'quality_head.nikolaev'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'proto_controller'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;

    -- HR
    FOR u IN SELECT user_id FROM users WHERE user_name = 'hr_head.sidorova'
    LOOP
        FOR r IN SELECT role_id FROM roles WHERE role_title = 'admin'
        LOOP INSERT INTO users_roles (u.user_id, r.role_id) ON CONFLICT DO NOTHING; END LOOP;
    END LOOP;
END $$;

-- ============================================================
-- 5. ИЕРАРХИЯ (head_id)
-- ============================================================
UPDATE users SET head_id = (SELECT user_id FROM users WHERE user_name = 'supervisor.ivanov')
WHERE user_name IN (
    'chief_engineer.smirnov',
    'industrialization.kuznetsov',
    'proto_pm.volkov',
    'proto_pm.novikov',
    'proto_pm.orlov',
    'shop_assembly.kovalchuk',
    'shop_body.melnik',
    'kto_head.belousov',
    'quality_head.nikolaev',
    'economist_head.smirnova',
    'purchaser_head.petrov',
    'logistician_head.ivanova',
    'hr_head.sidorova'
);

UPDATE users SET head_id = (SELECT user_id FROM users WHERE user_name = 'kto_head.belousov')
WHERE user_name LIKE 'technologist.%' OR user_name LIKE 'proctech.%';

UPDATE users SET head_id = (SELECT user_id FROM users WHERE user_name = 'shop_assembly.kovalchuk')
WHERE user_name LIKE 'master.assem.%';

UPDATE users SET head_id = (SELECT user_id FROM users WHERE user_name = 'shop_body.melnik')
WHERE user_name LIKE 'master.body.%';

UPDATE users SET head_id = (SELECT user_id FROM users WHERE user_name = 'quality_head.nikolaev')
WHERE user_name LIKE 'controller.%';

UPDATE users SET head_id = (SELECT user_id FROM users WHERE user_name = 'economist_head.smirnova')
WHERE user_name LIKE 'economist.%' AND user_name NOT LIKE 'economist_head%';

UPDATE users SET head_id = (SELECT user_id FROM users WHERE user_name = 'purchaser_head.petrov')
WHERE user_name LIKE 'purchaser.%' AND user_name NOT LIKE 'purchaser_head%';

UPDATE users SET head_id = (SELECT user_id FROM users WHERE user_name = 'logistician_head.ivanova')
WHERE user_name LIKE 'logistician.%' AND user_name NOT LIKE 'logistician_head%';

-- ============================================================
-- 6. ПРОЕКТЫ
-- ============================================================
INSERT INTO projects (title, description, status, created_by, created_at, updated_at) VALUES
('Разработка нового автомобиля X7', 'Проект по созданию нового автомобиля X7 с гибридной силовой установкой', 'active', (SELECT user_id FROM users WHERE user_name = 'proto_pm.volkov'), NOW(), NOW()),
('Модернизация ДВС серии V8', 'Разработка и внедрение модернизированного ДВС V8', 'active', (SELECT user_id FROM users WHERE user_name = 'proto_pm.novikov'), NOW(), NOW()),
('Создание новой КПП 8-ступенчатой', 'Разработка 8-ступенчатой автоматической КПП', 'draft', (SELECT user_id FROM users WHERE user_name = 'proto_pm.orlov'), NOW(), NOW()),
('Прототип гибридной платформы', 'Разработка и изготовление опытного образца гибридной платформы', 'active', (SELECT user_id FROM users WHERE user_name = 'proto_pm.volkov'), NOW(), NOW())
ON CONFLICT (title) DO NOTHING;

-- ============================================================
-- 7. КОМАНДЫ ПРОЕКТОВ (project_members)
-- ============================================================
INSERT INTO project_members (project_id, user_id, role, joined_at) VALUES
-- X7
((SELECT project_id FROM projects WHERE title = 'Разработка нового автомобиля X7'), (SELECT user_id FROM users WHERE user_name = 'proto_pm.volkov'), 'owner', NOW()),
((SELECT project_id FROM projects WHERE title = 'Разработка нового автомобиля X7'), (SELECT user_id FROM users WHERE user_name = 'economist.1'), 'economist', NOW()),
((SELECT project_id FROM projects WHERE title = 'Разработка нового автомобиля X7'), (SELECT user_id FROM users WHERE user_name = 'logistician.1'), 'logist', NOW()),
-- ДВС
((SELECT project_id FROM projects WHERE title = 'Модернизация ДВС серии V8'), (SELECT user_id FROM users WHERE user_name = 'proto_pm.novikov'), 'owner', NOW()),
((SELECT project_id FROM projects WHERE title = 'Модернизация ДВС серии V8'), (SELECT user_id FROM users WHERE user_name = 'economist.2'), 'economist', NOW()),
((SELECT project_id FROM projects WHERE title = 'Модернизация ДВС серии V8'), (SELECT user_id FROM users WHERE user_name = 'logistician.2'), 'logist', NOW()),
-- КПП
((SELECT project_id FROM projects WHERE title = 'Создание новой КПП 8-ступенчатой'), (SELECT user_id FROM users WHERE user_name = 'proto_pm.orlov'), 'owner', NOW()),
((SELECT project_id FROM projects WHERE title = 'Создание новой КПП 8-ступенчатой'), (SELECT user_id FROM users WHERE user_name = 'economist.3'), 'economist', NOW()),
-- Гибрид
((SELECT project_id FROM projects WHERE title = 'Прототип гибридной платформы'), (SELECT user_id FROM users WHERE user_name = 'proto_pm.volkov'), 'owner', NOW()),
((SELECT project_id FROM projects WHERE title = 'Прототип гибридной платформы'), (SELECT user_id FROM users WHERE user_name = 'economist.1'), 'economist', NOW()),
((SELECT project_id FROM projects WHERE title = 'Прототип гибридной платформы'), (SELECT user_id FROM users WHERE user_name = 'logistician.3'), 'logist', NOW())
ON CONFLICT DO NOTHING;

-- ============================================================
-- 8. ПРОВЕРКА
-- ============================================================
SELECT '✅ Роли: ' || COUNT(*) FROM roles;
SELECT '✅ Отделы: ' || COUNT(*) FROM department;
SELECT '✅ Пользователи: ' || COUNT(*) FROM users;
SELECT '✅ Связи ролей: ' || COUNT(*) FROM users_roles;
SELECT '✅ Проекты: ' || COUNT(*) FROM projects;
SELECT '✅ Участники: ' || COUNT(*) FROM project_members;

-- Вывод списка пользователей с ролями
\echo ''
\echo '📋 ПОЛЬЗОВАТЕЛИ И ИХ РОЛИ:'
SELECT 
    u.user_name,
    u.full_name,
    u.email,
    u.dept_code,
    array_agg(r.role_title) as roles
FROM users u
LEFT JOIN users_roles ur ON u.user_id = ur.user_id
LEFT JOIN roles r ON ur.role_id = r.role_id
GROUP BY u.user_id, u.user_name, u.full_name, u.email, u.dept_code
ORDER BY u.user_name;

\echo ''
\echo '📋 ПРОЕКТЫ И УЧАСТНИКИ:'
SELECT 
    p.title as project,
    u.user_name,
    pm.role as role_in_project
FROM projects p
JOIN project_members pm ON p.project_id = pm.project_id
JOIN users u ON pm.user_id = u.user_id
ORDER BY p.title;

\echo ''
\echo '✅ ВСЕ ДАННЫЕ ЗАГРУЖЕНЫ!'
