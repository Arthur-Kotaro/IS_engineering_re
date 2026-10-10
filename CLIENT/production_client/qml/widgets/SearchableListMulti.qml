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
    property var selectedValues: []
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
                property bool isChecked: root.isSelected(rowData ? rowData.value : null)

                Rectangle {
                    id: checkBox
                    width: 18
                    height: 18
                    radius: 3
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 6
                    color: delegateRoot.isChecked ? Colors.button : "transparent"
                    border.color: delegateRoot.isChecked ? Colors.button : Colors.border
                    border.width: 2

                    Text {
                        anchors.centerIn: parent
                        text: "✓"
                        color: Colors.buttonText
                        font.pixelSize: 12
                        font.bold: true
                        visible: delegateRoot.isChecked
                    }
                }

                Text {
                    anchors.left: checkBox.right
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
                        root.toggleValue(delegateRoot.rowData.value, !delegateRoot.isChecked)
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

    function isSelected(value) {
        if (!root.selectedValues) return false
        for (var i = 0; i < root.selectedValues.length; i++) {
            if (String(root.selectedValues[i]) === String(value)) return true
        }
        return false
    }

    function toggleValue(value, checked) {
        var arr = (root.selectedValues || []).slice()
        var idx = -1
        for (var i = 0; i < arr.length; i++) {
            if (String(arr[i]) === String(value)) { idx = i; break }
        }
        if (checked && idx < 0) {
            arr.push(value)
        } else if (!checked && idx >= 0) {
            arr.splice(idx, 1)
        }
        root.selectedValues = arr

        // Обновляем делегаты вручную (JS-массив не пересоздаёт делегаты при изменении selectedValues)
        for (var j = 0; j < listView.count; j++) {
            var item = listView.itemAtIndex(j)
            if (item) item.isChecked = root.isSelected(item.rowData ? item.rowData.value : null)
        }

        if (widgetBridge) {
            widgetBridge.sendWidgetInput(root.fieldId, {
                "type": "multiselect",
                "value": arr,
                "paramName": root.fieldId
            })
        }
    }

    onOptionsChanged: rebuildFiltered()

    Component.onCompleted: rebuildFiltered()
}
