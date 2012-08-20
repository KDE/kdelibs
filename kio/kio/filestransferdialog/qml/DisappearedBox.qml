import QtQuick 1.1
import org.kde.plasma.components 0.1
import "helpers.js" as Help

Rectangle {
    id: box
    property QtObject model
    visible: model.count !== 0
    height: (visible) ? childrenRect.height : 0

    Column {
        id: column
        width: parent.width
        spacing: 5

        Text {
            anchors.left: parent.left
            text: "<b>" + box.model.count + " files have disappeared from the source folder</b>"
                  + "<br/>Possible solutions:"
        }

        Column {
            id: description
            anchors.left: parent.left
            anchors.leftMargin: 5
            anchors.right: parent.right
            spacing: 5

            ManualSolution {
                id: manualSolution
                anchors.left: parent.left
                anchors.right: parent.right
                color: Help.randColor()
                onRetryAllClicked: dialog.retryAll(box.model.objectName)
                onSkipAllClicked: dialog.skipAll(box.model.objectName)
            }

            FilesView {
                visible: manualSolution.showFiles
                width: parent.width
                height: 100
                delegate: FileDelegate { modelName: box.model.objectName }
                model: box.model
            }
        }
        Rectangle {
            width: errorsView.availableWidth
            color: palette.dark
            height: 2
        }
    }
}
