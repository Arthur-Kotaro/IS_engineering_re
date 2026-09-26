# Обзор архитектуры ERP «Engineering:re»

## Компоненты

    Client (Qt6 QML)
        │ HTTP :8080
        ▼
    Nginx (API Gateway)
        ├─ auth_request → Auth
        ├─ кэш токенов
        ├─ маршрутизация
        └─ streaming
        │
        ├──► Auth Service :8010 (POST /verify)
        │       └──► Redis (blacklist)
        │
        └──► Бизнес-сервисы
                USER         :8000
                PROJECT      :8001
                MASTERGRAPHICS :8003 (план)
                NAVIGATION   :8009
                DELEGATION   :8011
                NOTIFICATION :8012
                UI COMPOSER  :8020

    UI Composer ходит в сервисы напрямую (минуя nginx) для sources.

## Сервисы

### AUTH_SERVICE :8010
- Валидатор JWT.
- POST /verify.

### USER_service :8000
- Пользователи, роли, аутентификация, issue JWT.
- Публичные: login, refresh, reset-password, register.
- Защищённые: users, hr, admin, internal.

### PROJECT_service :8001
- Проекты, команды, права на проект.
- Композиция direct OR subordinate OR delegated.

### NAVIGATION_SERVICE :8009
- Плитки рабочего стола. SQLite.

### DELEGATION_SERVICE :8011
- Управление делегированиями.
- Два типа: DIRECT, TEMPORARY.

### NOTIFICATION_SERVICE :8012
- Уведомления (in-app, email).

### UI_COMPOSER_SERVICE :8020
- Server-driven UI.
- Workflow + шаблоны + сессии.
- Слеп к правам.

### MASTERGRAPHICS_SERVICE :8003
- Хранение мастерграфиков. В разработке.

## Клиент

- Qt6 QML, C++20.
- Окна: AuthWindow, MainWindow, DelegatedWindow (план), SettingsWindow, NotificationsWindow.
- Компоненты: WidgetBridge, JsonUiRenderer, QmlObjectFactory, DataManager, ApiClient, AuthService, TokenManager, UserProfile.

## Стек

- Python 3.12+, FastAPI, uvicorn, SQLAlchemy 2.0 async, asyncpg.
- PostgreSQL (SQLite в Navigation).
- Redis.
- JWT HS256, bcrypt.
- Nginx.
- C++20, Qt6, CMake, Ninja.
