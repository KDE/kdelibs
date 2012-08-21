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
    id: progressBar
    property real maximumValue: 1.0
    property real finishedRatio : 0.4
    property real skippedRatio : 0.1
    property real errorsRatio : 0.1
    property real unfinishedRatio : maximumValue - finishedFiles - skippedFiles - errorsFiles
    property string finishedFiles
    property string skippedFiles
    property string errorsFiles
    property string unfinishedFiles
    property string finishedBytes
    property string skippedBytes
    property string errorsBytes
    property string unfinishedBytes
    property bool indeterminate: false

    signal normalDetails()
    signal errorsDetails()
    signal skippedDetails()

    width: 600
    height: 50
    radius: 0
    color: "#b35757"

    Rectangle {
        id: rightDecorator
        radius: progressBar.radius
        anchors.right: parent.right
        anchors.top: progressBar.top
        anchors.bottom: progressBar.bottom
        width: 0//radius * 2
        color: (unfinishedRatio !== 0) ? unfinished.color : (errorsRatio !== 0) ? errors.color : (skippedRatio !== 0) ? skipped.color : finished.color
    }

    Rectangle {
        id: leftDecorator
        radius: progressBar.radius
        anchors.left: progressBar.left
        anchors.top: progressBar.top
        anchors.bottom: progressBar.bottom
        width: 0//radius * 2
        color: (finishedRatio !== 0) ? finished.color : (skippedRatio !== 0) ? skipped.color : (errorsRatio !== 0) ? errors.color : unfinished.color
    }



    Rectangle {
        id: container
        anchors.fill: progressBar
        color: "transparent"
        //anchors.leftMargin: progressBar.radius
        //anchors.rightMargin: progressBar.radius
        width: parent.width// - 2 * parent.radius

        ProgressBarPart {
            id: finished
            files: progressBar.finishedFiles
            bytes: progressBar.finishedBytes
            ratio: progressBar.finishedRatio
            anchors.left: container.left
            width: desiredWidth - (skipped.stolenWidth + errors.stolenWidth) * (ratio / (finished.ratio + unfinished.ratio))
            color: "#23dd45"
            onShowDetails: progressBar.normalDetails()
            forceShadow: unfinished.hovered
            //onFilesChanged: {console.log("finished progPart: f:"+files+"; v:"+visible+" Ratio:"+ratio+"; w:"+width+"+; x:"+x+"; y:"+y)}
        }
        ProgressBarPart {
            id: skipped
            color: "#9c9797"
            files: progressBar.skippedFiles
            bytes: progressBar.skippedBytes
            ratio: progressBar.skippedRatio
            anchors.left: unfinished.right
            width: (ratio === 0) ? 0 : Math.max(desiredWidth, minWidth)
            onShowDetails: progressBar.skippedDetails()
            //onFilesChanged: {console.log("skipped progPart: f:"+files+"; v:"+visible+" Ratio:"+ratio+"; w:"+width+"+; x:"+x+"; y:"+y)}
        }
        ProgressBarPart {
            id: errors
            files: progressBar.errorsFiles
            bytes: progressBar.errorsBytes
            ratio: progressBar.errorsRatio
            color: "red"
            anchors.left: skipped.right
            width: (ratio === 0) ? 0 : Math.max(desiredWidth, minWidth)
            onShowDetails: progressBar.errorsDetails()
            //onFilesChanged: {console.log("errors progPart: f:"+files+"; v:"+visible+" Ratio:"+ratio+"; w:"+width+"+; x:"+x+"; y:"+y)}
        }
        ProgressBarPart {
            id: unfinished
            files: progressBar.unfinishedFiles
            bytes: progressBar.unfinishedBytes
            ratio: progressBar.unfinishedRatio
            anchors.left: finished.right
            width: desiredWidth - (skipped.stolenWidth + errors.stolenWidth) * (ratio / (finished.ratio + unfinished.ratio))
            onShowDetails: progressBar.normalDetails()
            forceShadow: finished.hovered
            //onFilesChanged: {console.log("unfinished progPart: f:"+files+"; v:"+visible+" Ratio:"+ratio+"; w:"+width+"+; x:"+x+"; y:"+y)}
        }
    }
    Image {
        id: upShade
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: parent.height * 0.22
        fillMode: Image.TileHorizontally
        smooth: true

        source: "img/progress-bar-up.png"
    }
    Image {
        id: downShade
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: parent.height * 0.13
        fillMode: Image.TileHorizontally
        smooth: true

        source: "img/progress-bar-down.png"
    }
}
