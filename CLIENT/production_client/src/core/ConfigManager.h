#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>

class ConfigManager {
public:
    ConfigManager();
    QString getServerHost() const;
    int getServerPort() const;
};

#endif // CONFIGMANAGER_H
