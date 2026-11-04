import QtQuick 6.0
import QtQuick.Controls 6.0

Menu
{
    id: root
    property string groupId: ""
    property string groupName: ""

    signal addTaskClicked()
    signal renameClicked()
    signal addAboveClicked()
    signal addBelowClicked()
    signal deleteClicked()

    MenuItem { text: "Добавить задачу"; onTriggered: root.addTaskClicked() }
    MenuItem { text: "Переименовать"; onTriggered: root.renameClicked() }
    MenuSeparator {}
    MenuItem { text: "Добавить группу сверху"; onTriggered: root.addAboveClicked() }
    MenuItem { text: "Добавить группу снизу"; onTriggered: root.addBelowClicked() }
    MenuSeparator {}
    MenuItem { text: "Удалить"; onTriggered: root.deleteClicked() }
}
