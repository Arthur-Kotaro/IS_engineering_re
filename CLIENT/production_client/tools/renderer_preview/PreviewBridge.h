#ifndef PREVIEWBRIDGE_H
#define PREVIEWBRIDGE_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QStringList>
#include <memory>

class QQmlEngine;
class JsonUiRenderer;

class PreviewBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentFile READ currentFile NOTIFY fileChanged)
    Q_PROPERTY(QStringList sampleFiles READ sampleFiles NOTIFY sampleFilesChanged)

public:
    explicit PreviewBridge(QQmlEngine* engine, QObject* parent = nullptr);
    ~PreviewBridge();

    QString currentFile() const { return m_currentFile; }
    QStringList sampleFiles() const { return m_sampleFiles; }

    Q_INVOKABLE void loadFile(const QString& path);
    Q_INVOKABLE void reload();
    Q_INVOKABLE void refreshSamples();
    Q_INVOKABLE void openFileDialog();

signals:
    void fileChanged(const QString& path);
    void renderComplete();
    void errorOccurred(const QString& message);
    void sampleFilesChanged();
    void widgetCountChanged(int count);

private slots:
    void onFileChanged(const QString& path);

private:
    void renderJson();
    QString findSamplesDirectory() const;
    void scanSamples();

    QQmlEngine* m_engine;
    JsonUiRenderer* m_renderer;
    QFileSystemWatcher* m_watcher;
    QString m_currentFile;
    QStringList m_sampleFiles;
    bool m_isLoading;
};

#endif // PREVIEWBRIDGE_H
