#ifndef APPCORE_H
#define APPCORE_H

#include <QObject>
#include <memory>

class QQmlEngine;
class DataManager;
class JsonUiRenderer;

namespace UsersService {
    class ApiClient;
    class AuthService;
    class TokenManager;
}

class AppCore : public QObject
{
    Q_OBJECT

public:
    explicit AppCore(QQmlEngine* engine, QObject* parent = nullptr);
    ~AppCore();

    void init();

    DataManager* dataManager() const { return m_dataManager; }
    JsonUiRenderer* renderer() const { return m_renderer; }

    static std::shared_ptr<UsersService::AuthService> authService();
    static std::shared_ptr<UsersService::ApiClient> apiClient();
    static std::shared_ptr<UsersService::TokenManager> tokenManager();

    Q_INVOKABLE void applyFontSize(int fontSize);

private:
    QQmlEngine* m_engine;
    DataManager* m_dataManager = nullptr;
    JsonUiRenderer* m_renderer = nullptr;

    static std::shared_ptr<UsersService::ApiClient> s_apiClient;
    static std::shared_ptr<UsersService::AuthService> s_authService;
    static std::shared_ptr<UsersService::TokenManager> s_tokenManager;
};

#endif // APPCORE_H
