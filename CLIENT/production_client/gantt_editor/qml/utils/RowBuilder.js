.pragma library

function isValidDate(d)
{
    return d !== undefined && d !== null && !isNaN(new Date(d).getTime())
}

// Строит список строк для левой панели (GroupDelegate)
function buildGroupRows(taskModel, taskIds, mode)
{
    var rows = []
    for (var i = 0; i < taskIds.length; i++)
    {
        var taskData = taskModel.getTask(taskIds[i])
        if (!taskData) continue
        var isCompleted = (taskData.status === 1)

        if (mode === 0)
        {
            rows.push({taskId: taskIds[i], rowKind: "target", data: taskData, isCompleted: isCompleted})
        }
        else if (mode === 1)
        {
            if (isCompleted)
                rows.push({taskId: taskIds[i], rowKind: "target", data: taskData, isCompleted: true})
            else
                rows.push({taskId: taskIds[i], rowKind: "forecast", data: taskData, isCompleted: false})
        }
        else
        {
            rows.push({taskId: taskIds[i], rowKind: "target", data: taskData, isCompleted: isCompleted})
            if (!isCompleted)
                rows.push({taskId: taskIds[i], rowKind: "forecast", data: taskData, isCompleted: false})
        }
    }
    return rows
}

// Вспомогательная функция для формирования строки сетки (GridArea)
function createGridItem(taskData, taskId, rowKind, isCompleted, hasDates, isUnapproved, nodeX)
{
    return {
        type: "task",
        rowKind: rowKind,
        taskId: taskId,
        taskTitle: taskData.title,
        taskResponsible: taskData.responsible,
        forecastStart: taskData.forecastStart,
        forecastEnd: taskData.forecastEnd,
        taskStatus: taskData.status,
        taskComment: taskData.comment,
        isCompleted: isCompleted,
        hasDates: hasDates,
        isUnapproved: isUnapproved,
        nodeX: nodeX,
        localGraphState: taskData.localGraphState,
        progressCurrent: taskData.progressCurrent,
        progressTotal: taskData.progressTotal,
        hasConflict: false,
        rowIndex: 0,
        taskStart: null,
        taskEnd: null
    }
}
