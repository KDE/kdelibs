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

