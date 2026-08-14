// production_client/src/core/AppCore.h
#ifndef APPCORE_H
#define APPCORE_H

#include <memory>

namespace UsersService {
    class AuthService;
    class ApiClient;
    class TokenManager;
}

class AppCore {
public:
    AppCore();
    void init();
};

// Глобальные геттеры
std::shared_ptr<UsersService::AuthService> getAuthService();
std::shared_ptr<UsersService::ApiClient> getApiClient();

#endif // APPCORE_H
