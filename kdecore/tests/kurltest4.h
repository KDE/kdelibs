/*
    Copyright (c) 2005 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef ktempfiletest_h
#define ktempfiletest_h

#include <qobject.h>

class KURLTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testQUrl();
    void testEmptyURL();
    void testIsValid();
    void testSetQuery();
    void testSetRef();
    void testDirectory();
    void testUtf8();
    void testIsLocalFile();
    void testSimpleMethods();
    void testParsingTolerance();
    void testNewLine();
    void testQueryParsing();
    void testEmptyQueryOrRef();
    void testURLsWithoutPath();
    void testPathAndQuery();
    void testSetFileName();
    void testPrettyURL();
    void testIsRelative();
    void testRelativePath();
    void testRelativeURL();
    void testAdjustPath();
    void testIPV6();
    void testBaseURL();
    void testSetUser();
    void testSubURL();
    void testComparisons();
    void testStreaming();
    void testBrokenStuff();
    void testMailto();
    void testSmb();
    void testOtherProtocols();
    void testOtherEncodings();
    void testPathOrURL();
    void testQueryItem();
    void testEncodeString();
    void testIdn();
    void testUriMode();
};

#endif

