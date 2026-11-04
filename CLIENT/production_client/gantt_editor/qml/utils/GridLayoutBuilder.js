.pragma library
.import "NodeLayout.js" as NodeLayout
.import "RowBuilder.js" as RowBuilder

function buildLayout(context)
{
    var localMode = context.localMode
    var displayStart = context.displayStart
    var dayWidth = context.dayWidth
    var mode = context.mode
    var conflictSet = context.conflictSet
    var projectData = context.projectData

    var allTasks = []
    var ids = projectData.taskModel.getAllTasks()
    for (var ai = 0; ai < ids.length; ai++)
    {
        var t = projectData.taskModel.getTask(ids[ai].id)
        if (t) allTasks.push(t)
    }

    var nodes = NodeLayout.computeNodePositions(allTasks, projectData.dependencyModel, displayStart, dayWidth)
    var targetNodes = nodes.target
    var forecastNodes = nodes.forecast

    var items = []
    var taskCounter = 0

    function pushRow(taskData, taskId, kind, startDate, endDate, rowHasDates, rowIsUnapproved)
    {
        var nodeXVal = 0
        if (!rowHasDates && !rowIsUnapproved)
        {
            if (kind === "target") nodeXVal = targetNodes[taskId] !== undefined ? targetNodes[taskId] : dayWidth / 2
            else nodeXVal = forecastNodes[taskId] !== undefined ? forecastNodes[taskId] : dayWidth / 2
        }

        var item = RowBuilder.createGridItem(taskData, taskId, kind, (taskData.status === 1), rowHasDates, rowIsUnapproved, nodeXVal)
        item.rowIndex = taskCounter
        item.hasConflict = conflictSet[taskId] === true
        item.taskStart = startDate
        item.taskEnd = endDate
        item.type = "task"
        
        items.push(item)
        taskCounter++
    }

    if (localMode)
    {
        for (var li = 0; li < allTasks.length; li++)
        {
            var tdL = allTasks[li]
            var hasDatesL = RowBuilder.isValidDate(tdL.startDate) && RowBuilder.isValidDate(tdL.endDate)
            var hasFcL = RowBuilder.isValidDate(tdL.forecastStart) && RowBuilder.isValidDate(tdL.forecastEnd)
            var isUnapprovedL = !hasDatesL && hasFcL
            
            pushRow(tdL, tdL.id, "target", tdL.startDate, tdL.endDate, hasDatesL, isUnapprovedL)
        }
    }
    else
    {
        var groups = projectData.groupModel
        for (var i = 0; i < groups.rowCount(); i++)
        {
            var groupId = groups.getGroupId(i)
            var groupName = groups.getGroupName(i)
            var expanded = groups.isExpanded(i)

            items.push({type: "group", id: groupId, name: groupName, rowIndex: taskCounter})
            taskCounter++

            if (expanded)
            {
                var tasks = projectData.taskModel.getTasksForGroup(groupId)
                for (var j = 0; j < tasks.length; j++)
                {
                    var taskData = projectData.taskModel.getTask(tasks[j])
                    if (!taskData) continue

                    var isCompleted = (taskData.status === 1)
                    var hasDates = RowBuilder.isValidDate(taskData.startDate) && RowBuilder.isValidDate(taskData.endDate)
                    var hasFc = RowBuilder.isValidDate(taskData.forecastStart) && RowBuilder.isValidDate(taskData.forecastEnd)
                    var isUnapproved = !hasDates && hasFc

                    if (mode === 0)
                    {
                        pushRow(taskData, tasks[j], "target", taskData.startDate, taskData.endDate, hasDates, isUnapproved)
                    }
                    else if (mode === 1)
                    {
                        if (isCompleted || !hasFc) 
                            pushRow(taskData, tasks[j], "target", taskData.startDate, taskData.endDate, hasDates, isUnapproved)
                        else
                        {
                            var fHasDates1 = RowBuilder.isValidDate(taskData.forecastStart) && RowBuilder.isValidDate(taskData.forecastEnd)
                            pushRow(taskData, tasks[j], "forecast", taskData.forecastStart, taskData.forecastEnd, fHasDates1, isUnapproved)
                        }
                    }
                    else
                    {
                        pushRow(taskData, tasks[j], "target", taskData.startDate, taskData.endDate, hasDates, isUnapproved)
                        if (!isCompleted)
                        {
                            var fHasDates2 = RowBuilder.isValidDate(taskData.forecastStart) && RowBuilder.isValidDate(taskData.forecastEnd)
                            pushRow(taskData, tasks[j], "forecast", taskData.forecastStart, taskData.forecastEnd, fHasDates2, isUnapproved)
                        }
                    }
                }
            }
        }
    }

    return items
}
