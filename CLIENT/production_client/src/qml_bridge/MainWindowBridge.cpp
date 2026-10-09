#include "MainWindowBridge.h"
#include "userserviceclient/AuthService.h"
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

MainWindowBridge::MainWindowBridge(std::shared_ptr<UsersService::AuthService> authService, QObject *parent)
: QObject(parent)
, m_authService(authService)
, m_networkManager(new QNetworkAccessManager(this))
, m_tilesLoaded(false)
{
    connect(m_authService.get(), &UsersService::AuthService::profileFetched,
            this, &MainWindowBridge::onProfileFetched);
    connect(m_authService.get(), &UsersService::AuthService::passwordExpiryInfo,
            this, &MainWindowBridge::onPasswordExpiryInfo);
    connect(m_authService.get(), &UsersService::AuthService::passwordChanged,
            this, &MainWindowBridge::onPasswordChanged);
    connect(m_authService.get(), &UsersService::AuthService::sessionExpired,
            this, &MainWindowBridge::onSessionExpired);
}

QString MainWindowBridge::userName() const { return m_profile.userName; }
QString MainWindowBridge::userFullName() const { return m_profile.fullName(); }
QString MainWindowBridge::userEmail() const { return m_profile.email; }

QString MainWindowBridge::userPosition() const {
    return m_profile.positionNameRu.isEmpty() ? m_profile.positionCode : m_profile.positionNameRu;
}

QString MainWindowBridge::userDept() const {
    return m_profile.deptNameRu;
}

QStringList MainWindowBridge::userRoles() const {
    return m_profile.roles;
}

int MainWindowBridge::passwordDaysLeft() const { return m_passwordDaysLeft; }
bool MainWindowBridge::passwordExpired() const { return m_passwordDaysLeft <= 0; }
QString MainWindowBridge::accessToken() const { return m_authService->currentSession().accessToken; }

void MainWindowBridge::changePassword(const QString& currentPassword, const QString& newPassword) {
    m_authService->changePassword(currentPassword, newPassword, newPassword);
}

void MainWindowBridge::logout() {
    m_authService->logout();
    emit logoutCompleted();
}

void MainWindowBridge::checkPasswordExpiry() {
    m_authService->checkPasswordExpiry();
}

void MainWindowBridge::loadTiles() {
    if (m_tilesLoaded) return;
    QString token = m_authService->currentSession().accessToken;
    if (token.isEmpty()) return;

    QNetworkRequest request(QUrl("http://localhost:8080/api/v1/navigation/dashboard"));
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Tiles load error:" << reply->errorString();
            return;
        }
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull()) return;
        QJsonArray tiles = doc.object()["tiles"].toArray();
        QVariantList list;
        for (const auto& t : tiles) list.append(t.toVariant());
        m_tilesLoaded = true;
        emit tilesLoaded(list);
    });
}

void MainWindowBridge::onProfileFetched(const UsersService::UserProfile& profile) {
    m_profile = profile;
    emit userDataChanged();
    m_authService->checkPasswordExpiry();
    loadTiles();
}

void MainWindowBridge::onPasswordExpiryInfo(int daysRemaining, bool isExpired, const QString& expiresAt) {
    Q_UNUSED(isExpired); Q_UNUSED(expiresAt);
    m_passwordDaysLeft = daysRemaining;
    emit passwordExpiryChanged();
}

void MainWindowBridge::onPasswordChanged(bool success, const QString& message) {
    emit passwordChangeCompleted(success, message);
}

void MainWindowBridge::onSessionExpired() {
    emit logoutCompleted();
}
