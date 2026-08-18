#include "AppCore.h"
#include "core/DataManager.h"
#include "renderer/JsonUiRenderer.h"
#include "userserviceclient/ApiClient.h"
#include "userserviceclient/AuthService.h"
#include "userserviceclient/TokenManager.h"
#include <QQmlEngine>
#include <QDebug>

// Статические экземпляры
std::shared_ptr<UsersService::ApiClient> AppCore::s_apiClient;
std::shared_ptr<UsersService::AuthService> AppCore::s_authService;
std::shared_ptr<UsersService::TokenManager> AppCore::s_tokenManager;

AppCore::AppCore(QQmlEngine* engine, QObject* parent)
    : QObject(parent)
    , m_engine(engine)
{
    qDebug() << "AppCore created";
}

AppCore::~AppCore()
{
    qDebug() << "AppCore destroyed";
}

void AppCore::init()
{
    qDebug() << "AppCore: Initializing...";

    // Создаем ApiClient с Gateway (порт 8080)
    s_apiClient = std::make_shared<UsersService::ApiClient>();
    s_apiClient->setServerUrl("localhost", 8080);

    // Создаем TokenManager
    s_tokenManager = std::make_shared<UsersService::TokenManager>();

    // Создаем AuthService
    s_authService = std::make_shared<UsersService::AuthService>(s_apiClient);

    // Создаем DataManager (использует ApiClient через common)
    m_dataManager = new DataManager(this);
    
    // Создаем JsonUiRenderer
    m_renderer = new JsonUiRenderer(m_engine, this);
    m_renderer->setDataManager(m_dataManager);

    qDebug() << "AppCore: Initialized with Gateway on localhost:8080";
    qDebug() << "AppCore: DataManager and Renderer created";
}

std::shared_ptr<UsersService::AuthService> AppCore::authService()
{
    return s_authService;
}

std::shared_ptr<UsersService::ApiClient> AppCore::apiClient()
{
    return s_apiClient;
}

std::shared_ptr<UsersService::TokenManager> AppCore::tokenManager()
{
    return s_tokenManager;
}
