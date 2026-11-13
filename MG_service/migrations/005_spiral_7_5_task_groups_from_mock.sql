-- MG_service: task_groups из макета (11 групп)
INSERT INTO task_groups (group_id, name, description, bus_instance_id) VALUES
    ('milestones_preparing', 'Подготовка к вехе', 'Milestones preparing', 'other_bus'),
    ('qcdp', 'Входной билет', 'QCDP', 'other_bus'),
    ('documentation', 'Документация', 'Конструкторская документация и ИС', 'bus_documentation_01'),
    ('architecture', 'Архитектура', 'Architecture', 'bus_architecture_01'),
    ('logistics', 'Логистика', 'Supply chain', 'other_bus'),
    ('after_sales', 'Послепродажное обслуживание', 'After sales', 'other_bus'),
    ('supplier_nomination', 'Номинация поставщиков', 'Инжиниринг продукта, СМС', 'bus_supplier_01'),
    ('prototypes', 'Прототипы', 'Prototyping', 'bus_prototypes_01'),
    ('validation', 'Валидация', 'Validation', 'bus_validation_01'),
    ('calibration', 'Калибровки', 'Calibration', 'bus_calibration_01'),
    ('production_preparing', 'Подготовка производства', 'Process/Процесс', 'other_bus')
ON CONFLICT (group_id) DO UPDATE SET
    name = EXCLUDED.name,
    description = EXCLUDED.description,
    bus_instance_id = EXCLUDED.bus_instance_id,
    is_deleted = FALSE,
    deleted_at = NULL,
    deleted_by = NULL;
