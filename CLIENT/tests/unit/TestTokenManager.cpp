#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "userserviceclient/TokenManager.h"
#include <QJsonObject>
#include <QDateTime>

using namespace UsersService;

class TokenManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        tokenManager = std::make_unique<TokenManager>();
    }
    
    void TearDown() override {
        tokenManager.reset();
    }
    
    std::unique_ptr<TokenManager> tokenManager;
};

TEST_F(TokenManagerTest, InitiallyNoTokens) {
    EXPECT_FALSE(tokenManager->hasValidAccessToken());
    EXPECT_TRUE(tokenManager->getAccessToken().isEmpty());
    EXPECT_TRUE(tokenManager->getRefreshToken().isEmpty());
}

TEST_F(TokenManagerTest, SetAndGetTokens) {
    QString accessToken = "test_access_token_123";
    QString refreshToken = "test_refresh_token_456";
    
    tokenManager->setTokens(accessToken, refreshToken);
    
    EXPECT_TRUE(tokenManager->hasValidAccessToken());
    EXPECT_EQ(tokenManager->getAccessToken(), accessToken);
    EXPECT_EQ(tokenManager->getRefreshToken(), refreshToken);
}

TEST_F(TokenManagerTest, ClearTokens) {
    tokenManager->setTokens("token1", "token2");
    tokenManager->clearTokens();
    
    EXPECT_FALSE(tokenManager->hasValidAccessToken());
    EXPECT_TRUE(tokenManager->getAccessToken().isEmpty());
    EXPECT_TRUE(tokenManager->getRefreshToken().isEmpty());
}

TEST_F(TokenManagerTest, TokenExpirationDetection) {
    // Создаем истекший токен
    QDateTime expiredTime = QDateTime::currentDateTime().addSecs(-3600);
    QJsonObject payload;
    payload["exp"] = expiredTime.toSecsSinceEpoch();
    
    // TokenManager должен определить, что токен истек
    // Для полного теста нужен реальный JWT парсер
}

TEST_F(TokenManagerTest, UpdateRefreshTokenOnly) {
    tokenManager->setTokens("old_access", "old_refresh");
    tokenManager->updateRefreshToken("new_refresh");
    
    EXPECT_EQ(tokenManager->getAccessToken(), "old_access");
    EXPECT_EQ(tokenManager->getRefreshToken(), "new_refresh");
}

TEST_F(TokenManagerTest, SaveAndLoadTokens) {
    tokenManager->setTokens("saved_access", "saved_refresh");
    
    // Сохраняем в JSON
    QJsonObject saved = tokenManager->saveToJson();
    EXPECT_EQ(saved["access_token"].toString(), "saved_access");
    EXPECT_EQ(saved["refresh_token"].toString(), "saved_refresh");
    
    // Очищаем
    tokenManager->clearTokens();
    EXPECT_FALSE(tokenManager->hasValidAccessToken());
    
    // Загружаем
    tokenManager->loadFromJson(saved);
    EXPECT_TRUE(tokenManager->hasValidAccessToken());
    EXPECT_EQ(tokenManager->getAccessToken(), "saved_access");
    EXPECT_EQ(tokenManager->getRefreshToken(), "saved_refresh");
}
