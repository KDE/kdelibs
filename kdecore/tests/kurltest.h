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

#ifndef kurltest_h
#define kurltest_h

#include <QtCore/QObject>

class KUrlTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testQUrl();
    void testEmptyURL();
    void testIsValid();
    void testSetQuery();
    void testEmptyNullReference();
    void testSetRef();
    void testSetHTMLRef();
    void testDirectory();
    void testUtf8();
    void testIsLocalFile();
    void testSimpleMethods();
    void testHostName();
    void testParsingTolerance();
    void testNewLine();
    void testQueryParsing();
    void testEmptyQueryOrRef();
    void testURLsWithoutPath();
    void testPathAndQuery();
    void testUpUrl();
    void testSetFileName();
    void testPrettyURL();
    void testIsRelative();
    void testRelativePath();
    void testRelativeURL();
    void testAdjustPath();
    void testIPV6();
    void testBaseURL();
    void testSetUser();
    void testSetEncodedFragment_data();
    void testSetEncodedFragment();
    void testSubURL();
    void testComparisons();
    void testStreaming();
    void testBrokenStuff();
    void testMoreBrokenStuff();
    void testMailto();
    void testSmb();
    void testOtherProtocols();
    void testOtherEncodings();
    void testPathOrURL();
    void testAssignment();
    void testQueryItem();
    void testEncodeString();
    void testIdn();
    void testUriMode();
    void testToLocalFile();
    void testUrl_data();
    void testUrl();
    void testToStringList();
};

#endif

