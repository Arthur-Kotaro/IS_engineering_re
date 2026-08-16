#include "PreviewBridge.h"
#include "renderer/JsonUiRenderer.h"
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>
#include <QGuiApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <QTimer>
#include <QFileDialog>
#include <QCoreApplication>

PreviewBridge::PreviewBridge(QQmlEngine* engine, QObject* parent)
    : QObject(parent)
    , m_engine(engine)
    , m_renderer(new JsonUiRenderer(engine, this))
    , m_watcher(new QFileSystemWatcher(this))
    , m_isLoading(false)
{
    connect(m_watcher, &QFileSystemWatcher::fileChanged,
            this, &PreviewBridge::onFileChanged);

    connect(m_renderer, &JsonUiRenderer::renderFinished,
            this, [this]() {
                emit renderComplete();
                emit widgetCountChanged(m_renderer->widgetCount());
            });

    scanSamples();
}

PreviewBridge::~PreviewBridge() {}

QString PreviewBridge::findSamplesDirectory() const
{
    QStringList paths = {
        QCoreApplication::applicationDirPath() + "/samples",
        QCoreApplication::applicationDirPath() + "/../samples",
        QCoreApplication::applicationDirPath() + "/../../tools/renderer_preview/samples",
        QCoreApplication::applicationDirPath() + "/../../../tools/renderer_preview/samples"
    };

    for (const QString& path : paths) {
        if (QDir(path).exists()) {
            qDebug() << "Samples directory found:" << path;
            return path;
        }
    }

    qDebug() << "Samples directory not found, using current directory";
    return ".";
}

void PreviewBridge::scanSamples()
{
    m_sampleFiles.clear();
    QString samplesDir = findSamplesDirectory();

    QDirIterator it(samplesDir, QStringList() << "*.json", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        m_sampleFiles << it.filePath();
    }

    m_sampleFiles.sort();
    emit sampleFilesChanged();

    qDebug() << "Found" << m_sampleFiles.size() << "sample files";

    if (!m_sampleFiles.isEmpty() && m_currentFile.isEmpty()) {
        loadFile(m_sampleFiles.first());
    }
}

void PreviewBridge::refreshSamples()
{
    scanSamples();
}

void PreviewBridge::openFileDialog()
{
    QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        "Выберите JSON-файл",
        QDir::homePath(),
        "JSON files (*.json);;All files (*)"
    );

    if (!filePath.isEmpty()) {
        loadFile(filePath);
    }
}

void PreviewBridge::loadFile(const QString& path)
{
    if (m_isLoading) {
        return;
    }

    m_isLoading = true;

    QString filePath = path;
    if (filePath.startsWith("file://")) {
        filePath = QUrl(filePath).toLocalFile();
    }

    if (!QFile::exists(filePath)) {
        emit errorOccurred("File not found: " + filePath);
        m_isLoading = false;
        return;
    }

    if (!m_watcher->files().isEmpty()) {
        m_watcher->removePaths(m_watcher->files());
    }
    m_watcher->addPath(filePath);

    m_currentFile = filePath;
    emit fileChanged(filePath);

    renderJson();
}

void PreviewBridge::reload()
{
    if (m_currentFile.isEmpty()) {
        emit errorOccurred("No file loaded");
        return;
    }

    qDebug() << "Reloading:" << m_currentFile;
    renderJson();
}

void PreviewBridge::onFileChanged(const QString& path)
{
    QTimer::singleShot(300, this, [this, path]() {
        if (path == m_currentFile && QFile::exists(path)) {
            qDebug() << "File changed, reloading:" << path;
            renderJson();
        }
    });
}

void PreviewBridge::renderJson()
{
    if (m_currentFile.isEmpty()) {
        emit errorOccurred("No file loaded");
        m_isLoading = false;
        return;
    }

    QFile file(m_currentFile);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred("Cannot open file: " + m_currentFile);
        m_isLoading = false;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred("JSON parse error: " + parseError.errorString());
        m_isLoading = false;
        return;
    }

    if (!doc.isObject()) {
        emit errorOccurred("JSON must be an object");
        m_isLoading = false;
        return;
    }

    QJsonObject root = doc.object();

    QQuickItem* container = nullptr;

    if (QGuiApplication::instance()) {
        QList<QWindow*> windows = QGuiApplication::topLevelWindows();
        for (QWindow* window : windows) {
            QQuickWindow* quickWindow = qobject_cast<QQuickWindow*>(window);
            if (quickWindow) {
                container = quickWindow->findChild<QQuickItem*>("renderContainer");
                if (container) {
                    qDebug() << "Found renderContainer in QQuickWindow";
                    break;
                }
            }
        }
    }

    if (!container && m_engine) {
        QObject* rootObject = m_engine->rootContext()->contextObject();
        if (rootObject) {
            QQuickWindow* window = qobject_cast<QQuickWindow*>(rootObject);
            if (window) {
                container = window->findChild<QQuickItem*>("renderContainer");
                if (!container) {
                    container = window->contentItem();
                }
            } else {
                QQuickItem* rootItem = qobject_cast<QQuickItem*>(rootObject);
                if (rootItem) {
                    container = rootItem->findChild<QQuickItem*>("renderContainer");
                    if (!container) {
                        container = rootItem;
                    }
                }
            }
        }
    }

    if (!container) {
        emit errorOccurred("Cannot find QML container");
        m_isLoading = false;
        return;
    }

    auto children = container->childItems();
    for (QQuickItem* child : children) {
        child->deleteLater();
    }

    try {
        m_renderer->render(root, container);
        qDebug() << "Render completed successfully";
    } catch (const std::exception& e) {
        emit errorOccurred("Render error: " + QString::fromStdString(e.what()));
    } catch (...) {
        emit errorOccurred("Unknown render error");
    }

    m_isLoading = false;
}
