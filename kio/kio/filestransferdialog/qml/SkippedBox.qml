import QtQuick 1.1
import org.kde.plasma.components 0.1

Rectangle {
    id: box
    property QtObject model
    property bool empty: model.count === 0
    Column {
        anchors.fill: parent
        Rectangle {
            id: description
            height: 50
            width: parent.width
            color: box.color
            Text {
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.verticalCenter: parent.verticalCenter
                text: "Copying the following files has been skipped:"
            }
            Button {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                text: "Clear list"
                enabled: false
                onClicked: dialog.clearSkipped(box.model.objectName)
            }
        }

        FilesView {
            width: parent.width
            height: parent.height - description.height
            delegate: FileDelegate { modelName: box.model.objectName }
            model: box.model
        }
    }
}
