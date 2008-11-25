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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kurlmimetest.h"
#include <qtest_kde.h>
#include <QtCore/QMimeData>
#include <kurl.h>
#include <kdebug.h>

#include "kurlmimetest.moc"

QTEST_KDEMAIN_CORE( KUrlMimeTest )

void KUrlMimeTest::testURLList()
{
    QMimeData* mimeData = new QMimeData;
    QVERIFY( !KUrl::List::canDecode( mimeData ) );
    QVERIFY(!mimeData->hasUrls());

    KUrl::List urls;
    urls.append( KUrl( "http://www.kde.org" ) );
    urls.append( KUrl( "http://wstephenson:secret@example.com/path" ) );
    urls.append( KUrl( "file:///home/dfaure/konqtests/Mat%C3%A9riel" ) );
    QMap<QString, QString> metaData;
    metaData["key"] = "value";
    metaData["key2"] = "value2";

    urls.populateMimeData( mimeData, metaData );

    QVERIFY(KUrl::List::canDecode( mimeData ));
    QVERIFY(mimeData->hasUrls());

    QMap<QString, QString> decodedMetaData;
    KUrl::List decodedURLs = KUrl::List::fromMimeData( mimeData, &decodedMetaData );
    QVERIFY( !decodedURLs.isEmpty() );
    KUrl::List expectedUrls = urls;
    expectedUrls[1] = KUrl("http://wstephenson@example.com/path"); // password removed
    QCOMPARE( expectedUrls.toStringList().join(" "), decodedURLs.toStringList().join(" ") );

    QList<QUrl> qurls = mimeData->urls();
    QCOMPARE(qurls.count(), urls.count());
    for (int i = 0; i < qurls.count(); ++i )
        QCOMPARE(qurls[i], static_cast<QUrl>(decodedURLs[i]));

    QVERIFY( !decodedMetaData.isEmpty() );
    QCOMPARE( decodedMetaData["key"], QString( "value" ) );
    QCOMPARE( decodedMetaData["key2"], QString( "value2" ) );

    delete mimeData;
}

void KUrlMimeTest::testOneURL()
{
    KUrl oneURL( "file:///tmp" );
    QMimeData* mimeData = new QMimeData;

    oneURL.populateMimeData( mimeData );

    QVERIFY( KUrl::List::canDecode( mimeData ) );
    QMap<QString, QString> decodedMetaData;
    KUrl::List decodedURLs = KUrl::List::fromMimeData( mimeData, &decodedMetaData );
    QVERIFY( !decodedURLs.isEmpty() );
    QCOMPARE( decodedURLs.count(), 1 );
    QCOMPARE( decodedURLs[0].url(), oneURL.url() );
    QVERIFY( decodedMetaData.isEmpty() );
    delete mimeData;
}

void KUrlMimeTest::testFromQUrl()
{
    QList<QUrl> qurls;
    qurls.append( QUrl( "http://www.kde.org" ) );
    qurls.append( QUrl( "file:///home/dfaure/konqtests/Mat%C3%A9riel" ) );
    QMimeData* mimeData = new QMimeData;
    mimeData->setUrls(qurls);

    QVERIFY(KUrl::List::canDecode(mimeData));
    QMap<QString, QString> decodedMetaData;
    KUrl::List decodedURLs = KUrl::List::fromMimeData( mimeData, &decodedMetaData );
    QVERIFY( !decodedURLs.isEmpty() );
    QCOMPARE( decodedURLs.count(), 2 );
    QCOMPARE( static_cast<QUrl>(decodedURLs[0]), qurls[0] );
    QCOMPARE( static_cast<QUrl>(decodedURLs[1]), qurls[1] );
    QVERIFY( decodedMetaData.isEmpty() );
    delete mimeData;
}
