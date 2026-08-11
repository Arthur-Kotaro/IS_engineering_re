#include "AuthBridge.h"
#include "userserviceclient/AuthService.h"
#include <QDebug>

AuthBridge::AuthBridge(std::shared_ptr<UsersService::AuthService> authService, QObject* parent)
    : QObject(parent)
    , m_authService(authService)
{
    connect(m_authService.get(), &UsersService::AuthService::loginCompleted,
            this, &AuthBridge::onLoginCompleted);
}

void AuthBridge::attemptLogin(const QString& email, const QString& password)
{
    qDebug() << "Login attempt:" << email;
    m_errorMessage.clear();
    m_authService->login(email, password);
}

void AuthBridge::logout()
{
    m_authService->logout();
    m_isAuthenticated = false;
    emit authStatusChanged();
}

void AuthBridge::onLoginCompleted(const UsersService::AuthResult& result)
{
    if (result.success) {
        qDebug() << "Login successful!";
        m_isAuthenticated = true;
        m_errorMessage.clear();
        emit authStatusChanged();
        emit loginSuccess();
    } else {
        qDebug() << "Login failed:" << result.errorMessage;
        m_errorMessage = result.errorMessage;
        
        // Отправляем сигнал с сообщением об ошибке
        emit loginFailed(result.errorMessage);
    }
}
