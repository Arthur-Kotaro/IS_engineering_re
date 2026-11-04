import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Qt.labs.platform 1.1 as Labs

Dialog {
    id: root
    title: "Сохранить как"
    width: 400; height: 200; modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    
    property int selectedFormat: 0
    
    ColumnLayout
    {
        anchors.fill: parent; spacing: 15
        Label { text: "Выберите формат сохранения:"; font.bold: true }
        ComboBox
        {
            id: formatCombo
            Layout.fillWidth: true
            model: [".gantt (проект)", "PDF документ", "PNG изображение"]
            onCurrentIndexChanged: root.selectedFormat = currentIndex
        }
        
        Labs.FileDialog
        {
            id: fileDialog
            title: "Сохранить как"
            fileMode: Labs.FileDialog.SaveFile
            onAccepted:
            {
                var filePath = file
                if (root.selectedFormat === 0)
                {
                    if (!filePath.endsWith(".gantt")) filePath += ".gantt"
                    projectController.saveProjectAs(filePath)
                }
                else if (root.selectedFormat === 1)
                {
                    if (!filePath.endsWith(".pdf")) filePath += ".pdf"
                    projectController.exportToPdf(filePath)
                }
                else
                {
                    if (!filePath.endsWith(".png")) filePath += ".png"
                    projectController.exportToPng(filePath, 1920, 1080)
                }
            }
        }
    }
    
    onAccepted: fileDialog.open()
}
