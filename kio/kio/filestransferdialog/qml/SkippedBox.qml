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
    id: box
    property QtObject model
    property bool empty: model.count === 0
    Column {
        anchors.fill: parent
        Rectangle {
            id: description
            height: 50
            width: parent.width
            color: box.color
            Text {
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.verticalCenter: parent.verticalCenter
                text: "Copying the following files has been skipped:"
            }
            Button {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                text: "Clear list"
                enabled: false
                onClicked: dialog.clearSkipped(box.model.objectName)
            }
        }

        FilesView {
            width: parent.width
            height: parent.height - description.height
            delegate: FileDelegate { modelName: box.model.objectName }
            model: box.model
        }
    }
}
