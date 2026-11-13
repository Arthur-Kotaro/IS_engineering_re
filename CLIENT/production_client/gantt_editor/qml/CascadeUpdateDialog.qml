import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Обновить сроки последователей?"
    width: 700
    height: 500
    modal: true
    standardButtons: Dialog.NoButton
    anchors.centerIn: Overlay.overlay

    property var successors: []
    property string baseReason: ""
    property var onConfirm: null

    function openWith(list, baseReason, callback) {
        root.successors = list
        root.baseReason = baseReason
        root.onConfirm = callback
        repeater.model = list
        root.open()
    }

    function stripInfluence(reason) {
        // Убираем все вхождения " (влияние: ...)" из baseReason
        return reason.replace(/ ?\(влияние:[^)]*\)/g, "")
    }

    function buildReason() {
        var titles = []
        for (var i = 0; i < root.successors.length; i++)
            titles.push(root.successors[i].title)
        var clean = stripInfluence(root.baseReason)
        return clean + " (влияние: " + titles.join(", ") + ")"
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10

        Label
        {
            text: "Изменение сроков предшественника повлияет на следующие задачи:"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: "#cccccc" }

        ScrollView
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ColumnLayout
            {
                width: parent.width
                spacing: 4

                Repeater
                {
                    id: repeater
                    model: []
                    delegate: Rectangle
                    {
                        Layout.fillWidth: true
                        height: 50
                        color: "#f5f5f5"
                        radius: 4
                        border.color: "#dddddd"

                        ColumnLayout
                        {
                            anchors.fill: parent
                            anchors.margins: 6
                            spacing: 2
                            Label { text: modelData.title; font.bold: true; font.pixelSize: 12 }
                            Label {
                                text: "Новые сроки: " +
                                      Qt.formatDateTime(new Date(modelData.newStart), "dd.MM.yyyy") +
                                      " — " +
                                      Qt.formatDateTime(new Date(modelData.newEnd), "dd.MM.yyyy")
                                font.pixelSize: 11
                            }
                        }
                    }
                }
            }
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignRight
            spacing: 10

            Button {
                text: "Не обновлять"
                onClicked: { root.onConfirm = null; root.close() }
            }
            Button {
                text: "Обновить последователей"
                onClicked: {
                    if (root.onConfirm) root.onConfirm(root.successors, root.buildReason())
                    root.close()
                }
            }
        }
    }
}
