#include "CacheManager.h"

CacheManager::CacheManager() {}

bool CacheManager::loadLastModifiedDates(QMap<int, QDateTime>& dates) {
    Q_UNUSED(dates);
    return true;
}

bool CacheManager::saveLastModifiedDates(const QMap<int, QDateTime>& dates) {
    Q_UNUSED(dates);
    return true;
}
