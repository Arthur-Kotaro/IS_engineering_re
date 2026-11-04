#include "ExportManager.h"
#include "ProjectData.h"

#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QQuickWindow>

ExportManager::ExportManager(QObject *parent) : QObject(parent) {}

ExportManager::~ExportManager()
{
    if (m_process)
    {
        m_process->kill();
        m_process->deleteLater();
    }
}

void ExportManager::setMainWindow(QQuickWindow* window)
{
    m_mainWindow = window;
}

void ExportManager::setExportSettings(const QVariantMap& settings)
{
    m_exportSettings = settings;
}

bool ExportManager::exportToPng(ProjectData* project, const QString& filePath)
{
    if (!project) { emit exportFinished(false, "Нет данных проекта"); return false; }
    if (m_process) { emit exportFinished(false, "Экспорт уже выполняется"); return false; }

    QString outPath = filePath;
    if (!outPath.endsWith(".png", Qt::CaseInsensitive)) outPath += ".png";

    startExportProcess(project, outPath, false);
    return true;
}

bool ExportManager::exportToPdf(ProjectData* project, const QString& filePath, const QDate& exportDate)
{
    Q_UNUSED(exportDate)
    if (!project) { emit exportFinished(false, "Нет данных проекта"); return false; }
    if (m_process) { emit exportFinished(false, "Экспорт уже выполняется"); return false; }

    QString outPath = filePath;
    if (!outPath.endsWith(".pdf", Qt::CaseInsensitive)) outPath += ".pdf";

    startExportProcess(project, outPath, true);
    return true;
}

void ExportManager::startExportProcess(ProjectData* project, const QString& filePath, bool pdf)
{
    qDebug() << "=== ExportManager: STARTING EXPORT PROCESS ===";
    
    QVariantMap json = project->toJson();
    if (!m_exportSettings.isEmpty()) json["exportSettings"] = m_exportSettings;
    QByteArray jsonBytes = QJsonDocument::fromVariant(json).toJson(QJsonDocument::Compact);
    qDebug() << "JSON size:" << jsonBytes.size() << "bytes";

    QString rendererPath = QCoreApplication::applicationDirPath() + "/export_renderer";
#ifdef Q_OS_WIN
    rendererPath += ".exe";
#endif

    qDebug() << "Looking for renderer at:" << rendererPath;
    if (!QFile::exists(rendererPath))
    {
        qWarning() << "FAILED: export_renderer not found at" << rendererPath;
        emit exportFinished(false, "export_renderer не найден");
        return;
    }
    qDebug() << "Renderer binary found.";

    m_pendingFilePath = filePath;

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::SeparateChannels);

    connect(m_process, &QProcess::finished, this, &ExportManager::onProcessFinished);
    connect(m_process, QOverload<QProcess::ProcessError>::of(&QProcess::errorOccurred),
            this, &ExportManager::onProcessError);

    qDebug() << "Starting process...";
    m_process->start(rendererPath, QStringList());

    if (!m_process->waitForStarted(3000))
    {
        qWarning() << "FAILED: Process did not start within 3 seconds. Error:" << m_process->errorString();
        m_process->deleteLater();
        m_process = nullptr;
        emit exportFinished(false, "Не удалось запустить export_renderer");
        return;
    }
    qDebug() << "Process started successfully. PID:" << m_process->processId();

    // Передача данных в pipe
    qDebug() << "Writing to stdin (pipe)...";
    {
        QDataStream out(m_process);
        out.setVersion(QDataStream::Qt_6_0);
        out << quint32(jsonBytes.size());
        out.writeRawData(jsonBytes.constData(), jsonBytes.size());
        out << quint8(pdf ? 1 : 0);
    }
    m_process->closeWriteChannel();
    qDebug() << "Pipe closed. Waiting for process to finish...";
}

void ExportManager::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    QByteArray stderrData = m_process->readAllStandardError();
    QByteArray stdoutData = m_process->readAllStandardOutput();
    QString outPath = m_pendingFilePath;

    m_process->deleteLater();
    m_process = nullptr;
    m_pendingFilePath.clear();

    qDebug() << "=== ExportManager: PROCESS FINISHED ===";
    qDebug() << "Exit code:" << exitCode << "Status:" << status;
    qDebug() << "stdout size:" << stdoutData.size() << "bytes";
    qDebug() << "stderr output:";
    qDebug().noquote() << QString::fromUtf8(stderrData);

    if (status != QProcess::NormalExit || exitCode != 0)
    {
        emit exportFinished(false, "Ошибка экспорта (код " + QString::number(exitCode) + "). Подробности в консоли.");
        return;
    }

    QDataStream in(&stdoutData, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_6_0);

    quint32 dataSize = 0;
    in >> dataSize;
    if (dataSize == 0 || dataSize > 500 * 1024 * 1024)
    {
        qWarning() << "FAILED: invalid dataSize from pipe:" << dataSize;
        emit exportFinished(false, "Некорректный размер данных");
        return;
    }

    QByteArray fileBytes;
    fileBytes.resize(dataSize);
    if (in.readRawData(fileBytes.data(), dataSize) != static_cast<int>(dataSize))
    {
        qWarning() << "FAILED: could not read all bytes from pipe";
        emit exportFinished(false, "Ошибка чтения данных из pipe");
        return;
    }

    QFile out(outPath);
    QDir().mkpath(QFileInfo(outPath).absolutePath());
    if (!out.open(QIODevice::WriteOnly))
    {
        qWarning() << "FAILED: cannot open file for writing:" << outPath;
        emit exportFinished(false, "Не удалось сохранить файл");
        return;
    }
    out.write(fileBytes);
    out.close();

    qDebug() << "File saved successfully to:" << outPath;
    emit exportFinished(true, "Экспорт выполнен");
}

void ExportManager::onProcessError(QProcess::ProcessError error)
{
    qWarning() << "=== ExportManager: PROCESS ERROR ===" << error;
    if (m_process)
    {
        qWarning() << "Error string:" << m_process->errorString();
        m_process->deleteLater();
        m_process = nullptr;
    }
    m_pendingFilePath.clear();
    emit exportFinished(false, "Ошибка процесса экспорта");
}
