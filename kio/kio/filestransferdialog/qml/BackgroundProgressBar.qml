// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle {
    id: container
    property real value: 0
    Rectangle {
        id: filled
        anchors.top: container.top
        anchors.bottom: container.bottom
        anchors.left: container.left
        width: parent.width * container.value
        color: "lightblue"
    }
}
