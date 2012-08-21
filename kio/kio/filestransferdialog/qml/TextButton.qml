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

Rectangle {
    id: container

    property alias text: label.text

    signal clicked

    width: label.width + 20; height: label.height + 6
    smooth: true
    radius: 10

    gradient: Gradient {
        GradientStop { id: gradientStop; position: 0.0; color: palette.light }
        GradientStop { position: 1.0; color: palette.button }
    }

    SystemPalette { id: palette }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: container.clicked()
    }

    Text {
        id: label
        anchors.centerIn: parent
    }

    states: State {
        name: "pressed"
        when: mouseArea.pressed
        PropertyChanges { target: gradientStop; color: palette.dark }
    }
}
