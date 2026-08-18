// production_client/src/qml_bridge/MainWindowBridge.cpp
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

QString MainWindowBridge::userName() const {
    return m_profile.userName;
}

QString MainWindowBridge::userEmail() const {
    return m_profile.email;
}

QString MainWindowBridge::userPosition() const {
    if (m_profile.roles.contains("chief_project_engineer")) return "Главный инженер";
    if (m_profile.roles.contains("project_secretary")) return "Секретарь проекта";
    if (m_profile.roles.contains("project_economist")) return "Экономист проекта";
    if (m_profile.roles.contains("system_architect")) return "Архитектор";
    if (m_profile.roles.contains("validator")) return "Специалист по валидации";
    if (m_profile.roles.contains("test_specialist")) return "Специалист по испытаниям";
    if (m_profile.roles.contains("project_manager_ist")) return "Руководитель проекта (ст.)";
    if (m_profile.roles.contains("project_manager_pfe")) return "Руководитель проекта (мл.)";
    if (m_profile.roles.contains("proto_purchaser")) return "Специалист по закупкам";
    if (m_profile.roles.isEmpty()) return "Сотрудник";
    return m_profile.roles.first();
}

QString MainWindowBridge::userRole() const {
    if (m_profile.roles.isEmpty()) return "user";
    return m_profile.roles.first();
}

int MainWindowBridge::passwordDaysLeft() const {
    return m_passwordDaysLeft;
}

bool MainWindowBridge::passwordExpired() const {
    return m_passwordDaysLeft <= 0;
}

QVariantList MainWindowBridge::projects() const {
    QVariantList list;
    for (const auto& proj : m_profile.projects) {
        QVariantMap map;
        map["project_id"] = proj["project_id"].toInt();
        map["name"] = proj["name"].toString();
        map["role"] = proj["role"].toString();
        list.append(map);
    }
    return list;
}

QString MainWindowBridge::widgetsPath() const {
    if (m_profile.roles.contains("chief_project_engineer")) return "widgets/roles/chief_project_engineer/";
    if (m_profile.roles.contains("project_secretary")) return "widgets/roles/project_secretary/";
    if (m_profile.roles.contains("project_economist")) return "widgets/roles/project_economist/";
    if (m_profile.roles.contains("system_architect")) return "widgets/roles/system_architect/";
    if (m_profile.roles.contains("validator")) return "widgets/roles/validator/";
    if (m_profile.roles.contains("test_specialist")) return "widgets/roles/test_specialist/";
    if (m_profile.roles.contains("project_manager_ist")) return "widgets/roles/project_manager_ist/";
    if (m_profile.roles.contains("project_manager_pfe")) return "widgets/roles/project_manager_pfe/";
    if (m_profile.roles.contains("proto_purchaser")) return "widgets/roles/proto_purchaser/";
    return "widgets/common/";
}

QString MainWindowBridge::accessToken() const {
    return m_authService->currentSession().accessToken;
}

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
    if (m_tilesLoaded) {
        qDebug() << "Tiles already loaded, skipping";
        return;
    }
    
    qDebug() << "Loading tiles from Navigation Service...";

    QString token = m_authService->currentSession().accessToken;
    if (token.isEmpty()) {
        qDebug() << "No access token available, will retry after profile fetch";
        return;
    }

    QNetworkRequest request(QUrl("http://localhost:8080/api/v1/navigation/dashboard"));
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Failed to load tiles:" << reply->errorString();
            return;
        }

        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull()) {
            qDebug() << "Invalid JSON response";
            return;
        }

        QJsonObject json = doc.object();
        QJsonArray tiles = json["tiles"].toArray();

        QVariantList tileList;
        for (const auto& tile : tiles) {
            tileList.append(tile.toVariant());
        }

        m_tilesLoaded = true;
        emit tilesLoaded(tileList);
        qDebug() << "Tiles loaded:" << tileList.size();
    });
}

void MainWindowBridge::onProfileFetched(const UsersService::UserProfile& profile) {
    m_profile = profile;
    emit userDataChanged();
    m_authService->checkPasswordExpiry();
    
    // Загружаем плитки после получения профиля
    loadTiles();
}

void MainWindowBridge::onPasswordExpiryInfo(int daysRemaining, bool isExpired, const QString& expiresAt) {
    Q_UNUSED(isExpired);
    Q_UNUSED(expiresAt);
    m_passwordDaysLeft = daysRemaining;
    qDebug() << "Password expiry updated: days left =" << daysRemaining;
    emit passwordExpiryChanged();
}

void MainWindowBridge::onPasswordChanged(bool success, const QString& message) {
    emit passwordChangeCompleted(success, message);
}

void MainWindowBridge::onSessionExpired() {
    emit logoutCompleted();
}
