#ifndef CACHEMANAGER_H
#define CACHEMANAGER_H

#include <QDateTime>
#include <QMap>

class CacheManager
{
  public:
    CacheManager();
    bool loadLastModifiedDates(QMap<int, QDateTime> &dates);
    bool saveLastModifiedDates(const QMap<int, QDateTime> &dates);
};

#endif // CACHEMANAGER_H
