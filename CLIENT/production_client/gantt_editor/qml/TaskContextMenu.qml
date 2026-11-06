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
        canAcceptForecast = false
        canEditForecast = true
        canSetProgress = false
        localGraphState = 0
        isTaskCompleted = false

        if (!taskId || !projectController || !projectController.projectData) return
        var task = projectController.projectData.taskModel.getTask(taskId)
        if (!task) return

        isTaskCompleted = (task.status === 1)
        localGraphState = task.localGraphState !== undefined ? task.localGraphState : 0
        canEditForecast = !(localGraphState === 2 || localGraphState === 3)
        canSetProgress = !isTaskCompleted && (localGraphState === 0)

        if (projectController.settingsManager.editingLocked) return
        if (isTaskCompleted) return

        var fs = task.forecastStart
        var fe = task.forecastEnd
        var ss = task.startDate
        var se = task.endDate
        if (!fs || !fe || !ss || !se) return
        if (fs.toString() === ss.toString() && fe.toString() === se.toString()) return
        canAcceptForecast = true
    }

    MenuItem {
        text: "Переименовать"
        enabled: !root.isLocked
        onTriggered: if (root.taskId && mainWindow && mainWindow.renameTaskDialog) mainWindow.renameTaskDialog.openForTask(root.taskId)
    }

    MenuItem {
        text: "Назначить ответственного"
        enabled: !root.isLocked
        onTriggered: if (root.taskId && mainWindow && mainWindow.responsibleDialog) mainWindow.responsibleDialog.openForTask(root.taskId)
    }

    MenuItem {
        text: "Изменить комментарий"
        enabled: !root.isLocked
        onTriggered: if (root.taskId && mainWindow && mainWindow.commentDialog) mainWindow.commentDialog.openForTask(root.taskId)
    }

    Menu {
        title: "Изменить статус"
        enabled: !root.isLocked
        MenuItem { text: "🟡 Запланировано"; onTriggered: if(projectController) projectController.projectData.taskModel.setTaskStatus(root.taskId, 0) }
        MenuItem { text: "🟢 Выполнено"; onTriggered: if(mainWindow) mainWindow.completeTaskDialog.openForTask(root.taskId) }
        MenuItem { text: "🟠 Имеются риски"; onTriggered: if(projectController) projectController.projectData.taskModel.setTaskStatus(root.taskId, 2) }
        MenuItem { text: "🔴 Блокировано"; onTriggered: if(projectController) projectController.projectData.taskModel.setTaskStatus(root.taskId, 3) }
    }

    MenuItem {
        text: "Изменить целевые сроки"
        enabled: !root.isLocked
        onTriggered: if (root.taskId && typeof mainWindow !== "undefined" && mainWindow.editTaskDialog) mainWindow.editTaskDialog.openForTask(root.taskId)
    }

    MenuItem {
        text: "Изменить прогнозные сроки"
        visible: !root.isLocalGraph
        enabled: !root.isLocked && root.canEditForecast && !root.isTaskCompleted
        onTriggered: if (root.taskId && typeof mainWindow !== "undefined" && mainWindow.editForecastDatesDialog) mainWindow.editForecastDatesDialog.openForTask(root.taskId)
    }

    MenuItem {
        text: "Утвердить прогноз"
        visible: !root.isLocalGraph
        enabled: !root.isLocked && root.canAcceptForecast
        onTriggered: if (root.taskId && projectController) projectController.taskController.acceptForecastAsTarget(root.taskId)
    }

    MenuItem {
        text: "Установить прогресс"
        enabled: !root.isLocked && root.canSetProgress
        onTriggered: if (root.taskId && mainWindow && mainWindow.setProgressDialog) mainWindow.setProgressDialog.openForTask(root.taskId)
    }

    MenuSeparator {}

    Menu {
        title: "Изменить зависимости"
        enabled: !root.isLocked

        MenuItem { text: "Добавить нисходящую зависимость"; onTriggered: if(mainWindow && mainWindow.dependencyDialog) mainWindow.dependencyDialog.openForTask(root.taskId, true) }
        MenuItem { text: "Добавить восходящую зависимость"; onTriggered: if(mainWindow && mainWindow.dependencyDialog) mainWindow.dependencyDialog.openForTask(root.taskId, false) }
        MenuSeparator {}
        MenuItem { text: "Удалить нисходящую зависимость"; onTriggered: if(projectController) projectController.projectData.dependencyModel.removeDownstreamDependency(root.taskId) }
        MenuItem { text: "Удалить восходящую зависимость"; onTriggered: if(projectController) projectController.projectData.dependencyModel.removeUpstreamDependency(root.taskId) }
    }

    MenuSeparator {}

    MenuItem {
        text: "Открыть локальный график"
        enabled: true
        onTriggered: {
            if (!root.taskId || !projectController) return
            console.log("Open local graph for task:", root.taskId)
            if (typeof mainWindow !== "undefined" && mainWindow.openLocalGraphEditor)
                mainWindow.openLocalGraphEditor(root.taskId)
        }
    }

    MenuSeparator {}

    MenuItem {
        text: "Добавить задачу сверху"
        enabled: !root.isLocked
        onTriggered: if (root.onAddTaskAboveCallback) root.onAddTaskAboveCallback(root.taskId)
    }

    MenuItem {
        text: "Добавить задачу снизу"
        enabled: !root.isLocked
        onTriggered: if (root.onAddTaskBelowCallback) root.onAddTaskBelowCallback(root.taskId)
    }

    MenuItem {
        text: "Удалить"
        enabled: !root.isLocked
        onTriggered: if(root.taskId && projectController) projectController.taskController.removeTask(root.taskId)
    }
}
