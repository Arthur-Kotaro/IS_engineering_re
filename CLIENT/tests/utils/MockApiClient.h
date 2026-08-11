#ifndef MOCKAPICLIENT_H
#define MOCKAPICLIENT_H

#include <gmock/gmock.h>
#include "userserviceclient/ApiClient.h"

namespace UsersService {

class MockApiClient : public ApiClient {
public:
    MockApiClient();
    
    MOCK_METHOD(void, setServerUrl, (const QString& host, int port), (override));
    MOCK_METHOD(void, setAuthToken, (const QString& token), (override));
    MOCK_METHOD(void, clearAuthToken, (), (override));
    MOCK_METHOD(void, get, (const QString& endpoint, ApiCallback callback), (override));
    MOCK_METHOD(void, post, (const QString& endpoint, const QJsonObject& data, ApiCallback callback), (override));
    MOCK_METHOD(void, put, (const QString& endpoint, const QJsonObject& data, ApiCallback callback), (override));
    MOCK_METHOD(void, del, (const QString& endpoint, ApiCallback callback), (override));
    MOCK_METHOD(bool, isOnline, (), (const, override));
};

} // namespace UsersService

#endif // MOCKAPICLIENT_H
