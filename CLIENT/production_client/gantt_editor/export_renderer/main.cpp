#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickItem>
#include <QImage>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QBuffer>
#include <QFile>
#include <QTemporaryDir>
#include <QPrinter>
#include <QPainter>
#include <QPageSize>
#include <QDebug>
#include <QTimer>
#include <cstdio>
#include <cstdarg>

#include "ProjectController.h"
#include "ProjectData.h"

static FILE* g_logFile = nullptr;

void logMsg(const char* format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    fprintf(stderr, "[export_renderer] %s\n", buffer);
    fflush(stderr);

    if (g_logFile)
    {
        fprintf(g_logFile, "[export_renderer] %s\n", buffer);
        fflush(g_logFile);
    }
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("GanttProjectExport");

    QString logPath = QCoreApplication::applicationDirPath() + "/export_renderer.log";
    g_logFile = fopen(logPath.toUtf8().constData(), "w");
    logMsg("=== EXPORT RENDERER STARTED ===");

    QQuickWindow::setSceneGraphBackend(QStringLiteral("software"));
    logMsg("Software backend requested.");

    // 1. Читаем stdin
    QFile in;
    if (!in.open(stdin, QIODevice::ReadOnly)) { logMsg("FAILED: stdin"); return 1; }
    QDataStream ds(&in);
    ds.setVersion(QDataStream::Qt_6_0);
    quint32 jsonSize = 0; ds >> jsonSize;
    if (jsonSize == 0 || jsonSize > 100 * 1024 * 1024) { logMsg("FAILED: json size"); return 1; }
    QByteArray jsonBytes; jsonBytes.resize(jsonSize);
    if (ds.readRawData(jsonBytes.data(), jsonSize) != (int)jsonSize) { logMsg("FAILED: json read"); return 1; }
    quint8 formatFlag = 0; ds >> formatFlag;

    QJsonDocument doc = QJsonDocument::fromJson(jsonBytes);
    if (doc.isNull() || !doc.isObject()) { logMsg("FAILED: json parse"); return 1; }
    QVariantMap projectMap = doc.object().toVariantMap();
    QVariantMap exportSettings = projectMap.take("exportSettings").toMap();

    // 2. Данные
    ProjectController controller;
    if (!controller.get_projectData()->fromSerializerJson(projectMap))
    {
        logMsg("FAILED: fromSerializerJson");
        return 1;
    }
    logMsg("Project data loaded.");

    // 3. QML
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("projectController", &controller);
    engine.load(QUrl(QStringLiteral("qrc:/qml/ExportView.qml")));
    if (engine.rootObjects().isEmpty()) { logMsg("FAILED: qml load"); return 1; }

    QQuickWindow* window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
    if (!window) { logMsg("FAILED: root not window"); return 1; }

    // Принудительно создаём окно (backing store), даже если оно невидимо
    window->create();
    logMsg("Window created (invisible).");

    QQuickItem* contentArea = window->findChild<QQuickItem*>("contentArea");
    if (!contentArea) { logMsg("FAILED: contentArea not found"); return 1; }
    logMsg("contentArea found.");

    QMetaObject::invokeMethod(window, "applySettings", Q_ARG(QVariant, exportSettings));

    // 4. Ждём, пока layout устаканится, затем меняем размер окна и снимаем
    QTimer::singleShot(500, [&]() {
        logMsg("Timer fired. Preparing to grab...");

        int fullW = qCeil(contentArea->width());
        int fullH = qCeil(contentArea->height());
        logMsg("Full content size: %d x %d", fullW, fullH);

        if (fullW < 10 || fullH < 10) { logMsg("FAILED: invalid size"); QCoreApplication::exit(1); return; }

        // Меняем размер окна на полный размер графика
        window->resize(fullW, fullH);
        logMsg("Window resized to %d x %d", fullW, fullH);

        // Даём Qt время перерисовать окно в новом размере
        QTimer::singleShot(500, [&]() {
            logMsg("Grabbing window...");
            QImage image = window->grabWindow();
            if (image.isNull()) {
                logMsg("FAILED: grabWindow returned null");
                QCoreApplication::exit(1);
                return;
            }
            logMsg("Image grabbed: %d x %d", image.width(), image.height());

            // 5. Сохранение
            QByteArray outBytes;
            if (formatFlag == 0) {
                logMsg("Saving to PNG...");
                QBuffer buffer(&outBytes);
                buffer.open(QIODevice::WriteOnly);
                image.save(&buffer, "PNG");
                buffer.close();
            } else {
                logMsg("Saving to PDF...");
                QTemporaryDir tmpDir;
                if (!tmpDir.isValid()) { logMsg("FAILED: temp dir"); QCoreApplication::exit(1); return; }
                QString pdfPath = tmpDir.path() + "/export.pdf";
                QPrinter printer(QPrinter::HighResolution);
                printer.setOutputFormat(QPrinter::PdfFormat);
                printer.setOutputFileName(pdfPath);
                qreal wMm = image.width() / 96.0 * 25.4;
                qreal hMm = image.height() / 96.0 * 25.4;
                printer.setPageSize(QPageSize(QSizeF(wMm, hMm), QPageSize::Millimeter));
                QPainter painter(&printer);
                if (!painter.isActive()) { logMsg("FAILED: pdf painter"); QCoreApplication::exit(1); return; }
                painter.drawImage(QRectF(0, 0, printer.width(), printer.height()), image);
                painter.end();
                QFile pdfRead(pdfPath);
                if (pdfRead.open(QIODevice::ReadOnly)) { outBytes = pdfRead.readAll(); pdfRead.close(); }
            }

            if (outBytes.isEmpty()) { logMsg("FAILED: out empty"); QCoreApplication::exit(1); return; }

            QFile out;
            if (!out.open(stdout, QIODevice::WriteOnly)) { logMsg("FAILED: stdout"); QCoreApplication::exit(1); return; }
            QDataStream outDs(&out);
            outDs.setVersion(QDataStream::Qt_6_0);
            outDs << quint32(outBytes.size());
            outDs.writeRawData(outBytes.constData(), outBytes.size());
            out.flush();
            logMsg("=== EXPORT SUCCESSFUL ===");
            QCoreApplication::exit(0);
        });
    });

    logMsg("Entering app.exec()...");
    int ret = app.exec();
    logMsg("app.exec() returned: %d", ret);
    if (g_logFile) fclose(g_logFile);
    return ret;
}
