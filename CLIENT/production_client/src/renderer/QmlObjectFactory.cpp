#include "QmlObjectFactory.h"
#include "qml/GeneratorHelpers.h"
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QDebug>
#include <QJsonArray>

// Внешние объявления генераторов
extern QString generateLabel(const QJsonObject&);
extern QString generatePushButton(const QJsonObject&);
extern QString generateComboBox(const QJsonObject&);
extern QString generateLineEdit(const QJsonObject&);
extern QString generateCheckBox(const QJsonObject&);
extern QString generateRadioButton(const QJsonObject&);
extern QString generateProgressBar(const QJsonObject&);
extern QString generateDateEdit(const QJsonObject&);
extern QString generateDateTimeEdit(const QJsonObject&);
extern QString generateCalendarWidget(const QJsonObject&);
extern QString generateTextEdit(const QJsonObject&);
extern QString generateSlider(const QJsonObject&);
extern QString generateSpinBox(const QJsonObject&);
extern QString generateListWidget(const QJsonObject&);
extern QString generateTreeWidget(const QJsonObject&);
extern QString generateFrame(const QJsonObject&);
extern QString generateSplitter(const QJsonObject&);
extern QString generateToolBox(const QJsonObject&);
extern QString generateTabWidget(const QJsonObject&);
extern QString generateStackedWidget(const QJsonObject&);
extern QString generateToolButton(const QJsonObject&);

extern QString generateVBoxLayout(const QJsonObject&);
extern QString generateHBoxLayout(const QJsonObject&);
extern QString generateGridLayout(const QJsonObject&);
extern QString generateGroupBox(const QJsonObject&);

extern QString generateTableWidget(const QJsonObject&);
extern QString generatePieChart(const QJsonObject&);
extern QString generateBarChart(const QJsonObject&);
extern QString generateBarCompareChart(const QJsonObject&);
extern QString generateLineChart(const QJsonObject&);

QmlObjectFactory::QmlObjectFactory(QQmlEngine* engine, QObject* parent)
    : QObject(parent)
    , m_engine(engine)
{
    qDebug() << "QmlObjectFactory::QmlObjectFactory()";
    registerBuiltInTypes();
    qDebug() << "  Registered" << m_qmlGenerators.size() << "widget types";
}

