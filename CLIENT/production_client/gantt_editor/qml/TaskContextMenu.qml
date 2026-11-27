import QtQuick 6.0
import QtQuick.Controls 6.0

Menu
{
    id: root
    property string taskId: ""
    property var onAddTaskAboveCallback: null
    property var onAddTaskBelowCallback: null
    property bool canAcceptForecast: false
    property bool canEditForecast: true
    property bool canSetProgress: false
    property int localGraphState: 0
    property bool isTaskCompleted: false

    readonly property bool isLocalGraph:
        projectController && projectController.projectData
        && projectController.projectData.graphKind === 1

    readonly property bool isLocked:
        projectController && projectController.settingsManager
        && projectController.settingsManager.editingLocked

    onOpened: { updateState() }

    function updateState()
    {
        canAcceptForecast = false; canEditForecast = true; canSetProgress = false
        localGraphState = 0; isTaskCompleted = false
        if (!taskId || !projectController || !projectController.projectData) return
        var task = projectController.projectData.taskModel.getTask(taskId)
        if (!task) return
        isTaskCompleted = (task.status === 1)
        localGraphState = task.localGraphState !== undefined ? task.localGraphState : 0
        // Блокируем только при ATTACHED (2) — ЛГ содержит задачи со сроками.
        // dateless (1) — ЛГ есть, но без задач со сроками → прогноз редактируется.
        // NOT_REQUIRED (0) — ЛГ нет.
        // MISSING (3) — трактуем как «нет ЛГ».
        canEditForecast = (localGraphState !== 2)
        // Прогресс вручную — только если ЛГ нет (0) или dateless (1) (без сроков).
        canSetProgress = !isTaskCompleted && (localGraphState !== 2)
        if (projectController.settingsManager.editingLocked) return
        if (isTaskCompleted) return
        var fs = task.forecastStart; var fe = task.forecastEnd
        var ss = task.startDate; var se = task.endDate
        if (!fs || !fe || !ss || !se) return
        if (fs.toString() === ss.toString() && fe.toString() === se.toString()) return
        canAcceptForecast = true
    }

    function setStatus(status) {
        if (root.isLocalGraph) {
            // ЛГ: через localGraphController (HTTP PUT в BUS)
            if (projectController && projectController.localGraphController)
                projectController.localGraphController.setLocalTaskStatus(root.taskId, status)
        } else {
            // МГ: через ProjectController (MG)
            if (projectController)
                projectController.updateTaskStatus(root.taskId, status)
        }
    }

    MenuItem {
        text: "Показать подробности"
        onTriggered: if (root.taskId && mainWindow && mainWindow.taskDetailsDialog)
                         mainWindow.taskDetailsDialog.openForTask(root.taskId)
    }

    MenuItem {
        text: "Переименовать"; enabled: !root.isLocked
        onTriggered: if (root.taskId && mainWindow && mainWindow.renameTaskDialog)
                         mainWindow.renameTaskDialog.openForTask(root.taskId)
    }

    MenuItem {
        text: "Назначить ответственного"; enabled: !root.isLocked
        onTriggered: if (root.taskId && mainWindow && mainWindow.responsibleDialog)
                         mainWindow.responsibleDialog.openForTask(root.taskId)
    }

    MenuItem {
        text: "Изменить комментарий"; enabled: !root.isLocked
        onTriggered: if (root.taskId && mainWindow && mainWindow.commentDialog)
                         mainWindow.commentDialog.openForTask(root.taskId)
    }

    Menu {
        title: "Изменить статус"
        enabled: !root.isLocked
        MenuItem { text: "🟡 Запланировано"; onTriggered: root.setStatus(0) }
        MenuItem { text: "🟢 Выполнено"
            onTriggered: {
                if (root.isLocalGraph) {
                    root.setStatus(1)
                } else if (mainWindow) {
                    mainWindow.completeTaskDialog.openForTask(root.taskId)
                }
            } }
            MenuItem { text: "🟠 Имеются риски"; onTriggered: root.setStatus(2) }
            MenuItem { text: "🔴 Блокировано"; onTriggered: root.setStatus(3) }
    }

    MenuItem {
        text: "Изменить целевые сроки"; enabled: !root.isLocked
        onTriggered: if (root.taskId && mainWindow && mainWindow.editTaskDialog)
                         mainWindow.editTaskDialog.openForTask(root.taskId)
    }

    MenuItem {
        text: "Изменить прогнозные сроки"
        visible: !root.isLocalGraph
        enabled: !root.isLocked && root.canEditForecast && !root.isTaskCompleted
        onTriggered: if (root.taskId && mainWindow && mainWindow.editForecastDatesDialog)
                         mainWindow.editForecastDatesDialog.openForTask(root.taskId)
    }

    MenuItem {
        text: "Утвердить прогноз"
        visible: !root.isLocalGraph
        enabled: !root.isLocked && root.canAcceptForecast
        onTriggered: if (root.taskId && projectController)
                         projectController.taskController.acceptForecastAsTarget(root.taskId)
    }

    MenuItem {
        text: "Установить прогресс"
        enabled: !root.isLocked && root.canSetProgress
        onTriggered: if (root.taskId && mainWindow && mainWindow.setProgressDialog)
                         mainWindow.setProgressDialog.openForTask(root.taskId)
    }

    MenuSeparator {}

    Menu {
        title: "Изменить зависимости"
        enabled: !root.isLocked
        MenuItem { text: "Добавить нисходящую зависимость"
            onTriggered: if (mainWindow && mainWindow.dependencyDialog)
                             mainWindow.dependencyDialog.openForTask(root.taskId, true) }
        MenuItem { text: "Добавить восходящую зависимость"
            onTriggered: if (mainWindow && mainWindow.dependencyDialog)
                             mainWindow.dependencyDialog.openForTask(root.taskId, false) }
        MenuSeparator {}
        MenuItem { text: "Удалить все нисходящие зависимости"
            onTriggered: {
                if (!projectController || !root.taskId) return
                var deps = projectController.projectData.dependencyModel
                var succ = deps.getSuccessors(root.taskId)
                for (var i = 0; i < succ.length; i++) {
                    for (var d = 0; d < deps.rowCount(); d++) {
                        var idx = deps.index(d, 0)
                        if (deps.data(idx, Qt.UserRole + 2) === root.taskId &&
                            deps.data(idx, Qt.UserRole + 3) === succ[i]) {
                            if (root.isLocalGraph) {
                                if (projectController.localGraphController)
                                    projectController.localGraphController.removeLocalDependency(deps.data(idx, Qt.UserRole + 1))
                            } else {
                                projectController.deleteDependency(deps.data(idx, Qt.UserRole + 1))
                            }
                            break
                        }
                    }
                }
            } }
        MenuItem { text: "Удалить все восходящие зависимости"
            onTriggered: {
                if (!projectController || !root.taskId) return
                var deps = projectController.projectData.dependencyModel
                var preds = deps.getPredecessors(root.taskId)
                for (var i = 0; i < preds.length; i++) {
                    for (var d = 0; d < deps.rowCount(); d++) {
                        var idx = deps.index(d, 0)
                        if (deps.data(idx, Qt.UserRole + 2) === preds[i] &&
                            deps.data(idx, Qt.UserRole + 3) === root.taskId) {
                            if (root.isLocalGraph) {
                                if (projectController.localGraphController)
                                    projectController.localGraphController.removeLocalDependency(deps.data(idx, Qt.UserRole + 1))
                            } else {
                                projectController.deleteDependency(deps.data(idx, Qt.UserRole + 1))
                            }
                            break
                        }
                    }
                }
            } }
    }

    MenuSeparator { visible: !root.isLocalGraph }

    MenuItem {
        text: "Открыть локальный график"
        visible: !root.isLocalGraph
        onTriggered: {
            if (!root.taskId || !projectController) return
                var task = projectController.projectData.taskModel.getTask(root.taskId)
                if (task) {
                    var groupId = task.groupId
                    var busUrl = projectController.projectData.groupModel.getBusUrl(groupId)
                    if (busUrl) projectController.openLocalGraphProcess(root.taskId, busUrl)
                        else if (mainWindow && mainWindow.toast)
                            mainWindow.toast.show("Не удалось определить BUS для группы задачи")
                }
        }
    }

    MenuSeparator {}

    MenuItem {
        text: "Добавить задачу сверху"; enabled: !root.isLocked
        onTriggered: if (root.onAddTaskAboveCallback) root.onAddTaskAboveCallback(root.taskId)
    }

    MenuItem {
        text: "Добавить задачу снизу"; enabled: !root.isLocked
        onTriggered: if (root.onAddTaskBelowCallback) root.onAddTaskBelowCallback(root.taskId)
    }

    MenuSeparator {}

    MenuItem {
        text: "Удалить"; enabled: !root.isLocked
        onTriggered: {
            if (!root.taskId || !projectController) return
                if (root.isLocalGraph) {
                    projectController.localGraphController.removeLocalTask(root.taskId)
                } else {
                    projectController.deleteTask(root.taskId)
                }
        }
    }
}
