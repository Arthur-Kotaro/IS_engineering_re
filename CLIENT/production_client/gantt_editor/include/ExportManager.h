#ifndef EXPORTMANAGER_H
#define EXPORTMANAGER_H

#include <QObject>
#include <QString>
#include <QDate>
#include <QImage>
#include <QByteArray>
#include <QVariantMap>
#include <QProcess>

class ProjectData;
class QQuickWindow;

class ExportManager : public QObject
{
    Q_OBJECT

public:
    explicit ExportManager(QObject *parent = nullptr);
    ~ExportManager() override;

    void setMainWindow(QQuickWindow* window);
    Q_INVOKABLE void setExportSettings(const QVariantMap& settings);

    Q_INVOKABLE bool exportToPng(ProjectData* project, const QString& filePath);
    Q_INVOKABLE bool exportToPdf(ProjectData* project, const QString& filePath, const QDate& exportDate);

signals:
    void exportProgress(int percent);
    void exportFinished(bool success, const QString& message);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onProcessError(QProcess::ProcessError error);

private:
    QQuickWindow* m_mainWindow = nullptr;
    QVariantMap m_exportSettings;

    void startExportProcess(ProjectData* project, const QString& filePath, bool pdf);
    void finishExport(bool success, const QString& message);

    QProcess* m_process = nullptr;
    QString m_pendingFilePath;
};

#endif
