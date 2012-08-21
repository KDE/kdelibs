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
import "helpers.js" as Help

Rectangle {
    id: box
    property QtObject model
    visible: model.count !== 0
    height: (visible) ? childrenRect.height : 0

    Column {
        id: column
        width: parent.width
        spacing: 5

        Text {
            anchors.left: parent.left
            text: "<b>" + box.model.count + " files have disappeared from the source folder</b>"
                  + "<br/>Possible solutions:"
        }

        Column {
            id: description
            anchors.left: parent.left
            anchors.leftMargin: 5
            anchors.right: parent.right
            spacing: 5

            ManualSolution {
                id: manualSolution
                anchors.left: parent.left
                anchors.right: parent.right
                color: Help.randColor()
                onRetryAllClicked: dialog.retryAll(box.model.objectName)
                onSkipAllClicked: dialog.skipAll(box.model.objectName)
            }

            FilesView {
                visible: manualSolution.showFiles
                width: parent.width
                height: 100
                delegate: FileDelegate { modelName: box.model.objectName }
                model: box.model
            }
        }
        Rectangle {
            width: errorsView.availableWidth
            color: palette.dark
            height: 2
        }
    }
}
