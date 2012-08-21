/*  This file is part of the KDE project
    Copyright (C) 2012 Cyril Oblikov <munknex@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2+ as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

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
