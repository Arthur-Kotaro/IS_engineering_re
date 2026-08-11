#ifndef NOTIFICATIONBRIDGE_H
#define NOTIFICATIONBRIDGE_H

#include <QObject>

class NotificationBridge : public QObject {
    Q_OBJECT
public:
    explicit NotificationBridge(QObject* parent = nullptr);
};

#endif // NOTIFICATIONBRIDGE_H
