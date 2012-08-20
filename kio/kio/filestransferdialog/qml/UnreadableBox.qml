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
            text: "<b>You are not allowed to read " + model.count + " files</b>"
                 +"<br/> Possible solutions:"

        }
        Column {
            id: description
            anchors.left: parent.left
            anchors.leftMargin: 5
            anchors.right: parent.right
            spacing: 5

            Rectangle {
                id: autoSolutions
                anchors.left: parent.left
                anchors.right: parent.right
                height: childrenRect.height
                color: Help.randColor()
                Text {
                    anchors.left: parent.left
                    anchors.verticalCenter: buttons.verticalCenter
                    text: "Automatically change rights:"
                }
                Row {
                    id: buttons
                    anchors.right: parent.right
                    spacing: 5
                    Button {
                        id: onlyForCopyingFilesButton
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Only for listed files"
                        onClicked: dialog.changeRights(box.model.objectName, false)
                    }
                    Button {
                        id: alwaysButton
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Always"
                        onClicked: dialog.changeRights(box.model.objectName, true)
                    }
                }
            }

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
