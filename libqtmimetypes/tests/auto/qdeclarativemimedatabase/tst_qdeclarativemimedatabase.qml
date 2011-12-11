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

import "testdata_list.js" as TestData

TestCase {
    name: "tst_QDeclarativeMimedatabase"

    function pngMimeTypeName() {
        return "image/png"
    }

    function pngMimeTypeComment() {
        return "PNG File";
    }

    function pngMimeTypeGenericIconName() {
        return "/usr/share/icons/oxygen/64x64/mimetypes/image-x-generic.png"
    }

    function pngMimeTypeIconName() {
        return "/usr/share/icons/oxygen/64x64/mimetypes/image-x-generic.png"
    }

    function firstPngMimeTypeSuffixes() {
        return ".png"
    }

    MimeType {
        id: instantiatedPngMimeType
        name: pngMimeTypeName()
        genericIconName: pngMimeTypeGenericIconName()
        iconName: pngMimeTypeIconName()
    }

    MimeType {
        id: otherPngMimeType
    }

    function test_name() {
        otherPngMimeType.assign(instantiatedPngMimeType)
        otherPngMimeType.name = ""

        // Verify that the Id is part of the equality test:
        compare(instantiatedPngMimeType.name, pngMimeTypeName())

        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)
    }
    
    MimeDatabase {
        id: database
    }

    function list() {
        return TestData.list()
//        return [
//                   { filePath: "test.bmp", mimeType: "image/bmp", xFail: "" },
//                   { filePath: "archive.tar.gz", mimeType: "application/x-compressed-tar", xFail: "ox" }
//               ]
    }

    function listValue(ix) {
        return list()[ix]
    }

    function fileInfoSuffix(fileName)
    {
        var foundSuffix = "";

        var mimeTypeNames = database.mimeTypeNames
        for (var mimeTypeIx = 0; mimeTypeIx < mimeTypeNames.length; ++mimeTypeIx) {
            var suffixes = database.mimeTypeForName(mimeTypeNames[mimeTypeIx]).suffixes
            //print(mimeTypeNames[mimeTypeIx], suffixes.length)
            for (var suffixIx = 0; suffixIx < suffixes.length; ++suffixIx) {
                var suffix = suffixes[suffixIx]
                //print(mimeTypeNames[mimeTypeIx], '*.' + suffix, suffix.length, '*.' + foundSuffix, foundSuffix.length)
                if (fileName.length >= suffix.length + 1 && suffix.length > foundSuffix.length) {
                    if (fileName.slice(-(suffix.length + 1)) == '.' + suffix) {
                        //print(mimeTypeNames[mimeTypeIx], '*.' + suffix)
                        foundSuffix = suffix
                    }
                }
            }
        }

        return foundSuffix
    }
    
    function listContains(stringList, searchString)
    {
        // TODO sort the list and perform binary search!
        for (var stringListIx = 0; stringListIx < stringList.length; ++stringListIx) {
            if (stringList[stringListIx] == searchString) {
                return true
            }
        }
        
        return false;
    }

    function test_findByName() {
        var ok = 0

        for (var listIx = 0; listIx < list().length; ++listIx) {
            //print(listValue(listIx).filePath)

            var resultMimeTypeName = database.findByName(listValue(listIx).filePath).name
            //print("findByName(" + listValue(listIx).filePath + ") returned", resultMimeTypeName)

            // Results are ambiguous when multiple MIME types have the same glob
            // -> accept the current result if the found MIME type actually
            // matches the file's extension.
            var foundMimeType = database.mimeTypeForName(resultMimeTypeName);
            var extension = fileInfoSuffix(listValue(listIx).filePath);
            //print("findNameSuffix() returned", extension)
            //print("globPatterns:", foundMimeType.globPatterns, "- extension:", "*." + extension)
            if (listContains(foundMimeType.globPatterns, "*." + extension)) {
                ok = ok + 1
                continue;
            }

            // Expected to fail
            if (listValue(listIx).xFail.length >= 1 && listValue(listIx).xFail[0] == "x") {
                if (resultMimeTypeName != listValue(listIx).mimeType) {
                    ok = ok + 1
                }
                else {
                    print("Error:", listValue(listIx).filePath, "has xFail[0] == \"x\" but has a correct resultMimeTypeName")
                }
            }
            else {
                if (resultMimeTypeName == listValue(listIx).mimeType) {
                    ok = ok + 1
                }
                else {
                    print("Error:", listValue(listIx).filePath, "has been identified incorrectly as", resultMimeTypeName)
                }
            }
        }

        compare(ok, list().length)
    }
    
    function test_findByFile() {
        var ok = 0

        for (var listIx = 0; listIx < list().length; ++listIx) {
            //print(listValue(listIx).filePath)

            var resultMimeTypeName = database.findByFile(listValue(listIx).filePath).name
            //print("findByFile(" + listValue(listIx).filePath + ") returned", resultMimeTypeName)

            // Expected to fail
            if (listValue(listIx).xFail.length >= 3 && listValue(listIx).xFail[2] == "x") {
                if (resultMimeTypeName != listValue(listIx).mimeType) {
                    ok = ok + 1
                }
                else {
                    print("Error:", listValue(listIx).filePath, "has xFail[2] == \"x\" but has a correct resultMimeTypeName")
                }
            }
            else {
                if (resultMimeTypeName == listValue(listIx).mimeType) {
                    ok = ok + 1
                }
                else {
                    print("Error:", listValue(listIx).filePath, "has been identified incorrectly as", resultMimeTypeName, listValue(listIx).xFail.length, listValue(listIx).xFail[2])
                }
            }
        }

        compare(ok, list().length)
    }
}
