import QtQuick 1.1
import org.kde.plasma.components 0.1

Rectangle {
    id: manualSolutions
    signal retryAllClicked()
    signal skipAllClicked()
    property alias showFiles : showFilesTrigger.checked
    height: childrenRect.height
    Text {
        anchors.left: parent.left
        anchors.verticalCenter: buttons.verticalCenter
        text: "Manually solve the problem:"
    }
    Row {
        id: buttons
        anchors.right: parent.right
        spacing: 5
        Button {
            id: retryAllButton
            anchors.verticalCenter: parent.verticalCenter
            text: "Retry"
            onClicked: retryAllClicked()
        }
        Button {
            id: skipAllButton
            anchors.verticalCenter: parent.verticalCenter
            text: "Skip these files"
            onClicked: skipAllClicked()
        }
        Button {
            id: showFilesTrigger
            checkable: true
            iconSource: (checked) ? "arrow-up" : "arrow-down"
        }
    }
}
