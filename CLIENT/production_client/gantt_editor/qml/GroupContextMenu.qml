import QtQuick 6.0
import QtQuick.Controls 6.0

Menu
{
    id: root
    property string groupId: ""
    property string groupName: ""
    property bool showGroupWizard: true

    signal addTaskClicked()
    signal renameClicked()
    signal addAboveClicked()
    signal addBelowClicked()
    signal deleteClicked()

    MenuItem { text: "Добавить задачу"; onTriggered: root.addTaskClicked() }
    MenuItem { text: "Переименовать"; onTriggered: root.renameClicked() }
    MenuSeparator { visible: root.showGroupWizard }
    MenuItem { text: "Добавить группу сверху"; visible: root.showGroupWizard; onTriggered: root.addAboveClicked() }
    MenuItem { text: "Добавить группу снизу"; visible: root.showGroupWizard; onTriggered: root.addBelowClicked() }
    MenuSeparator { visible: root.showGroupWizard }
    MenuItem { text: "Удалить"; onTriggered: root.deleteClicked() }
}
