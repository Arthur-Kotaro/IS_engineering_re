import QtQuick 6.0

Rectangle {
    id: root
    width: 2
    height: parent ? parent.height : 100
    color: "red"
    visible: false
    z: 10

    property date displayStart: new Date()
    property int dayWidth: 30

    onDisplayStartChanged: updateLinePosition()
    onDayWidthChanged: updateLinePosition()
    onXChanged: {}

    Timer {
        id: timeLineTimer
        interval: 60000
        running: true
        repeat: true
        onTriggered: updateLinePosition()
    }

    function updateLinePosition() {
        if (typeof projectController === "undefined" || !projectController || !projectController.projectData) {
            return
        }

        var now = new Date()
        var displayStart = root.displayStart

        if (!displayStart) return

        var daysDiff = Math.floor((now - displayStart) / (1000 * 60 * 60 * 24))
        var minutesInDay = now.getHours() * 60 + now.getMinutes()
        var xPos = daysDiff * root.dayWidth + (minutesInDay / (24 * 60)) * root.dayWidth

        // Всегда обновляем x, чтобы TaskRow мог использовать актуальную координату
        // даже когда линия за пределами видимой области.
        root.x = xPos
        root.visible = (parent && xPos >= 0 && xPos <= parent.width)
    }

    Component.onCompleted: {
        updateLinePosition()
    }
}
