import QtQuick 6.0

ListView {
    property var projects: []
    model: projects
    delegate: Text { text: modelData.name || "" }
}
