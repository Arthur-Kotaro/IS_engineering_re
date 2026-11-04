import QtQuick 6.0

Rectangle
{
    id: root
    property var rowData
    property real mouseX: 0
    property real mouseY: 0
    property bool fromNode: false
    property var anchorItem: null // Ссылка на TaskBar или TaskNode

    color: "#333333"
    radius: 4
    opacity: 0.95
    z: 100

    x:
    {
        if (!anchorItem) return 0
            var baseX = fromNode ? (anchorItem.x + anchorItem.width + 16) : (anchorItem.x + mouseX + 16)
            return baseX
    }
    y:
    {
        if (!anchorItem) return 0
            var baseY = fromNode ? (anchorItem.y + 8) : (anchorItem.y + mouseY + 12)
            return baseY
    }

    states: State
    {
        name: "left"
        when:
        {
            if (!anchorItem) return false
                var baseX = fromNode ? (anchorItem.x + anchorItem.width + 16) : (anchorItem.x + mouseX + 16)
                return (baseX + width) > root.parent.width
        }
        PropertyChanges
        {
            target: root
            x:
            {
                if (!anchorItem) return 0
                    if (fromNode) return anchorItem.x - width - 16
                        return anchorItem.x + mouseX - width - 16
            }
        }
    }

    width: tooltipText.implicitWidth + 24
    height: tooltipText.implicitHeight + 20

    Text
    {
        id: tooltipText
        anchors.centerIn: parent
        color: "white"
        font.pixelSize: 14
        lineHeight: 1.2
        text:
        {
            if (!rowData) return ""

            var lines = []
            var label = (rowData.rowKind === "forecast") ? "Прогноз: " : ""
            lines.push(label + rowData.taskTitle)

            var resp = rowData.taskResponsible && rowData.taskResponsible !== "" ? rowData.taskResponsible : "—"
            lines.push("Ответственный: " + resp)

            if (rowData.hasDates === true)
            {
                var s = Qt.formatDateTime(new Date(rowData.taskStart), "dd.MM.yyyy")
                var e = Qt.formatDateTime(new Date(rowData.taskEnd), "dd.MM.yyyy")
                var dur = Math.floor((new Date(rowData.taskEnd) - new Date(rowData.taskStart)) / 86400000) + 1
                lines.push("Целевые сроки: " + s + " — " + e + " (" + dur + " дн.)")
            }

            if (rowData.hasForecast === true && (rowData.rowKind === "forecast" || rowData.isUnapproved === true))
            {
                var fs = Qt.formatDateTime(new Date(rowData.forecastStart), "dd.MM.yyyy")
                var fe = Qt.formatDateTime(new Date(rowData.forecastEnd), "dd.MM.yyyy")
                lines.push("Прогноз: " + fs + " — " + fe)
            }

            var statusText = "Запланировано"
            switch (rowData.taskStatus)
            {
                case 0: statusText = "Запланировано"; break
                case 1: statusText = "Выполнено"; break
                case 2: statusText = "Имеются риски"; break
                case 3: statusText = "Блокировано"; break
            }
            lines.push("Статус: " + statusText)

            if (rowData.hasConflict === true)
            {
                var predEnd = projectController?.projectData?.getLinkedPredecessorEndDate()
                if (predEnd && !isNaN(new Date(predEnd).getTime()))
                    lines.push("⚠ План нарушает границу предшественника (завершение " + Qt.formatDateTime(new Date(predEnd), "dd.MM.yyyy") + ")")
            }

            if ((rowData.progressTotal || 0) > 0 && (rowData.progressCurrent || 0) >= 0)
                lines.push("Прогресс: " + rowData.progressCurrent + " / " + rowData.progressTotal)

            var lgs = rowData.localGraphState !== undefined ? rowData.localGraphState : 0
            var lgsText = ""
            switch (lgs)
            {
                case 1: lgsText = "требуется"; break
                case 2: lgsText = "привязан"; break
                case 3: lgsText = "отсутствует"; break
            }
            if (lgsText !== "") lines.push("Локальный график: " + lgsText)

            if (rowData.taskComment && rowData.taskComment !== "")
                lines.push("Комментарий: " + rowData.taskComment)

            return lines.join("\n")
        }
    }
}
