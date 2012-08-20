import QtQuick 1.1
import org.kde.plasma.components 0.1

Rectangle {
    id: part
    property string files
    property string bytes
    property real ratio
    property real minWidth: 70
    property real desiredWidth: container.width * ratio
    property real stolenWidth : width - desiredWidth
    property bool forceShadow: false
    property alias hovered: mouseArea.containsMouse

    signal showDetails()

    radius: 0
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    visible: ratio !== 0.0

    Column {
        anchors.centerIn: part
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: part.files
            visible: width < part.width
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: part.bytes
            visible: width < part.width
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true;
        onClicked: part.showDetails()
        Rectangle {
            id: shadow
            anchors.fill: parent
            visible: mouseArea.containsMouse || part.forceShadow
            color: "lightgrey"
            opacity: 0.6
        }
    }






}
