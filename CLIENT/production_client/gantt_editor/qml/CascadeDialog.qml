import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Создание каскада задач"
    width: 1380
    height: 700
    modal: true
    standardButtons: Dialog.NoButton
    anchors.centerIn: Overlay.overlay

    property int taskCount: 0
    property date startDate: new Date()
    property var cascadeModel: []
    property bool createWithoutDates: false
    property bool isLocalMode: projectController && projectController.projectData && projectController.projectData.graphKind === 1
    property var conflictMessages: []

    function groupNames() {
        if (!projectController || !projectController.projectData) return []
        return projectController.projectData.groupModel.getGroupIds().map(function(id) {
            return projectController.projectData.groupModel.getGroup(id).name
        })
    }
    function groupIds() {
        if (!projectController || !projectController.projectData) return []
        return projectController.projectData.groupModel.getGroupIds()
    }

    function buildCascade() {
        cascadeModel = []
        for (var i = 0; i < taskCount; i++) {
            cascadeModel.push({ groupIndex: 0, title: "Задача " + (i + 1),
                                 responsible: "", duration: 7, comment: "" })
        }
        cascadeRepeater.model = cascadeModel
    }

    function resetDialog() {
        taskCount = 0
        cascadeModel = []
        conflictMessages = []
        countField.text = "3"
        startDate = new Date()
        startDateField.text = Qt.formatDateTime(root.startDate, "dd.MM.yyyy")
        createWithoutDates = false
        withoutDatesCheck.checked = false
        cascadeRepeater.model = []
    }

    function localCheckConflicts() {
        var result = []
        if (!projectController || !projectController.projectData) return result
        var gm = projectController.projectData.groupModel
        var taskModel = projectController.projectData.taskModel
        var gIds = groupIds()
        var names = {}
        for (var i = 0; i < cascadeModel.length; i++) {
            var item = cascadeModel[i]
            var gid = gIds[item.groupIndex] || gIds[0]
            var key = gid + "||" + item.title
            if (names[key]) result.push("Дублирование имени \"" + item.title + "\" в каскаде")
            names[key] = true
            var existing = taskModel.getTasksForGroup(gid)
            for (var j = 0; j < existing.length; j++) {
                var t = taskModel.getTask(existing[j])
                if (t && t.title === item.title) {
                    var gname = gm.getGroup(gid).name || gid
                    result.push("Задача с именем \"" + item.title + "\" уже есть в группе \"" + gname + "\"")
                }
            }
        }
        return result
    }

    onOpened: { resetDialog(); countField.forceActiveFocus() }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10

        RowLayout
        {
            Layout.fillWidth: true
            spacing: 10

            Label { text: "Количество задач:" }
            TextField {
                id: countField
                Layout.preferredWidth: 60
                text: "3"
                validator: IntValidator { bottom: 1; top: 100 }
                onEditingFinished: { root.taskCount = parseInt(text) || 3; root.buildCascade() }
            }

            Label { text: "Дата начала:"; visible: !root.createWithoutDates }
            TextField {
                id: startDateField
                Layout.preferredWidth: 100
                visible: !root.createWithoutDates
                onEditingFinished: {
                    var parts = text.split(".")
                    if (parts.length === 3) {
                        var d = new Date(parseInt(parts[2]), parseInt(parts[1]) - 1, parseInt(parts[0]))
                        if (!isNaN(d.getTime())) root.startDate = d
                    }
                }
            }

            CheckBox {
                id: withoutDatesCheck
                text: "Создать без сроков"
                onCheckedChanged: root.createWithoutDates = checked
            }

            Button { text: "Построить"; onClicked: { root.taskCount = parseInt(countField.text) || 3; root.buildCascade() } }

            Item { Layout.fillWidth: true }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: "#cccccc" }

        ScrollView
        {
            id: scrollView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ColumnLayout
            {
                width: scrollView.availableWidth
                spacing: 5

                Repeater
                {
                    id: cascadeRepeater
                    model: cascadeModel
                    delegate: RowLayout
                    {
                        id: cascadeRow
                        Layout.fillWidth: true
                        width: parent.width
                        spacing: 5
                        property int rowIndex: index

                        Label { text: (cascadeRow.rowIndex + 1) + "."; Layout.preferredWidth: 25 }

                        ComboBox {
                            Layout.preferredWidth: 300
                            visible: !root.isLocalMode
                            model: root.groupNames()
                            onCurrentIndexChanged: {
                                if (cascadeModel[cascadeRow.rowIndex])
                                    cascadeModel[cascadeRow.rowIndex].groupIndex = currentIndex
                            }
                        }

                        TextField {
                            Layout.preferredWidth: 200
                            text: modelData ? modelData.title : ""
                            placeholderText: "Название"
                            onTextChanged: {
                                if (cascadeModel[cascadeRow.rowIndex])
                                    cascadeModel[cascadeRow.rowIndex].title = text
                            }
                        }

                        ComboBox {
                            Layout.fillWidth: true
                            Layout.minimumWidth: 150
                            model: projectController ? projectController.projectMembers : []
                            textRole: "full_name"
                            valueRole: "user_id"
                            onActivated: {
                                if (cascadeModel[cascadeRow.rowIndex]) {
                                    var m = projectController.projectMembers[currentIndex]
                                    cascadeModel[cascadeRow.rowIndex].responsible = String(m.user_id)
                                }
                            }
                        }

                        TextField {
                            Layout.preferredWidth: 50
                            visible: !root.createWithoutDates
                            text: modelData ? modelData.duration : "7"
                            validator: IntValidator { bottom: 1; top: 365 }
                            onTextChanged: {
                                if (cascadeModel[cascadeRow.rowIndex])
                                    cascadeModel[cascadeRow.rowIndex].duration = parseInt(text) || 7
                            }
                        }

                        Label { text: "дн."; visible: !root.createWithoutDates; Layout.preferredWidth: 30 }

                        TextField {
                            Layout.fillWidth: true
                            Layout.minimumWidth: 200
                            placeholderText: "Комментарий"
                            text: modelData ? modelData.comment : ""
                            onTextChanged: {
                                if (cascadeModel[cascadeRow.rowIndex])
                                    cascadeModel[cascadeRow.rowIndex].comment = text
                            }
                        }
                    }
                }
            }
        }

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: 2
            Repeater
            {
                model: root.conflictMessages
                delegate: Label {
                    text: modelData
                    color: "#c62828"
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignRight
            spacing: 10
            Button { text: "Отмена"; onClicked: root.close() }
            Button
            {
                text: "Создать"
                onClicked:
                {
                    root.conflictMessages = root.localCheckConflicts()
                    if (root.conflictMessages.length > 0) return

                    var tasks = []
                    var dependencies = []
                    var gIds = root.groupIds()
                    var currentDate = new Date(root.startDate)
                    var prevBatchId = ""

                    for (var i = 0; i < root.cascadeModel.length; i++)
                    {
                        var item = root.cascadeModel[i]
                        var gid = gIds[item.groupIndex] || gIds[0]
                        var batchId = "b" + i
                        var t = { batch_id: batchId, group_id: gid, title: item.title }
                        if (!root.createWithoutDates)
                        {
                            var endDate = new Date(currentDate)
                            endDate.setDate(endDate.getDate() + item.duration - 1)
                            t.start_date = Qt.formatDateTime(currentDate, "yyyy-MM-dd")
                            t.end_date = Qt.formatDateTime(endDate, "yyyy-MM-dd")
                            currentDate = new Date(endDate)
                            currentDate.setDate(currentDate.getDate() + 1)
                        }
                        if (item.responsible) {
                            var rid = parseInt(item.responsible)
                            if (!isNaN(rid) && rid > 0) t.responsible_id = rid
                        }
                        tasks.push(t)
                        if (prevBatchId !== "")
                            dependencies.push({ predecessor_batch_id: prevBatchId, successor_batch_id: batchId })
                        prevBatchId = batchId
                    }

                    projectController.createTasksBatch(tasks, dependencies)
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
