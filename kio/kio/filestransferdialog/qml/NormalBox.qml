import QtQuick 1.1
import org.kde.plasma.components 0.1

Rectangle {
    id: box
    property QtObject model
    Column {
        anchors.fill: parent
        Rectangle {
            id: description
            height: 40
            width: parent.width
            color: box.color
            Text {
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.verticalCenter: parent.verticalCenter
                text: "List of copying files:"
            }
            Button {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                text: "Clear already copied files"
                onClicked: dialog.clearFinished(box.model.objectName)
            }
        }

        FilesView {
            width: parent.width
            height: parent.height - description.height
            delegate: FileDelegate {modelName: box.model.objectName}
            model: box.model
        }
    }
}
