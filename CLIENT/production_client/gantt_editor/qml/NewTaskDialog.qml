import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Создание новой задачи"
    width: 500
    height: 650
    modal: true
    standardButtons: Dialog.NoButton
    anchors.centerIn: Overlay.overlay

    property string groupId: ""
    property string taskTitle: "Новая задача"
    property int responsibleId: 0
    property date startDate: new Date()
    property date endDate: new Date()
    property string comment: ""
    property bool createWithoutDates: false
    property var conflictMessages: []

    function openForGroup(gId) {
        groupId = gId
        taskTitle = "Новая задача"
        responsibleId = 0
        startDate = new Date()
        endDate = new Date()
        endDate.setDate(endDate.getDate() + 7)
        comment = ""
        createWithoutDates = false
        conflictMessages = []
        withoutDatesCheck.checked = false
        titleField.text = taskTitle
        responsibleCombo.currentIndex = -1
        startDateField.text = Qt.formatDateTime(startDate, "dd.MM.yyyy")
        endDateField.text = Qt.formatDateTime(endDate, "dd.MM.yyyy")
        commentField.text = ""
        open()
    }

    function localCheckConflicts() {
        var result = []
        if (!projectController || !projectController.projectData) return result
        var gm = projectController.projectData.groupModel
        var groupName = gm.getGroup(groupId).name || groupId
        var taskModel = projectController.projectData.taskModel
        var taskIds = taskModel.getTasksForGroup(groupId)
        for (var i = 0; i < taskIds.length; i++) {
            var t = taskModel.getTask(taskIds[i])
            if (t && t.title === taskTitle) {
                result.push("Задача с именем \"" + taskTitle + "\" уже есть в группе \"" + groupName + "\"")
            }
        }
        return result
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        Label { text: "Название задачи:"; font.bold: true }
        TextField { id: titleField; Layout.fillWidth: true; text: root.taskTitle; onTextChanged: root.taskTitle = text }

        Label { text: "Ответственный:"; font.bold: true }
        ComboBox {
            id: responsibleCombo
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            model: projectController ? projectController.projectMembers : []
            textRole: "full_name"
            valueRole: "user_id"
            onActivated: {
                var m = projectController.projectMembers[currentIndex]
                root.responsibleId = m.user_id
            }
        }

        CheckBox {
            id: withoutDatesCheck
            text: "Создать без сроков"
            onCheckedChanged: root.createWithoutDates = checked
        }

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: 12
            visible: !root.createWithoutDates

            Label { text: "Дата начала:"; font.bold: true }
            TextField {
                id: startDateField
                Layout.fillWidth: true
                onEditingFinished: {
                    var parts = text.split(".")
                    if (parts.length === 3) {
                        var d = new Date(parseInt(parts[2]), parseInt(parts[1]) - 1, parseInt(parts[0]))
                        if (!isNaN(d.getTime())) root.startDate = d
                        else text = Qt.formatDateTime(root.startDate, "dd.MM.yyyy")
                    } else text = Qt.formatDateTime(root.startDate, "dd.MM.yyyy")
                }
            }

            Label { text: "Дата завершения:"; font.bold: true }
            TextField {
                id: endDateField
                Layout.fillWidth: true
                onEditingFinished: {
                    var parts = text.split(".")
                    if (parts.length === 3) {
                        var d = new Date(parseInt(parts[2]), parseInt(parts[1]) - 1, parseInt(parts[0]))
                        if (!isNaN(d.getTime())) root.endDate = d
                        else text = Qt.formatDateTime(root.endDate, "dd.MM.yyyy")
                    } else text = Qt.formatDateTime(root.endDate, "dd.MM.yyyy")
                }
            }
        }

        Label { text: "Комментарий:"; font.bold: true }
        TextField {
            id: commentField
            Layout.fillWidth: true
            placeholderText: "Введите комментарий"
            text: root.comment
            onTextChanged: root.comment = text
        }

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: 2
            Repeater {
                model: root.conflictMessages
                delegate: Label {
                    text: modelData; color: "#c62828"; font.pixelSize: 12
                    wrapMode: Text.WordWrap; Layout.fillWidth: true
                }
            }
        }

        Item { Layout.fillHeight: true }

        RowLayout
        {
            Layout.alignment: Qt.AlignRight
            spacing: 10
            Button { text: "Отмена"; onClicked: root.close() }
            Button {
                text: "Создать"
                onClicked: {
                    root.conflictMessages = root.localCheckConflicts()
                    if (root.conflictMessages.length > 0) return
                    var rid = root.responsibleId > 0 ? String(root.responsibleId) : ""
                    if (projectController.isLocalGraph) {
                        // ЛГ — через localGraphController.addLocalTask
                        var resp = root.responsibleId > 0 ? String(root.responsibleId) : "0"
                        var s = root.createWithoutDates ? new Date() : root.startDate
                        var e = root.createWithoutDates ? new Date() : root.endDate
                        projectController.localGraphController.addLocalTask(root.taskTitle, resp, s, e)
                    } else {
                        if (root.createWithoutDates)
                            projectController.createTaskWithoutDates(root.groupId, root.taskTitle, rid)
                            else
                                projectController.createTask(root.groupId, root.taskTitle, rid, root.startDate, root.endDate)
                    }
                    root.close()
                }
            }
        }
    }

    Connections
    {
        target: projectController
        function onTitleConflictDetected(conflicts) {
            var msgs = []
            for (var i = 0; i < conflicts.length; i++) {
                var c = conflicts[i]
                msgs.push("Задача с именем \"" + c.title + "\" уже есть в группе \"" +
                          (c.group_name || c.group_id) + "\"")
            }
            root.conflictMessages = msgs
        }
    }
}
