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
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1

Rectangle {
    id: taskBar
    property alias showDetails: detailsButton.checked
    height: 40;

    Column {
        id: leftColumn
        anchors {
            left: taskBar.left
            leftMargin: 5
            verticalCenter: taskBar.verticalCenter
        }
        Text {
            text: "Progress:"
        }
    }

    MultiPartProgressBar {
        id: mppb
        color: "transparent"
        anchors {
            left: leftColumn.right
            right: rightRow.left
            leftMargin: 10
            rightMargin: 10
            verticalCenter: taskBar.verticalCenter
        }
        height: 32
        finishedRatio: 0
        errorsRatio: 0
        skippedRatio: 0
        unfinishedRatio: 0

        onNormalDetails: { taskBar.showDetails = true; detailedInfo.currentTab = normalContent }
        onSkippedDetails: { taskBar.showDetails = true; detailedInfo.currentTab = skippedContent }
        onErrorsDetails: { taskBar.showDetails = true; detailedInfo.currentTab = errorsContent }

        Connections { target: TransferModel; onFinishedAmountChanged: { mppb.finishedBytes = amounts[0]; mppb.finishedFiles = amounts[1]; mppb.finishedRatio = amounts[2] } }
        Connections { target: TransferModel; onErrorsAmountChanged: { mppb.errorsBytes = amounts[0]; mppb.errorsFiles = amounts[1]; mppb.errorsRatio = amounts[2] } }
        Connections { target: TransferModel; onSkippedAmountChanged: { mppb.skippedBytes = amounts[0]; mppb.skippedFiles = amounts[1]; mppb.skippedRatio = amounts[2] } }
        Connections { target: TransferModel; onUnfinishedAmountChanged: { mppb.unfinishedBytes = amounts[0]; mppb.unfinishedFiles = amounts[1]; mppb.unfinishedRatio = amounts[2] } }
    }

    signal stop
    signal suspend
    signal resume
    Row {
        id: rightRow
        anchors {
            right: parent.right
            margins: 5
            verticalCenter: parent.verticalCenter
        }
        spacing: 5

        MyToolButton {
            id: detailsButton
            iconSource: (checked) ? "arrow-up" : "arrow-down"
            iconSize: theme.mediumIconSize
            checkable: true
            onClicked: undefined
        }
        MyToolButton {
            id: pauseButton
            iconSize: theme.mediumIconSize
            iconSource: (checked) ? "media-playback-start" : "media-playback-pause"
            checkable: true
            onClicked: (checked) ? suspend() : resume()
        }

        
        MyToolButton {
            id: stopButton
            iconSize: theme.mediumIconSize
            iconSource: "media-playback-stop"
            onClicked: stop()
        }
    }
}
