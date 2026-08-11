#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QCoreApplication>
#include <QTimer>
#include <QSignalSpy>
#include "userserviceclient/AuthService.h"
#include "../utils/MockApiClient.h"
#include "../utils/TestHelpers.h"

using namespace UsersService;
using ::testing::_;
using ::testing::Invoke;

// Forward declaration (для тестирования нужно будет включить EngineeringConsoleUI)
// В реальном проекте нужно добавить EngineeringConsoleUI.h в include пути

class EngineeringConsoleUITest : public ::testing::Test {
protected:
    void SetUp() override {
        mockApiClient = std::make_shared<testing::NiceMock<MockApiClient>>();
        authService = std::make_shared<AuthService>(mockApiClient);
    }
    
    void TearDown() override {
        authService.reset();
        mockApiClient.reset();
    }
    
    std::shared_ptr<testing::NiceMock<MockApiClient>> mockApiClient;
    std::shared_ptr<AuthService> authService;
};

TEST_F(EngineeringConsoleUITest, SuccessfulUserLogin) {
    QString email = "test@example.com";
    QString password = "password123";
    QString accessToken = "test_access_token";
    QString refreshToken = "test_refresh_token";
    
    EXPECT_CALL(*mockApiClient, post("/auth/login", _, _))
        .WillOnce(Invoke([accessToken, refreshToken](const QString&, const QJsonObject& data, ApiCallback callback) {
            // Проверяем данные входа
            EXPECT_EQ(data["email"].toString(), "test@example.com");
            EXPECT_EQ(data["password"].toString(), "password123");
            
            ApiResponse response;
            response.success = true;
            response.httpCode = 200;
            
            QJsonObject responseData;
            responseData["access_token"] = accessToken;
            responseData["refresh_token"] = refreshToken;
            response.data = QJsonDocument(responseData);
            
            callback(response);
        }));
    
    bool loginSuccess = false;
    connect(authService.get(), &AuthService::loginCompleted,
            [&](const AuthResult& result) {
                loginSuccess = result.success;
            });
    
    authService->login(email, password);
    
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, &QEventLoop::quit);
    connect(authService.get(), &AuthService::loginCompleted, &loop, &QEventLoop::quit);
    loop.exec();
    
    EXPECT_TRUE(loginSuccess);
}

TEST_F(EngineeringConsoleUITest, LoginFailureWithInvalidCredentials) {
    EXPECT_CALL(*mockApiClient, post("/auth/login", _, _))
        .WillOnce(Invoke([](const QString&, const QJsonObject&, ApiCallback callback) {
            ApiResponse response;
            response.success = false;
            response.httpCode = 401;
            response.errorType = "invalid_credentials";
            response.errorString = "Invalid credentials";
            callback(response);
        }));
    
    bool loginFailed = false;
    QString errorType;
    
    connect(authService.get(), &AuthService::loginCompleted,
            [&](const AuthResult& result) {
                loginFailed = !result.success;
                errorType = result.errorType;
            });
    
    authService->login("wrong@example.com", "wrongpass");
    
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, &QEventLoop::quit);
    connect(authService.get(), &AuthService::loginCompleted, &loop, &QEventLoop::quit);
    loop.exec();
    
    EXPECT_TRUE(loginFailed);
    EXPECT_EQ(errorType, "invalid_credentials");
}

TEST_F(EngineeringConsoleUITest, PasswordExpiryCheckAfterLogin) {
    // Сначала успешный логин
    UserSession session;
    session.isValid = true;
    session.accessToken = "valid_token";
    
    EXPECT_CALL(*mockApiClient, get("/auth/password-expiry", _))
        .WillOnce(Invoke([](const QString&, ApiCallback callback) {
            ApiResponse response;
            response.success = true;
            response.httpCode = 200;
            
            QJsonObject data;
            data["days_remaining"] = 5;
            data["is_expired"] = false;
            data["expires_at"] = "2025-12-31T23:59:59";
            response.data = QJsonDocument(data);
            
            callback(response);
        }));
    
    bool expiryChecked = false;
    int daysRemaining = 0;
    
    connect(authService.get(), &AuthService::passwordExpiryInfo,
            [&](int days, bool, const QString&) {
                daysRemaining = days;
                expiryChecked = true;
            });
    
    authService->checkPasswordExpiry();
    
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, &QEventLoop::quit);
    connect(authService.get(), &AuthService::passwordExpiryInfo, &loop, &QEventLoop::quit);
    loop.exec();
    
    EXPECT_TRUE(expiryChecked);
    EXPECT_EQ(daysRemaining, 5);
}

