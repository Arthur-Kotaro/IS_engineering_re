// production_client/src/core/AppCore.cpp
#include "AppCore.h"
#include "userserviceclient/ApiClient.h"
#include "userserviceclient/AuthService.h"
#include "userserviceclient/TokenManager.h"
#include <QDebug>
#include <memory>

// Глобальные экземпляры для доступа из Bridge
std::shared_ptr<UsersService::ApiClient> g_apiClient;
std::shared_ptr<UsersService::AuthService> g_authService;
std::shared_ptr<UsersService::TokenManager> g_tokenManager;

AppCore::AppCore() {}

void AppCore::init() {
    qDebug() << "AppCore: Initializing...";

    // Создаем ApiClient с Gateway (порт 8080)
    g_apiClient = std::make_shared<UsersService::ApiClient>();
    g_apiClient->setServerUrl("localhost", 8080);  // ← Gateway!

    // Создаем TokenManager
    g_tokenManager = std::make_shared<UsersService::TokenManager>();

    // Создаем AuthService
    g_authService = std::make_shared<UsersService::AuthService>(g_apiClient);

    qDebug() << "AppCore: Initialized with Gateway on localhost:8080";
}

// Геттеры для доступа из Bridge
std::shared_ptr<UsersService::AuthService> getAuthService() {
    return g_authService;
}

std::shared_ptr<UsersService::ApiClient> getApiClient() {
    return g_apiClient;
}
