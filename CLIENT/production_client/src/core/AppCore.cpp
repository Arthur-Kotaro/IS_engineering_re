#include "AppCore.h"
#include "core/DataManager.h"
#include "renderer/JsonUiRenderer.h"
#include "userserviceclient/ApiClient.h"
#include "userserviceclient/AuthService.h"
#include "userserviceclient/TokenManager.h"
#include <QQmlEngine>
#include <QQmlContext>
#include <QGuiApplication>
#include <QFont>
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

    s_apiClient = std::make_shared<UsersService::ApiClient>();
    s_apiClient->setServerUrl("localhost", 8080);

    s_tokenManager = std::make_shared<UsersService::TokenManager>();

    s_authService = std::make_shared<UsersService::AuthService>(s_apiClient);

    m_dataManager = new DataManager(this);
    
    m_renderer = new JsonUiRenderer(m_engine, this);
    m_renderer->setDataManager(m_dataManager);

    if (m_engine) {
        m_engine->rootContext()->setContextProperty("renderer", m_renderer);
        qDebug() << "AppCore: renderer set to QML context";
    }

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

void AppCore::applyFontSize(int fontSize)
{
    if (!m_engine) return;
    
    QFont defaultFont = QGuiApplication::font();
    defaultFont.setPixelSize(fontSize);
    QGuiApplication::setFont(defaultFont);
    
    if (m_renderer) {
        m_renderer->refreshAllCharts();
        m_renderer->refreshContentHeight();
    }
    
    qDebug() << "AppCore: Font size applied:" << fontSize;
}
