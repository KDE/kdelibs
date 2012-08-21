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
