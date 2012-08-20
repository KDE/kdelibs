import QtQuick 1.1

TabWidget {
    id: tabs
    width: 640; height: 480

    Rectangle {
        property string title: "In Process"
        anchors.fill: parent
        color: "#e3e3e3"

        Rectangle {
            anchors.fill: parent; anchors.margins: 20
            color: "#ff7f7f"
            Text {
                width: parent.width - 20
                anchors.centerIn: parent; horizontalAlignment: Qt.AlignHCenter
                text: "Roses are red"
                font.pixelSize: 20
                wrapMode: Text.WordWrap
            }
        }
    }

    Rectangle {
        property string title: "Finished"
        anchors.fill: parent
        color: "#e3e3e3"

        Rectangle {
            anchors.fill: parent; anchors.margins: 20
            color: "#7fff7f"
            Text {
                width: parent.width - 20
                anchors.centerIn: parent; horizontalAlignment: Qt.AlignHCenter
                text: "Flower stems are green"
                font.pixelSize: 20
                wrapMode: Text.WordWrap
            }
        }
    }


    Rectangle {
        property string title: "Errors"
        anchors.fill: parent; color: "#e3e3e3"

        Rectangle {
            anchors.fill: parent; anchors.margins: 20
            color: "#7f7fff"
            Text {
                width: parent.width - 20
                anchors.centerIn: parent; horizontalAlignment: Qt.AlignHCenter
                text: "Violets are blue"
                font.pixelSize: 20
                wrapMode: Text.WordWrap
            }
        }
    }

    Rectangle {
        property string title: "Skipped"
        anchors.fill: parent; color: "#e3e3e3"

        SystemPalette { id: palette; colorGroup: SystemPalette.Active }
        SkippedView {
            id: skippedView
            width: parent.width
            //height: parent.height - taskBar.height
            height: parent.height
            color: palette.base
        }
    }
}

