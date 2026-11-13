import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Назначить ответственного"
    width: 450
    height: 230
    modal: true
    standardButtons: Dialog.NoButton
    anchors.centerIn: Overlay.overlay

    property string taskId: ""
    property int selectedUserId: 0

    function openForTask(tId) {
        taskId = tId
        selectedUserId = 0
        var task = projectController?.projectData?.taskModel?.getTask(tId)
        if (task && task.responsible) {
            var rid = parseInt(task.responsible)
            if (!isNaN(rid)) selectedUserId = rid
        }
        // Найти индекс в members
        var members = projectController ? projectController.projectMembers : []
        var idx = -1
        for (var i = 0; i < members.length; i++) {
            if (members[i].user_id === selectedUserId) { idx = i; break }
        }
        if (idx >= 0) memberCombo.currentIndex = idx
        else memberCombo.currentIndex = -1
        open()
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        Label { text: "Ответственный:"; Layout.fillWidth: true; font.pixelSize: 13 }

        ComboBox
        {
            id: memberCombo
            Layout.fillWidth: true
            Layout.preferredHeight: 35
            model: projectController ? projectController.projectMembers : []
            textRole: "full_name"
            valueRole: "user_id"
            onActivated: {
                var m = projectController.projectMembers[currentIndex]
                root.selectedUserId = m.user_id
            }
        }

        Item { Layout.fillHeight: true }

        RowLayout
        {
            Layout.fillWidth: true; spacing: 10
            Button { text: "Отмена"; Layout.fillWidth: true; Layout.preferredHeight: 35
                onClicked: root.close() }
            Button {
                text: "ОК"; Layout.fillWidth: true; Layout.preferredHeight: 35
                enabled: memberCombo.currentIndex >= 0
                onClicked: {
                    if (root.taskId && projectController && root.selectedUserId > 0) {
                        projectController.updateTaskResponsible(root.taskId, root.selectedUserId)
                        root.close()
                    }
                }
            }
        }
    }
}
