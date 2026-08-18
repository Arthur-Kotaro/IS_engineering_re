-- 001_initial.sql
-- Создание таблиц для Delegation Service

-- Таблица delegations
CREATE TABLE IF NOT EXISTS delegations (
    delegation_id SERIAL PRIMARY KEY,
    delegator_id INTEGER NOT NULL,
    delegator_name VARCHAR(100),
    delegate_id INTEGER NOT NULL,
    delegate_name VARCHAR(100),
    main_delegate_id INTEGER,
    delegation_type VARCHAR(20) NOT NULL,
    starts_at TIMESTAMP WITH TIME ZONE NOT NULL,
    expires_at TIMESTAMP WITH TIME ZONE NOT NULL,
    status VARCHAR(20) DEFAULT 'active',
    reason VARCHAR(500),
    created_by INTEGER NOT NULL,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    revoked_at TIMESTAMP WITH TIME ZONE,
    revoked_by INTEGER,
    revoke_reason VARCHAR(500)
);

CREATE INDEX idx_delegations_delegator_id ON delegations(delegator_id);
CREATE INDEX idx_delegations_delegate_id ON delegations(delegate_id);
CREATE INDEX idx_delegations_status ON delegations(status);
CREATE INDEX idx_delegations_expires_at ON delegations(expires_at);
CREATE INDEX idx_delegations_type ON delegations(delegation_type);

-- Таблица delegation_history
CREATE TABLE IF NOT EXISTS delegation_history (
    history_id SERIAL PRIMARY KEY,
    delegation_id INTEGER REFERENCES delegations(delegation_id) ON DELETE CASCADE,
    action VARCHAR(50) NOT NULL,
    user_id INTEGER NOT NULL,
    user_name VARCHAR(100),
    timestamp TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    details JSONB,
    ip_address VARCHAR(45),
    user_agent VARCHAR(255)
);

CREATE INDEX idx_history_delegation_id ON delegation_history(delegation_id);
CREATE INDEX idx_history_user_id ON delegation_history(user_id);
CREATE INDEX idx_history_timestamp ON delegation_history(timestamp);

-- Таблица delegation_rules
CREATE TABLE IF NOT EXISTS delegation_rules (
    rule_id SERIAL PRIMARY KEY,
    role VARCHAR(50) UNIQUE NOT NULL,
    can_delegate BOOLEAN DEFAULT FALSE,
    max_delegations INTEGER DEFAULT 1,
    max_duration_days INTEGER DEFAULT 30,
    requires_approval BOOLEAN DEFAULT FALSE,
    direct_allowed BOOLEAN DEFAULT TRUE,
    reverse_allowed BOOLEAN DEFAULT FALSE,
    temporary_allowed BOOLEAN DEFAULT TRUE
);

CREATE INDEX idx_rules_role ON delegation_rules(role);

-- Добавляем правила по умолчанию
INSERT INTO delegation_rules (role, can_delegate, max_delegations, max_duration_days, reverse_allowed)
VALUES 
    ('admin', TRUE, 5, 90, TRUE),
    ('manager', TRUE, 3, 60, TRUE),
    ('hr', TRUE, 2, 30, TRUE),
    ('user', FALSE, 0, 0, FALSE)
ON CONFLICT (role) DO NOTHING;
