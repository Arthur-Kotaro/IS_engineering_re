#include "ConfigManager.h"

ConfigManager::ConfigManager() {}

QString ConfigManager::getServerHost() const {
    return "localhost";
}

int ConfigManager::getServerPort() const {
    // Изменяем порт на 8080 (Gateway)
    return 8080;
}
