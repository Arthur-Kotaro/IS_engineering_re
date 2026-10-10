import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Styles 1.0

// Простой одиночный селект с фильтром (ComboBox).
// Пока не используется в форме создания пользователя,
// добавлен как часть базового набора.

ColumnLayout {
    id: root

    property string label: ""
    property string fieldId: ""
    property bool required: false
    property var options: []
    property var currentValue: null

    spacing: 4

    Text {
        visible: root.label !== ""
        text: root.label + (root.required ? " *" : "")
        color: Colors.text
        font.pixelSize: GlobalSettings.fontSize
        Layout.fillWidth: true
    }

    ComboBox {
        id: combo
        Layout.fillWidth: true
        Layout.preferredHeight: 36
        font.pixelSize: GlobalSettings.fontSize
        editable: true
        model: root.options
        textRole: "label"
        valueRole: "value"

        background: Rectangle {
            color: Colors.surface
            border.color: Colors.border
            border.width: 1
            radius: 4
        }

        contentItem: TextField {
            text: combo.editable ? combo.editText : combo.displayText
            color: Colors.text
            font.pixelSize: GlobalSettings.fontSize
            verticalAlignment: Text.AlignVCenter
            background: Item {}
        }

        onActivated: function(index) {
            if (index < 0) return
            root.currentValue = combo.valueAt(index)
            if (widgetBridge) {
                widgetBridge.sendWidgetInput(root.fieldId, {
                    "type": "select",
                    "value": root.currentValue,
                    "paramName": root.fieldId
                })
            }
        }
    }
}
