/* This file is part of the KDE libraries
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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kunittest/tester.h>
#include <kunittest/module.h>
#include <qmimedata.h>
#include <kurl.h>
#include <kdebug.h>

class KURLMimeTest : public KUnitTest::Tester
{
public:
    void allTests() {
        testURLList();
        testOneURL();
    }

    void testURLList();
    void testOneURL();
};

KUNITTEST_MODULE( kunittest_kurlmimetest, "KURLMimeTest" );
KUNITTEST_MODULE_REGISTER_TESTER( KURLMimeTest );

void KURLMimeTest::testURLList()
{
    kdDebug() << k_funcinfo << endl;
    QMimeData* mimeData = new QMimeData;
    KURL::List urls;
    urls.append( KURL( "http://www.kde.org" ) );
    urls.append( KURL( "file:///home/dfaure/konqtests/Mat%C3%A9riel" ) );
    QMap<QString, QString> metaData;
    metaData["key"] = "value";
    metaData["key2"] = "value2";

    urls.addToMimeData( mimeData, metaData );

    QMap<QString, QString> decodedMetaData;
    KURL::List decodedURLs = KURL::List::fromMimeData( mimeData, &decodedMetaData );
    VERIFY( !decodedURLs.isEmpty() );
    CHECK( urls.toStringList().join(" "), decodedURLs.toStringList().join(" ") );
    VERIFY( !decodedMetaData.isEmpty() );
    CHECK( decodedMetaData["key"], QString( "value" ) );
    CHECK( decodedMetaData["key2"], QString( "value2" ) );

    delete mimeData;
}

void KURLMimeTest::testOneURL()
{
    KURL oneURL = "file:///tmp";
    QMimeData* mimeData = new QMimeData;

    oneURL.addToMimeData( mimeData );

    QMap<QString, QString> decodedMetaData;
    KURL::List decodedURLs = KURL::List::fromMimeData( mimeData, &decodedMetaData );
    VERIFY( !decodedURLs.isEmpty() );
    CHECK( decodedURLs.count(), 1 );
    CHECK( decodedURLs[0].url(), oneURL.url() );
    VERIFY( decodedMetaData.isEmpty() );
    delete mimeData;
}
