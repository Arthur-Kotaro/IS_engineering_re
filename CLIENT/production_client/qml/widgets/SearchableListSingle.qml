import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Styles 1.0

ColumnLayout {
    id: root

    property string label: ""
    property string fieldId: ""
    property bool required: false
    property var options: []
    property var currentValue: null
    property var filteredOptions: []

    spacing: 4

    Text {
        visible: root.label !== ""
        text: root.label + (root.required ? " *" : "")
        color: Colors.text
        font.pixelSize: GlobalSettings.fontSize
        Layout.fillWidth: true
    }

    TextField {
        id: filterField
        placeholderText: "Фильтр..."
        placeholderTextColor: Colors.textSecondary
        color: Colors.text
        selectionColor: Colors.primary
        font.pixelSize: GlobalSettings.fontSize
        Layout.fillWidth: true
        Layout.preferredHeight: 36

        background: Rectangle {
            color: Colors.surface
            border.color: Colors.border
            border.width: 1
            radius: 4
        }

        onTextChanged: root.rebuildFiltered()
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 300
        color: Colors.surface
        border.color: Colors.border
        border.width: 1
        radius: 4

        ListView {
            id: listView
            anchors.fill: parent
            anchors.margins: 4
            clip: true
            model: root.filteredOptions
            spacing: 2

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            delegate: Item {
                id: delegateRoot
                width: listView.width
                height: 32

                property var rowData: modelData

                Rectangle {
                    id: radioCircle
                    width: 18
                    height: 18
                    radius: 9
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 6
                    color: "transparent"
                    border.color: {
                        if (!delegateRoot.rowData) return Colors.border
                        return (root.currentValue !== null
                                && String(root.currentValue) === String(delegateRoot.rowData.value))
                                ? Colors.button : Colors.border
                    }
                    border.width: 2

                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        anchors.centerIn: parent
                        color: Colors.button
                        visible: delegateRoot.rowData
                                 && root.currentValue !== null
                                 && String(root.currentValue) === String(delegateRoot.rowData.value)
                    }
                }

                Text {
                    anchors.left: radioCircle.right
                    anchors.leftMargin: 8
                    anchors.right: parent.right
                    anchors.rightMargin: 6
                    anchors.verticalCenter: parent.verticalCenter
                    text: delegateRoot.rowData ? delegateRoot.rowData.label : ""
                    color: Colors.text
                    font.pixelSize: GlobalSettings.fontSize
                    elide: Text.ElideRight
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (!delegateRoot.rowData) return
                        root.currentValue = delegateRoot.rowData.value
                        if (widgetBridge) {
                            widgetBridge.sendWidgetInput(root.fieldId, {
                                "type": "select",
                                "value": delegateRoot.rowData.value,
                                "paramName": root.fieldId
                            })
                        }
                    }
                }
            }
        }

        Text {
            anchors.centerIn: parent
            visible: root.filteredOptions.length === 0
            text: "Нет совпадений"
            color: Colors.textSecondary
            font.pixelSize: GlobalSettings.fontSize
        }
    }

    function rebuildFiltered() {
        var q = filterField.text.toLowerCase()
        var arr = []
        for (var i = 0; i < root.options.length; i++) {
            var opt = root.options[i]
            if (!opt) continue
            var lbl = String(opt.label || "")
            if (q === "" || lbl.toLowerCase().indexOf(q) >= 0) {
                arr.push({ "value": opt.value, "label": lbl })
            }
        }
        root.filteredOptions = arr
    }

    onOptionsChanged: rebuildFiltered()

    Component.onCompleted: rebuildFiltered()
}
