import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Qt.labs.platform 1.1 as Labs

Dialog
{
    id: root
    title: "Экспорт графика"
    width: 620
    height: 600
    modal: true
    standardButtons: Dialog.NoButton
    anchors.centerIn: Overlay.overlay

    property int formatIndex: 0
    property string filePath: ""
    property bool busy: false

    function defaultFilePath()
    {
        if (!projectController || !projectController.projectData) return ""
        var basePath = projectController.projectData.filePath
        var dir = ""
        var baseName = "graph"
        if (basePath && basePath.length > 0)
        {
            var slash = Math.max(basePath.lastIndexOf("/"), basePath.lastIndexOf("\\"))
            if (slash >= 0)
            {
                dir = basePath.substring(0, slash + 1)
                baseName = basePath.substring(slash + 1)
                var dot = baseName.lastIndexOf(".")
                if (dot >= 0) baseName = baseName.substring(0, dot)
            }
        }
        var ext = (formatIndex === 0) ? ".png" : ".pdf"
        return dir + baseName + "_" + Qt.formatDateTime(new Date(), "yyyyMMdd_HHmmss") + ext
    }

    onOpened:
    {
        root.busy = false
        root.filePath = defaultFilePath()
        pathField.text = root.filePath
        errorLabel.visible = false
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        Label { text: "Формат:"; font.bold: true }
        ComboBox
        {
            id: formatCombo
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            model: ["PNG (изображение)", "PDF (документ)"]
            currentIndex: root.formatIndex
            onActivated:
            {
                root.formatIndex = currentIndex
                root.filePath = root.defaultFilePath()
                pathField.text = root.filePath
            }
        }

        Label { text: "Путь к файлу:"; font.bold: true }
        RowLayout
        {
            Layout.fillWidth: true
            spacing: 8

            TextField
            {
                id: pathField
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                text: root.filePath
                onTextChanged: root.filePath = text
            }

            Button
            {
                text: "Обзор..."
                Layout.preferredHeight: 36
                Layout.minimumWidth: 90
                onClicked: saveFileDialog.open()
            }
        }

        Label { text: "Режим отображения:"; font.bold: true }
        RowLayout
        {
            spacing: 16
            ButtonGroup { id: viewModeGroup }
            RadioButton
            {
                text: "Целевой"
                checked: true
                property int value: 0
                ButtonGroup.group: viewModeGroup
            }
            RadioButton
            {
                text: "Прогнозный"
                property int value: 1
                ButtonGroup.group: viewModeGroup
            }
            RadioButton
            {
                text: "Комбинированный"
                property int value: 2
                ButtonGroup.group: viewModeGroup
            }
        }

        Label { text: "Масштаб:"; font.bold: true }
        RowLayout
        {
            spacing: 16
            ButtonGroup { id: zoomGroup }
            RadioButton
            {
                text: "День"
                checked: true
                property int value: 0
                ButtonGroup.group: zoomGroup
            }
            RadioButton
            {
                text: "Неделя"
                property int value: 1
                ButtonGroup.group: zoomGroup
            }
        }

        Label { text: "Элементы:"; font.bold: true }
        GridLayout
        {
            columns: 2
            columnSpacing: 16
            rowSpacing: 6

            CheckBox { id: progressCheck; text: "Прогресс"; checked: true }
            CheckBox { id: depsCheck; text: "Зависимости"; checked: true }
            CheckBox { id: historyCheck; text: "Переносы"; checked: true }
            CheckBox { id: commentsCheck; text: "Комментарии"; checked: true }
            CheckBox { id: holidaysCheck; text: "Праздники"; checked: false }
        }

        Label
        {
            id: errorLabel
            color: "#d32f2f"
            visible: false
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.pixelSize: 12
        }

        Item { Layout.fillHeight: true }

        RowLayout
        {
            Layout.fillWidth: true
            spacing: 10
            Layout.alignment: Qt.AlignRight

            Button
            {
                text: root.busy ? "Экспорт…" : "Экспорт"
                Layout.preferredHeight: 38
                Layout.minimumWidth: 120
                enabled: !root.busy && root.filePath !== ""
                onClicked:
                {
                    if (!projectController) return
                    root.busy = true
                    errorLabel.visible = false
                    var settings = {
                        showProgress: progressCheck.checked,
                        showDependencies: depsCheck.checked,
                        showTaskHistory: historyCheck.checked,
                        showComments: commentsCheck.checked,
                        showHolidays: holidaysCheck.checked,
                        viewMode: viewModeGroup.checkedButton ? viewModeGroup.checkedButton.value : 0,
                        zoomLevel: zoomGroup.checkedButton ? zoomGroup.checkedButton.value : 0
                    }
                    projectController.exportManager.setExportSettings(settings)
                    if (root.formatIndex === 0)
                        projectController.exportToPng(root.filePath)
                        else
                            projectController.exportToPdf(root.filePath)
                }
            }

            Button
            {
                text: "Отмена"
                Layout.preferredHeight: 38
                Layout.minimumWidth: 100
                enabled: !root.busy
                onClicked: root.close()
            }
        }
    }

    Connections
    {
        target: projectController ? projectController.exportManager : null
        function onExportFinished(success, message)
        {
            root.busy = false
            if (success) root.close()
            else
            {
                errorLabel.text = message
                errorLabel.visible = true
            }
        }
    }

    Labs.FileDialog
    {
        id: saveFileDialog
        title: "Сохранить как"
        fileMode: Labs.FileDialog.SaveFile
        defaultSuffix: root.formatIndex === 0 ? "png" : "pdf"
        nameFilters: root.formatIndex === 0
                     ? ["PNG изображения (*.png)", "Все файлы (*)"]
                     : ["PDF документы (*.pdf)", "Все файлы (*)"]
        onAccepted:
        {
            var p = file.toString()
            if (p.startsWith("file://")) p = p.substring(7)
            root.filePath = p
            pathField.text = p
        }
    }
}
