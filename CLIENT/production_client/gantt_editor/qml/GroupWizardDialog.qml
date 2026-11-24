import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Создать группу"
    width: 600
    height: 500
    modal: true
    standardButtons: Dialog.NoButton
    anchors.centerIn: Overlay.overlay

    property string position: "bottom"
    property string afterGroupId: ""
    property string groupName: ""
    property int step: 1
    property int mode: 0   // 0 = стандартная, 1 = оригинальная
    property var defaultGroups: []
    property var existingGroupIds: []
    property var selectedIds: []

    function openWith(pos, afterId, afterName) {
        root.position = pos
        root.afterGroupId = afterId || ""
        root.groupName = afterName || ""
        root.step = 1
        root.mode = 0
        root.selectedIds = []
        root.defaultGroups = []
        // Загрузить стандартные группы
        if (projectController)
            projectController.loadDefaultGroups()
        // Существующие группы — из текущей модели
        if (projectController && projectController.projectData) {
            existingGroupIds = projectController.projectData.groupModel.getGroupIds()
        } else {
            existingGroupIds = []
        }
        root.open()
    }

    function allStandardCreated() {
        if (defaultGroups.length === 0) return false
        for (var i = 0; i < defaultGroups.length; i++) {
            if (existingGroupIds.indexOf(defaultGroups[i].group_id) < 0) return false
        }
        return true
    }

    function availableStandardGroups() {
        var result = []
        for (var i = 0; i < defaultGroups.length; i++) {
            var g = defaultGroups[i]
            result.push({
                group_id: g.group_id,
                name: g.name,
                exists: existingGroupIds.indexOf(g.group_id) >= 0
            })
        }
        return result
    }

    Connections
    {
        target: projectController
        function onDefaultGroupsLoaded(list) {
            root.defaultGroups = list
            if (root.allStandardCreated()) {
                root.mode = 1  // только оригинальная
            }
            root.selectedIds = []
            repeater.model = root.availableStandardGroups()
        }
        function onGroupCreated(success, message) {
            if (success) root.close()
        }
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 12

        // ---------- Шаг 1 ----------
        ColumnLayout
        {
            visible: root.step === 1
            Layout.fillWidth: true
            spacing: 10

            Label { text: "Тип группы:"; font.bold: true; font.pixelSize: 14 }

            RadioButton {
                text: "Создать стандартную группу задач"
                enabled: !root.allStandardCreated()
                checked: root.mode === 0
                onCheckedChanged: if (checked) root.mode = 0
            }
            RadioButton {
                text: "Создать оригинальную группу задач"
                checked: root.mode === 1
                onCheckedChanged: if (checked) root.mode = 1
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 10
                Button { text: "Отмена"; onClicked: root.close() }
                Button {
                    text: "Далее"
                    onClicked: {
                        if (root.mode === 0) {
                            root.step = 2
                            repeater.model = root.availableStandardGroups()
                        } else {
                            root.step = 3
                        }
                    }
                }
            }
        }

        // ---------- Шаг 2 (стандартная) ----------
        ColumnLayout
        {
            visible: root.step === 2
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            Label { text: "Выберите стандартные группы:"; font.bold: true }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                ColumnLayout {
                    width: parent.width
                    spacing: 4

                    Repeater {
                        id: repeater
                        model: []
                        delegate: CheckBox {
                            text: modelData.name + (modelData.exists ? " (уже создана)" : "")
                            enabled: !modelData.exists
                            checked: modelData.exists || root.selectedIds.indexOf(modelData.group_id) >= 0
                            onCheckedChanged: {
                                if (modelData.exists) return
                                var arr = root.selectedIds.slice()
                                var idx = arr.indexOf(modelData.group_id)
                                if (checked && idx < 0) arr.push(modelData.group_id)
                                else if (!checked && idx >= 0) arr.splice(idx, 1)
                                root.selectedIds = arr
                            }
                        }
                    }
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 10
                Button { text: "Назад"; onClicked: root.step = 1 }
                Button {
                    text: "Создать"
                    enabled: root.selectedIds.length > 0
                    onClicked: projectController.createGroupsWithPosition(root.selectedIds, root.position, root.afterGroupId)
                }
            }
        }

        // ---------- Шаг 3 (оригинальная) ----------
        ColumnLayout
        {
            visible: root.step === 3
            Layout.fillWidth: true
            spacing: 10

            Label { text: "Название группы:"; font.bold: true }
            TextField {
                id: nameField
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                placeholderText: "Введите название"
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 10
                Button { text: "Назад"; onClicked: root.step = 1 }
                Button {
                    text: "Создать"
                    enabled: nameField.text !== ""
                    onClicked: projectController.createCustomGroupWithPosition(nameField.text, root.position, root.afterGroupId)
                }
            }
        }
    }
}
