#ifndef AUTHBRIDGE_H
#define AUTHBRIDGE_H

#include <QObject>
#include <memory>

namespace UsersService {
class AuthService;
struct AuthResult;
}

class AuthBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isAuthenticated READ isAuthenticated NOTIFY authStatusChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit AuthBridge(std::shared_ptr<UsersService::AuthService> authService, QObject* parent = nullptr);
    
    Q_INVOKABLE void attemptLogin(const QString& email, const QString& password);
    Q_INVOKABLE void logout();
    
    bool isAuthenticated() const { return m_isAuthenticated; }
    QString errorMessage() const { return m_errorMessage; }

signals:
    void authStatusChanged();
    void errorMessageChanged();
    void loginSuccess();
    void loginFailed(const QString& message);  // Новый сигнал

private slots:
    void onLoginCompleted(const UsersService::AuthResult& result);

private:
    std::shared_ptr<UsersService::AuthService> m_authService;
    bool m_isAuthenticated = false;
    QString m_errorMessage;
};

#endif // AUTHBRIDGE_H