void QmlObjectFactory::registerBuiltInTypes()
{
    qDebug() << "  Registering built-in widget types...";

    // ============================================================
    // БАЗОВЫЕ ВИДЖЕТЫ
    // ============================================================
    m_qmlGenerators["QLabel"] = [](const QJsonObject& spec) { return generateLabel(spec); };
    m_qmlGenerators["QPushButton"] = [](const QJsonObject& spec) { return generatePushButton(spec); };
    m_qmlGenerators["QComboBox"] = [](const QJsonObject& spec) { return generateComboBox(spec); };
    m_qmlGenerators["QLineEdit"] = [](const QJsonObject& spec) { return generateLineEdit(spec); };
    m_qmlGenerators["QCheckBox"] = [](const QJsonObject& spec) { return generateCheckBox(spec); };
    m_qmlGenerators["QRadioButton"] = [](const QJsonObject& spec) { return generateRadioButton(spec); };
    m_qmlGenerators["QProgressBar"] = [](const QJsonObject& spec) { return generateProgressBar(spec); };
    m_qmlGenerators["QDateEdit"] = [](const QJsonObject& spec) { return generateDateEdit(spec); };
    m_qmlGenerators["QDateTimeEdit"] = [](const QJsonObject& spec) { return generateDateTimeEdit(spec); };
    m_qmlGenerators["QCalendarWidget"] = [](const QJsonObject& spec) { return generateCalendarWidget(spec); };
    m_qmlGenerators["QTextEdit"] = [](const QJsonObject& spec) { return generateTextEdit(spec); };
    m_qmlGenerators["QPlainTextEdit"] = [](const QJsonObject& spec) { return generateTextEdit(spec); };
    m_qmlGenerators["QSlider"] = [](const QJsonObject& spec) { return generateSlider(spec); };
    m_qmlGenerators["QSpinBox"] = [](const QJsonObject& spec) { return generateSpinBox(spec); };
    m_qmlGenerators["QListWidget"] = [](const QJsonObject& spec) { return generateListWidget(spec); };
    m_qmlGenerators["QTreeWidget"] = [](const QJsonObject& spec) { return generateTreeWidget(spec); };
    m_qmlGenerators["QFrame"] = [](const QJsonObject& spec) { return generateFrame(spec); };
    m_qmlGenerators["QSplitter"] = [](const QJsonObject& spec) { return generateSplitter(spec); };
    m_qmlGenerators["QToolBox"] = [](const QJsonObject& spec) { return generateToolBox(spec); };
    m_qmlGenerators["QTabWidget"] = [](const QJsonObject& spec) { return generateTabWidget(spec); };
    m_qmlGenerators["QStackedWidget"] = [](const QJsonObject& spec) { return generateStackedWidget(spec); };
    m_qmlGenerators["QToolButton"] = [](const QJsonObject& spec) { return generateToolButton(spec); };

    // ============================================================
    // LAYOUT-КОНТЕЙНЕРЫ
    // ============================================================
    m_qmlGenerators["QVBoxLayout"] = [](const QJsonObject& spec) { return generateVBoxLayout(spec); };
    m_qmlGenerators["QHBoxLayout"] = [](const QJsonObject& spec) { return generateHBoxLayout(spec); };
    m_qmlGenerators["QGridLayout"] = [](const QJsonObject& spec) { return generateGridLayout(spec); };
    m_qmlGenerators["QGroupBox"] = [](const QJsonObject& spec) { return generateGroupBox(spec); };

    // ============================================================
    // ДИАГРАММЫ
    // ============================================================
    m_qmlGenerators["ChartPie"] = [](const QJsonObject& spec) { return generatePieChart(spec); };
    m_qmlGenerators["ChartBar"] = [](const QJsonObject& spec) { return generateBarChart(spec); };
    m_qmlGenerators["ChartBarCompare"] = [](const QJsonObject& spec) { return generateBarCompareChart(spec); };
    m_qmlGenerators["ChartLine"] = [](const QJsonObject& spec) { return generateLineChart(spec); };

    // ============================================================
    // ТАБЛИЦЫ
    // ============================================================
    m_qmlGenerators["QTableWidget"] = [](const QJsonObject& spec) { return generateTableWidget(spec); };

    qDebug() << "  All widget types registered. Total:" << m_qmlGenerators.size();
}

QObject* QmlObjectFactory::create(const QString& type, const QJsonObject& spec, QQuickItem* parent)
{
    qDebug() << "    QmlObjectFactory::create(" << type << ") parent=" << (parent ? "valid" : "null")
             << "parent class:" << (parent ? parent->metaObject()->className() : "null");

    if (!m_qmlGenerators.contains(type)) {
        qWarning() << "      UNKNOWN widget type:" << type;
        return nullptr;
    }

    QString qmlCode = m_qmlGenerators[type](spec);

    qDebug() << "      QML code length:" << qmlCode.length() << "bytes";

    QString fullQml = QString(
        "import QtQuick 6.0\n"
        "import QtQuick.Controls 6.0\n"
        "import QtQuick.Layouts 6.0\n"
        "%1\n"
    ).arg(qmlCode);

    QQmlComponent component(m_engine);
    component.setData(fullQml.toUtf8(), QUrl());

    if (component.isError()) {
        qWarning() << "      QML component error:" << component.errorString();
        return nullptr;
    }

    QObject* object = component.create();
    if (!object) {
        qWarning() << "      Failed to create widget:" << component.errorString();
        return nullptr;
    }

    QQuickItem* item = qobject_cast<QQuickItem*>(object);
    if (item && parent) {
        item->setParentItem(parent);
        qDebug() << "      Widget parented to parentItem (setParentItem)";
    } else if (item) {
        qDebug() << "      Widget created WITHOUT parent";
    }

    qDebug() << "      Widget created SUCCESSFULLY";

    return object;
}
