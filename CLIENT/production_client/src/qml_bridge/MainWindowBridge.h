#ifndef MAINWINDOWBRIDGE_H
#define MAINWINDOWBRIDGE_H

#include <QObject>
#include <QVariantList>
#include <memory>
#include "userserviceclient/UserProfile.h"

class QNetworkAccessManager;

namespace UsersService {
    class AuthService;
}

class MainWindowBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString userName READ userName NOTIFY userDataChanged)
    Q_PROPERTY(QString userFullName READ userFullName NOTIFY userDataChanged)
    Q_PROPERTY(QString userEmail READ userEmail NOTIFY userDataChanged)
    Q_PROPERTY(QString userPosition READ userPosition NOTIFY userDataChanged)
    Q_PROPERTY(QString userDept READ userDept NOTIFY userDataChanged)
    Q_PROPERTY(QStringList userRoles READ userRoles NOTIFY userDataChanged)
    Q_PROPERTY(int passwordDaysLeft READ passwordDaysLeft NOTIFY passwordExpiryChanged)
    Q_PROPERTY(bool passwordExpired READ passwordExpired NOTIFY passwordExpiryChanged)
    Q_PROPERTY(QString accessToken READ accessToken NOTIFY userDataChanged)

public:
    explicit MainWindowBridge(std::shared_ptr<UsersService::AuthService> authService, QObject *parent = nullptr);

    QString userName() const;
    QString userFullName() const;
    QString userEmail() const;
    QString userPosition() const;
    QString userDept() const;
    QStringList userRoles() const;
    int passwordDaysLeft() const;
    bool passwordExpired() const;
    QString accessToken() const;

    Q_INVOKABLE void changePassword(const QString& currentPassword, const QString& newPassword);
    Q_INVOKABLE void logout();
    Q_INVOKABLE void checkPasswordExpiry();
    Q_INVOKABLE void loadTiles();

signals:
    void userDataChanged();
    void passwordExpiryChanged();
    void passwordChangeCompleted(bool success, const QString& message);
    void logoutCompleted();
    void tilesLoaded(const QVariantList& tiles);

private slots:
    void onProfileFetched(const UsersService::UserProfile& profile);
    void onPasswordExpiryInfo(int daysRemaining, bool isExpired, const QString& expiresAt);
    void onPasswordChanged(bool success, const QString& message);
    void onSessionExpired();

private:
    std::shared_ptr<UsersService::AuthService> m_authService;
    UsersService::UserProfile m_profile;
    int m_passwordDaysLeft = 0;
    QNetworkAccessManager* m_networkManager;
    bool m_tilesLoaded = false;
};

#endif // MAINWINDOWBRIDGE_H