TEST_F(EngineeringConsoleUITest, TokenRefresh) {
    QString oldRefreshToken = "old_refresh";
    QString newAccessToken = "new_access_token";
    QString newRefreshToken = "new_refresh_token";
    
    EXPECT_CALL(*mockApiClient, post("/auth/refresh", _, _))
        .WillOnce(Invoke([newAccessToken, newRefreshToken](const QString&, const QJsonObject& data, ApiCallback callback) {
            EXPECT_EQ(data["refresh_token"].toString(), "old_refresh");
            
            ApiResponse response;
            response.success = true;
            QJsonObject responseData;
            responseData["access_token"] = newAccessToken;
            responseData["refresh_token"] = newRefreshToken;
            response.data = QJsonDocument(responseData);
            callback(response);
        }));
    
    bool tokenRefreshed = false;
    connect(authService.get(), &AuthService::tokenRefreshed,
            [&](const QString& access, const QString& refresh) {
                EXPECT_EQ(access, newAccessToken);
                EXPECT_EQ(refresh, newRefreshToken);
                tokenRefreshed = true;
            });
    
    authService->refreshToken(oldRefreshToken);
    
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, &QEventLoop::quit);
    connect(authService.get(), &AuthService::tokenRefreshed, &loop, &QEventLoop::quit);
    loop.exec();
    
    EXPECT_TRUE(tokenRefreshed);
}

TEST_F(EngineeringConsoleUITest, ChangePassword) {
    QString currentPwd = "oldpass";
    QString newPwd = "newpass";
    
    EXPECT_CALL(*mockApiClient, post("/auth/change-password", _, _))
        .WillOnce(Invoke([currentPwd, newPwd](const QString&, const QJsonObject& data, ApiCallback callback) {
            EXPECT_EQ(data["current_password"].toString(), currentPwd);
            EXPECT_EQ(data["new_password"].toString(), newPwd);
            
            ApiResponse response;
            response.success = true;
            QJsonObject responseData;
            responseData["message"] = "Password changed successfully";
            response.data = QJsonDocument(responseData);
            callback(response);
        }));
    
    bool passwordChanged = false;
    connect(authService.get(), &AuthService::passwordChanged,
            [&](bool success, const QString&) {
                passwordChanged = success;
            });
    
    authService->changePassword(currentPwd, newPwd);
    
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, &QEventLoop::quit);
    connect(authService.get(), &AuthService::passwordChanged, &loop, &QEventLoop::quit);
    loop.exec();
    
    EXPECT_TRUE(passwordChanged);
}

TEST_F(EngineeringConsoleUITest, ResetPassword) {
    QString email = "test@example.com";
    
    EXPECT_CALL(*mockApiClient, post("/auth/reset-password", _, _))
        .WillOnce(Invoke([email](const QString&, const QJsonObject& data, ApiCallback callback) {
            EXPECT_EQ(data["email"].toString(), email);
            
            ApiResponse response;
            response.success = true;
            QJsonObject responseData;
            responseData["message"] = "Reset email sent";
            response.data = QJsonDocument(responseData);
            callback(response);
        }));
    
    bool passwordReset = false;
    connect(authService.get(), &AuthService::passwordReset,
            [&](bool success, const QString&) {
                passwordReset = success;
            });
    
    authService->resetPassword(email);
    
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, &QEventLoop::quit);
    connect(authService.get(), &AuthService::passwordReset, &loop, &QEventLoop::quit);
    loop.exec();
    
    EXPECT_TRUE(passwordReset);
}

TEST_F(EngineeringConsoleUITest, HandleNetworkError) {
    EXPECT_CALL(*mockApiClient, post("/auth/login", _, _))
        .WillOnce(Invoke([](const QString&, const QJsonObject&, ApiCallback callback) {
            ApiResponse response;
            response.success = false;
            response.errorType = "network";
            response.errorString = "Connection refused";
            callback(response);
        }));
    
    QString errorType;
    connect(authService.get(), &AuthService::loginCompleted,
            [&](const AuthResult& result) {
                errorType = result.errorType;
            });
    
    authService->login("test@example.com", "password");
    
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, &QEventLoop::quit);
    connect(authService.get(), &AuthService::loginCompleted, &loop, &QEventLoop::quit);
    loop.exec();
    
    EXPECT_EQ(errorType, "network");
}
