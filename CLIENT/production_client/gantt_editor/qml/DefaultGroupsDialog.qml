import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Создать группы по умолчанию"
    width: 600
    height: Math.min(800, contentColumn.implicitHeight + 40)
    modal: true
    standardButtons: Dialog.NoButton
    anchors.centerIn: Overlay.overlay

    property var groups: []
    property var selectedIds: []

    onOpened: {
        groups = []
        selectedIds = []
        repeater.model = []
        if (projectController)
            projectController.loadDefaultGroups()
    }

    Connections
    {
        target: projectController
        function onDefaultGroupsLoaded(list) {
            root.groups = list
            var ids = []
            for (var i = 0; i < list.length; i++)
                ids.push(list[i].group_id)
            root.selectedIds = ids
            selectAllBox.checked = true
            repeater.model = list
        }
    }

    ColumnLayout
    {
        id: contentColumn
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10

        CheckBox
        {
            id: selectAllBox
            text: "Выбрать все"
            checked: true
            onCheckedChanged: {
                var ids = []
                if (checked) {
                    for (var i = 0; i < root.groups.length; i++)
                        ids.push(root.groups[i].group_id)
                }
                root.selectedIds = ids
                repeater.model = null
                repeater.model = root.groups
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: "#cccccc" }

        ScrollView
        {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(600, groupListLayout.implicitHeight + 20)
            clip: true

            ColumnLayout
            {
                id: groupListLayout
                width: parent.width
                spacing: 4

                Repeater
                {
                    id: repeater
                    model: []

                    delegate: CheckBox
                    {
                        text: modelData.name + " (" + modelData.group_id + ")"
                        checked: root.selectedIds.indexOf(modelData.group_id) >= 0
                        onCheckedChanged: {
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

        RowLayout
        {
            Layout.alignment: Qt.AlignRight
            spacing: 10

            Button
            {
                text: "Отмена"
                onClicked: root.close()
            }

            Button
            {
                text: "Создать"
                enabled: root.selectedIds.length > 0
                onClicked: {
                    if (projectController)
                        projectController.createGroupsWithPosition(root.selectedIds, "top", "")
                    root.close()
                }
            }
        }
    }
}
