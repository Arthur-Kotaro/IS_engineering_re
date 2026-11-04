import QtQuick 6.0

Rectangle
{
    id: root
    width: 2
    height: parent ? parent.height : 100
    property color lineColor: "#2266cc"
    color: lineColor
    visible: false
    z: 9

    property bool showArrows: false
    property string arrowDirection: "right"

    // Верхняя стрелка
    Rectangle
    {
        visible: root.showArrows
        width: 12
        height: 2
        color: root.lineColor
        x: root.arrowDirection === "right" ? 0 : -10
        y: 6

        Rectangle
        {
            width: 6
            height: 6
            color: root.lineColor
            rotation: 45
            x: root.arrowDirection === "right" ? 8 : -2
            y: -2
        }
    }

    // Нижняя стрелка
    Rectangle
    {
        visible: root.showArrows
        width: 12
        height: 2
        color: root.lineColor
        x: root.arrowDirection === "right" ? 0 : -10
        y: root.height - 8

        Rectangle
        {
            width: 6
            height: 6
            color: root.lineColor
            rotation: 45
            x: root.arrowDirection === "right" ? 8 : -2
            y: -2
        }
    }
}
