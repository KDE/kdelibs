/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtMimeTypes addon of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

import Qt 4.7
import QtTest 1.0
import QtMimeTypes 1.0

import "tst_qdeclarativemimetypedata.js" as Data

TestCase {
    name: "tst_QDeclarativeMimeTypeProperties"

    MimeType {
        id: firstMimeType
        name: Data.mimeTypeName()
        genericIconName: Data.mimeTypeGenericIconName()
        iconName: Data.mimeTypeIconName()
        globPatterns: Data.mimeTypeGlobPatterns()
    }

    MimeType {
        id: secondMimeType
    }

    MimeType {
        id: defaultMimeType
    }

    function test_Elements() {
        secondMimeType.assign(firstMimeType)
        compare(secondMimeType.equals(firstMimeType), true)
        compare(secondMimeType.equals(defaultMimeType), false)
        compare(firstMimeType.equals(secondMimeType), true)
        compare(defaultMimeType.equals(secondMimeType), false)

        secondMimeType.assign(defaultMimeType)
        compare(secondMimeType.equals(firstMimeType), false)
        compare(secondMimeType.equals(defaultMimeType), true)
        compare(firstMimeType.equals(secondMimeType), false)
        compare(defaultMimeType.equals(secondMimeType), true)
    }

    function test_JavaScriptObjects() {
        secondMimeType.assign(firstMimeType)   // to uncover problems in assignProperties()
        var javaScriptObject = new Object

        compare(firstMimeType.equalsProperties(javaScriptObject), false)
        secondMimeType.assignProperties(javaScriptObject);
        compare(firstMimeType.equals(secondMimeType), false)

        javaScriptObject.name = Data.mimeTypeName()
        javaScriptObject.genericIconName = Data.mimeTypeGenericIconName()
        javaScriptObject.iconName = Data.mimeTypeIconName()
        javaScriptObject.globPatterns = Data.mimeTypeGlobPatterns()
        compare(firstMimeType.equalsProperties(javaScriptObject), true)
        secondMimeType.assignProperties(javaScriptObject);
        compare(firstMimeType.equals(secondMimeType), true)
    }

    function test_name() {
        // Verify that the Name is part of the equality test:
        //compare(firstMimeType.name, Data.mimeTypeName())
        compare(Data.equalMimeTypeName(firstMimeType.name, Data.mimeTypeName()), true)

        secondMimeType.assign(firstMimeType)

        secondMimeType.name = defaultMimeType.name   // simulate an error
        compare(firstMimeType.equals(secondMimeType), false)
        secondMimeType.name = firstMimeType.name
        compare(firstMimeType.equals(secondMimeType), true)

        var javaScriptObject = firstMimeType.properties()

        javaScriptObject.name = defaultMimeType.name   // simulate an error
        compare(firstMimeType.equalsProperties(javaScriptObject), false)
        secondMimeType.assignProperties(javaScriptObject);
        compare(firstMimeType.equals(secondMimeType), false)

        javaScriptObject.name = firstMimeType.name
        compare(firstMimeType.equalsProperties(javaScriptObject), true)
        secondMimeType.assignProperties(javaScriptObject);
        compare(firstMimeType.equals(secondMimeType), true)
    }

    function test_genericIconName() {
        // Verify that the GenericIconName is part of the equality test:
        //compare(firstMimeType.genericIconName, Data.mimeTypeGenericIconName())
        compare(Data.equalMimeTypeGenericIconName(firstMimeType.genericIconName, Data.mimeTypeGenericIconName()), true)

        secondMimeType.assign(firstMimeType)

        secondMimeType.genericIconName = defaultMimeType.genericIconName   // simulate an error
        compare(firstMimeType.equals(secondMimeType), false)
        secondMimeType.genericIconName = firstMimeType.genericIconName
        compare(firstMimeType.equals(secondMimeType), true)

        var javaScriptObject = firstMimeType.properties()

        javaScriptObject.genericIconName = defaultMimeType.genericIconName   // simulate an error
        compare(firstMimeType.equalsProperties(javaScriptObject), false)
        secondMimeType.assignProperties(javaScriptObject);
        compare(firstMimeType.equals(secondMimeType), false)

        javaScriptObject.genericIconName = firstMimeType.genericIconName
        compare(firstMimeType.equalsProperties(javaScriptObject), true)
        secondMimeType.assignProperties(javaScriptObject);
        compare(firstMimeType.equals(secondMimeType), true)
    }

    function test_iconName() {
        // Verify that the IconName is part of the equality test:
        //compare(firstMimeType.iconName, Data.mimeTypeIconName())
        compare(Data.equalMimeTypeIconName(firstMimeType.iconName, Data.mimeTypeIconName()), true)

        secondMimeType.assign(firstMimeType)

        secondMimeType.iconName = defaultMimeType.iconName   // simulate an error
        compare(firstMimeType.equals(secondMimeType), false)
        secondMimeType.iconName = firstMimeType.iconName
        compare(firstMimeType.equals(secondMimeType), true)

        var javaScriptObject = firstMimeType.properties()

        javaScriptObject.iconName = defaultMimeType.iconName   // simulate an error
        compare(firstMimeType.equalsProperties(javaScriptObject), false)
        secondMimeType.assignProperties(javaScriptObject);
        compare(firstMimeType.equals(secondMimeType), false)

        javaScriptObject.iconName = firstMimeType.iconName
        compare(firstMimeType.equalsProperties(javaScriptObject), true)
        secondMimeType.assignProperties(javaScriptObject);
        compare(firstMimeType.equals(secondMimeType), true)
    }

    function test_globPatterns() {
        // Verify that the GlobPatterns is part of the equality test:
        //compare(firstMimeType.globPatterns, Data.mimeTypeGlobPatterns())
        compare(Data.equalMimeTypeGlobPatterns(firstMimeType.globPatterns, Data.mimeTypeGlobPatterns()), true)

        secondMimeType.assign(firstMimeType)

        secondMimeType.globPatterns = defaultMimeType.globPatterns   // simulate an error
        compare(firstMimeType.equals(secondMimeType), false)
        secondMimeType.globPatterns = firstMimeType.globPatterns
        compare(firstMimeType.equals(secondMimeType), true)

        var javaScriptObject = firstMimeType.properties()

        javaScriptObject.globPatterns = defaultMimeType.globPatterns   // simulate an error
        compare(firstMimeType.equalsProperties(javaScriptObject), false)
        secondMimeType.assignProperties(javaScriptObject);
        compare(firstMimeType.equals(secondMimeType), false)

        javaScriptObject.globPatterns = firstMimeType.globPatterns
        compare(firstMimeType.equalsProperties(javaScriptObject), true)
        secondMimeType.assignProperties(javaScriptObject);
        compare(firstMimeType.equals(secondMimeType), true)
    }
}
