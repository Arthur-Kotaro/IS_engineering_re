.pragma library

function computeNodePositions(allTasks, dependencyModel, displayStart, dayWidth)
{
    var taskById = {}
    for (var i = 0; i < allTasks.length; i++)
        taskById[allTasks[i].id] = allTasks[i]

    var targetNodes = {}
    var forecastNodes = {}

    for (var k = 0; k < allTasks.length; k++)
    {
        var task = allTasks[k]
        if (hasDates(task)) continue
        if (hasForecast(task)) continue

        targetNodes[task.id] = computeNodeX(task.id, "target", taskById, targetNodes, dependencyModel, displayStart, dayWidth)
        forecastNodes[task.id] = computeNodeX(task.id, "forecast", taskById, forecastNodes, dependencyModel, displayStart, dayWidth)
    }

    return { target: targetNodes, forecast: forecastNodes }
}

function computeNodeX(taskId, kind, taskById, nodes, dependencyModel, displayStart, dayWidth)
{
    if (nodes[taskId] !== undefined) return nodes[taskId]

    var preds = dependencyModel.getPredecessors(taskId)
    var maxPredEnd = null
    var maxPredNodeX = -1

    for (var i = 0; i < preds.length; i++)
    {
        var pid = preds[i]
        var predTask = taskById[pid]
        if (!predTask) continue

        var pEnd = null

        if (kind === "target")
        {
            if (hasDates(predTask)) pEnd = new Date(predTask.endDate)
            else if (hasForecast(predTask)) pEnd = new Date(predTask.forecastEnd)
        }
        else
        {
            if (hasForecast(predTask)) pEnd = new Date(predTask.forecastEnd)
            else if (hasDates(predTask))
                pEnd = new Date(predTask.endDate)
        }

        if (pEnd !== null)
        {
            if (maxPredEnd === null || pEnd > maxPredEnd) maxPredEnd = pEnd
        }
        else
        {
            var pNodeX = computeNodeX(pid, kind, taskById, nodes, dependencyModel, displayStart, dayWidth)
            if (pNodeX > maxPredNodeX) maxPredNodeX = pNodeX
        }
    }

    var result
    if (maxPredEnd !== null) result = (Math.floor((maxPredEnd - displayStart) / 86400000) + 1) * dayWidth + dayWidth / 2
    else if (maxPredNodeX >= 0)
        result = maxPredNodeX + 3 * dayWidth
    else
        result = dayWidth / 2

    if (result < dayWidth / 2) result = dayWidth / 2
    nodes[taskId] = result
    return result
}

function hasDates(taskData)
{
    return taskData && taskData.startDate && taskData.endDate &&
           !isNaN(new Date(taskData.startDate).getTime()) &&
           !isNaN(new Date(taskData.endDate).getTime())
}

function hasForecast(taskData)
{
    return taskData && taskData.forecastStart && taskData.forecastEnd &&
           !isNaN(new Date(taskData.forecastStart).getTime()) &&
           !isNaN(new Date(taskData.forecastEnd).getTime())
}
