// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle {
    property alias text: name.text
    width: name.width + 10
    height: 26
    color: "#7bd3d3d3" //"#7bffff00"
    radius: 11
    Text {
        id: name
        anchors.centerIn: parent
    }
}
