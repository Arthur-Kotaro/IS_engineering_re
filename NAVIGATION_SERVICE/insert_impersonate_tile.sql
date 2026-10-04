INSERT OR IGNORE INTO tiles (
    role, tile_id, label, endpoint, method, icon, is_active, sort_order, badge_enabled, badge_endpoint
) VALUES
    ('user',       'impersonate', 'Войти как ...', '/page/impersonate', 'GET', '🎭', 1, 90, 0, NULL),
    ('admin',      'impersonate', 'Войти как ...', '/page/impersonate', 'GET', '🎭', 1, 90, 0, NULL),
    ('hr',         'impersonate', 'Войти как ...', '/page/impersonate', 'GET', '🎭', 1, 90, 0, NULL),
    ('manager',    'impersonate', 'Войти как ...', '/page/impersonate', 'GET', '🎭', 1, 90, 0, NULL);
