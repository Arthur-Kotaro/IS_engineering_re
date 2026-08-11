#ifndef ENGINEERINGCONSOLEUI_H
#define ENGINEERINGCONSOLEUI_H

#include <QObject>

class EngineeringConsoleUI : public QObject
{
    Q_OBJECT

public:
    explicit EngineeringConsoleUI(const QString& host, int port, QObject* parent = nullptr);
    ~EngineeringConsoleUI();

public slots:
    void run();

private:
    QString m_host;
    int m_port;
};

#endif // ENGINEERINGCONSOLEUI_H
