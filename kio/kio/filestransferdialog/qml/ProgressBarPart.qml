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
