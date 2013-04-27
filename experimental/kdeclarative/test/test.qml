/*
 *   Copyright 2011 Marco Martin <mart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets

Rectangle {
    width: 300
    height: 300
    color: "red"

    Component.onCompleted: {
        print(testObject.prop)
        testObject.prop = i18n("New text")
        print(testObject.prop)
        //QtScript binded elements don't appear to notify
        textElement.text = testObject.prop

        //test urls
        var url = new Url("http://www.kde.org")
        print(url.protocol)
        print(url.host)

        //test icons
        iconWidget.icon = new QIcon("konqueror")
    }
    Column {
        anchors.fill: parent
        Text {
            id: textElement
            text: testObject.prop
        }
        PlasmaWidgets.IconWidget {
            id: iconWidget
        }
    }
}
