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
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import "helpers.js" as Help
import FileTransferHelpers 1.0

Rectangle {
    id: dialog
    property real defaultWidth: 600
    property real defaultHeight: 400
    width: defaultWidth
    height: taskBar.height
    onHeightChanged: MainWindow.size.height = height
    Behavior on height {
        NumberAnimation {
            duration: 250
        }
    }
    SystemPalette { id: palette; colorGroup: SystemPalette.Active }
    color: palette.window
    Rectangle {
        id: transparentBackground
        width: dialog.width
        height: dialog.height
        color: "#00000000"
    }

    // for manual resolution
    signal retryFile(string modelName, int id)
    signal retryAll(string modelName)
    signal skipFile(string modelName, int id)
    signal skipAll(string modelName)

    // for unreadable files
    signal changeRights(string modelName, bool always)

    // for normal files
    signal clearFinished(string modelName)

    // for skipped files
    signal clearSkipped(string modelName)

    onRetryFile: TransferModel.retryFile(modelName, id)
    onSkipFile: {console.log(modelName + " " + id); TransferModel.skipFile(modelName, id)}
    onSkipAll: TransferModel.skipAllFiles(modelName)
    onRetryAll: TransferModel.retryAllFiles(modelName)
    onClearFinished: TransferModel.clearFinished(modelName)

    TaskBar {
        id:taskBar
        anchors { left: parent.left; right: parent.right; top: parent.top }
        //color: "green"
        color: dialog.color
        opacity: 1
        onStop: TransferModel.cancel()
        onSuspend: TransferModel.suspend()
	onResume: TransferModel.resume()
    }
//    TabBar {
//        id: tabBar
//        width: 400
//        anchors.top: taskBar.bottom
//        anchors.horizontalCenter: parent.horizontalCenter
//        TabButton { tab: normalContent; text: "All" }
//        TabButton { tab: errorsContent; text: "Errors" }
//        TabButton { tab: skippedContent; text: "Skipped" }
//    }
//    Rectangle {
//        id: tabBar
//        width: 400
//        anchors.top: taskBar.bottom
//        anchors.horizontalCenter: parent.horizontalCenter
//    }
    Connections {
        target: taskBar
        onShowDetailsChanged: (taskBar.showDetails) ? dialog.height = dialog.defaultHeight : dialog.height = taskBar.height
    }
    TabGroup {
        id: detailedInfo
        anchors.margins: 5
        anchors {
            left: parent.left; right: parent.right;
            top: taskBar.bottom; bottom: parent.bottom
        }
        visible: taskBar.showDetails
        Page {
            id: normalContent
            NormalBox {
                anchors.fill: parent
                color: dialog.color
                model: TransferModel.normalModel()
            }
        }
        Page {
            id: skippedContent
            SkippedBox {
                onEmptyChanged: (empty) ? detailedInfo.currentTab = normalContent : undefined
                anchors.fill: parent
                color: dialog.color
                model: TransferModel.skippedModel()
            }
        }
        Page {
            id: fakeContent
            Rectangle {
                anchors.fill: parent
                color: dialog.color
            }
        }
        Page {
            id: errorsContent
            property bool empty: !(unreadableBox.visible || disappearedBox.visible)
            onEmptyChanged: (empty) ? detailedInfo.currentTab = normalContent : undefined
            Rectangle {
                anchors.fill: parent
                color: Help.randColor()

                VisualItemModel {
                    id: errorsModel
                    UnreadableBox {
                        id: unreadableBox
                        width: errorsView.availableWidth
                        color: Help.randColor()
                        model: TransferModel.unreadableModel()
                    }
                    DisappearedBox {
                        id:disappearedBox
                        width: errorsView.availableWidth
                        color: Help.randColor()
                        model: TransferModel.disappearedModel()
                    }
                }

                ListView {
                    id: errorsView
                    property int availableWidth: width - (scrollBar.visible ? scrollBar.width+5 : 0)
                    anchors.fill: parent
                    focus: true
                    clip: true
                    spacing: 4
                    model: errorsModel
                    ScrollBar {
                        id: scrollBar
                        orientation: Qt.Vertical
                        flickableItem: parent
                        anchors {
                            top: parent.top
                            right: parent.right
                            bottom: parent.bottom
                        }
                    }
                }
            }
        }
    }
}
