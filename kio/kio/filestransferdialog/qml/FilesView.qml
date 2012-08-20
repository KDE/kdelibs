import QtQuick 1.1

Rectangle {
    id: filesView
    property Component delegate
    property QtObject model

    Component {
        id: highlghitBar
        Rectangle {
            width: listView.width
            color: "lightsteelblue"
            //color: "#efefef"
        }
    }

    ListView {
        id: listView
        anchors.fill: parent
        model: filesView.model
        delegate: filesView.delegate
        //highlight: highlghitBar
        spacing: 0
        clip: true
        focus: true
    }
}
