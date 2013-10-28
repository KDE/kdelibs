/* This file is part of the KDE libraries
    Copyright (c) 2005 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

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
#include <QtTest>
#include <QMimeData>
#include <kurl.h>

QTEST_MAIN( KUrlMimeTest )

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
    metaData[QLatin1String("key")] = QLatin1String("value");
    metaData[QLatin1String("key2")] = QLatin1String("value2");

    urls.populateMimeData( mimeData, metaData );

    QVERIFY(KUrl::List::canDecode( mimeData ));
    QVERIFY(mimeData->hasUrls());
    QVERIFY(mimeData->hasText());

    QMap<QString, QString> decodedMetaData;
    KUrl::List decodedURLs = KUrl::List::fromMimeData( mimeData, KUrl::List::PreferKdeUrls, &decodedMetaData );
    QVERIFY( !decodedURLs.isEmpty() );
    KUrl::List expectedUrls = urls;
    expectedUrls[1] = KUrl("http://wstephenson:secret@example.com/path"); // password kept, unlike in KDE4, but that's okay, it's not displayed
    const QString space(QLatin1Char(' '));
    QCOMPARE( expectedUrls.toStringList().join(space), decodedURLs.toStringList().join(space) );

    const QList<QUrl> qurls = mimeData->urls();
    QCOMPARE(qurls.count(), urls.count());
    for (int i = 0; i < qurls.count(); ++i )
        QCOMPARE(qurls[i], static_cast<QUrl>(decodedURLs[i]));

    QVERIFY( !decodedMetaData.isEmpty() );
    QCOMPARE( decodedMetaData[QLatin1String("key")], QString::fromLatin1( "value" ) );
    QCOMPARE( decodedMetaData[QLatin1String("key2")], QString::fromLatin1( "value2" ) );

    delete mimeData;
}

void KUrlMimeTest::testOneURL()
{
    KUrl oneURL( "file:///tmp" );
    QMimeData* mimeData = new QMimeData;

    oneURL.populateMimeData( mimeData );

    QVERIFY( KUrl::List::canDecode( mimeData ) );
    QMap<QString, QString> decodedMetaData;
    KUrl::List decodedURLs = KUrl::List::fromMimeData( mimeData, KUrl::List::PreferKdeUrls, &decodedMetaData );
    QVERIFY( !decodedURLs.isEmpty() );
    QCOMPARE( decodedURLs.count(), 1 );
    QCOMPARE( decodedURLs[0].url(), oneURL.url() );
    QVERIFY( decodedMetaData.isEmpty() );
    delete mimeData;
}

void KUrlMimeTest::testFromQUrl()
{
    QList<QUrl> qurls;
    qurls.append( QUrl( QLatin1String("http://www.kde.org") ) );
    qurls.append( QUrl( QLatin1String("file:///home/dfaure/konqtests/Mat%C3%A9riel") ) );
    QMimeData* mimeData = new QMimeData;
    mimeData->setUrls(qurls);

    QVERIFY(KUrl::List::canDecode(mimeData));
    QMap<QString, QString> decodedMetaData;
    KUrl::List decodedURLs = KUrl::List::fromMimeData( mimeData, KUrl::List::PreferKdeUrls, &decodedMetaData );
    QVERIFY( !decodedURLs.isEmpty() );
    QCOMPARE( decodedURLs.count(), 2 );
    QCOMPARE( static_cast<QUrl>(decodedURLs[0]), qurls[0] );
    QCOMPARE( static_cast<QUrl>(decodedURLs[1]), qurls[1] );
    QVERIFY( decodedMetaData.isEmpty() );
    delete mimeData;
}

void KUrlMimeTest::testMostLocalUrlList()
{
    QMimeData* mimeData = new QMimeData;
    KUrl::List urls;
    urls.append(KUrl("desktop:/foo"));
    urls.append(KUrl("desktop:/bar"));
    KUrl::List localUrls;
    localUrls.append(KUrl("file:/home/dfaure/Desktop/foo"));
    localUrls.append(KUrl("file:/home/dfaure/Desktop/bar"));

    urls.populateMimeData(localUrls, mimeData);

    QVERIFY(KUrl::List::canDecode(mimeData));
    QVERIFY(mimeData->hasUrls());
    QVERIFY(mimeData->hasText());
    QVERIFY(mimeData->hasFormat(QLatin1String("text/plain")));

    // KUrl decodes the real "kde" urls by default
    KUrl::List decodedURLs = KUrl::List::fromMimeData(mimeData);
    QVERIFY(!decodedURLs.isEmpty());
    const QString space(QLatin1Char(' '));
    QCOMPARE(decodedURLs.toStringList().join(space), urls.toStringList().join(space) );

    // KUrl can also be told to decode the "most local" urls
    decodedURLs = KUrl::List::fromMimeData(mimeData, KUrl::List::PreferLocalUrls);
    QVERIFY(!decodedURLs.isEmpty());
    QCOMPARE(decodedURLs.toStringList().join(space), localUrls.toStringList().join(space) );

    // QMimeData decodes the "most local" urls
    const QList<QUrl> qurls = mimeData->urls();
    QCOMPARE(qurls.count(), localUrls.count());
    for (int i = 0; i < qurls.count(); ++i )
        QCOMPARE(qurls[i], static_cast<QUrl>(localUrls[i]));

}
