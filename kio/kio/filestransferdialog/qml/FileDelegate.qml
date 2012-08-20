import QtQuick 1.1
import org.kde.plasma.components 0.1
import FileTransferHelpers 1.0

Item {
    id: delegate;
    property string modelName: "modelNameWasNotSetup"
    anchors.left: parent.left
    anchors.right: parent.right
    height: 30
    clip: true

    BackgroundProgressBar {
        anchors.fill: parent
        value: progress
    }

    Text {
        id: file;
        height: delegate.height
        anchors.left: parent.left
        anchors.right: buttons.left
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideMiddle
        text: url
    }
    Component.onCompleted: console.log("Delegate: actions:"+actions+"; some:"+[FileHelper.Unfinished, FileHelper.Unreadable, FileHelper.Disappeared])
    Row {
        id: buttons
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 10
        spacing: 5

        MyToolButton {
            id: retryButton
            iconSource: "task-recurring"
            iconSize: theme.smallMediumIconSize
            visible: (actions === FileHelper.Skipped)
            onClicked: { enabled = false; dialog.retryFile(modelName, fid) }
        }
        MyToolButton {
            id: skipButton
            iconSource: "task-reject" //"list-remove"
            iconSize: theme.smallMediumIconSize
            visible: ([FileHelper.Unfinished, FileHelper.Unreadable, FileHelper.Disappeared].indexOf(actions) > -1)
            onClicked: { enabled = false; dialog.skipFile(modelName, fid) }
        }
        MyToolButton {
            id: openDirButton
            iconSource: "folder"
            iconSize: theme.smallMediumIconSize
            visible: true
            onClicked: {
                console.log("JS: opening dir for file \"" + fid + "\"")
            }
        }
    }

    SystemPalette { id: palette; colorGroup: SystemPalette.Active }
    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        //color: "#cccccc"
        color: palette.dark
        height: 1
    }

    ListView.onRemove: SequentialAnimation {
        PropertyAction { target: delegate; property: "ListView.delayRemove"; value: true }
        NumberAnimation { target: delegate; property: "scale"; to: 0; duration: 250; easing.type: Easing.InOutQuad }
        PropertyAction { target: delegate; property: "ListView.delayRemove"; value: false }
    }

}
