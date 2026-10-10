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

    spacing: 4

    // ---- Заголовок ----
    Text {
        visible: root.label !== ""
        text: root.label + (root.required ? " *" : "")
        color: Colors.text
        font.pixelSize: GlobalSettings.fontSize
        Layout.fillWidth: true
    }

    // ---- Строка фильтра ----
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
    }

    // ---- Список с множественным выбором ----
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
            model: filteredModel

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            delegate: CheckBox {
                id: checkDelegate
                width: listView.width
                text: modelData.label
                font.pixelSize: GlobalSettings.fontSize
                checked: root.isSelected(modelData.value)

                onCheckedChanged: {
                    root.toggleValue(modelData.value, checked)
                }
            }
        }

        Text {
            anchors.centerIn: parent
            visible: filteredModel.count === 0
            text: "Нет совпадений"
            color: Colors.textSecondary
            font.pixelSize: GlobalSettings.fontSize
        }
    }

    // ---- Модель с фильтром ----
    ListModel {
        id: filteredModel
    }

    function rebuildFilteredModel() {
        filteredModel.clear()
        var q = filterField.text.toLowerCase()
        for (var i = 0; i < root.options.length; i++) {
            var opt = root.options[i]
            var lbl = String(opt.label || "")
            if (q === "" || lbl.toLowerCase().indexOf(q) >= 0) {
                filteredModel.append({ "value": opt.value, "label": lbl })
            }
        }
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

        if (widgetBridge) {
            widgetBridge.sendWidgetInput(root.fieldId, {
                "type": "multiselect",
                "value": arr,
                "paramName": root.fieldId
            })
        }
    }

    onOptionsChanged: rebuildFilteredModel()

    Connections {
        target: filterField
        function onTextChanged() {
            root.rebuildFilteredModel()
        }
    }

    Component.onCompleted: rebuildFilteredModel()
}
