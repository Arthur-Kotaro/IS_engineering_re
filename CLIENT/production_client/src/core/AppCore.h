#ifndef APPCORE_H
#define APPCORE_H

#include <memory>
#include <QObject>

class QQmlEngine;
class DataManager;
class JsonUiRenderer;

namespace UsersService {
    class AuthService;
    class ApiClient;
    class TokenManager;
}

class AppCore : public QObject
{
    Q_OBJECT

public:
    explicit AppCore(QQmlEngine* engine = nullptr, QObject* parent = nullptr);
    ~AppCore();

    void init();

    // Геттеры для C++ кода (не экспортируются в QML)
    DataManager* dataManager() const { return m_dataManager; }
    JsonUiRenderer* renderer() const { return m_renderer; }

    // Глобальные геттеры
    static std::shared_ptr<UsersService::AuthService> authService();
    static std::shared_ptr<UsersService::ApiClient> apiClient();
    static std::shared_ptr<UsersService::TokenManager> tokenManager();

private:
    QQmlEngine* m_engine = nullptr;
    DataManager* m_dataManager = nullptr;
    JsonUiRenderer* m_renderer = nullptr;

    static std::shared_ptr<UsersService::ApiClient> s_apiClient;
    static std::shared_ptr<UsersService::AuthService> s_authService;
    static std::shared_ptr<UsersService::TokenManager> s_tokenManager;
};

#endif // APPCORE_H
