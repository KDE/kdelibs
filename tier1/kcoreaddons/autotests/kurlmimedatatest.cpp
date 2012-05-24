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

#include "kurlmimedatatest.h"
#include <QtTest>
#include <QMimeData>
#include <kurlmimedata.h>

QTEST_MAIN( KUrlMimeDataTest )

void KUrlMimeDataTest::testURLList()
{
    QMimeData* mimeData = new QMimeData();
    QVERIFY(!mimeData->hasUrls());

    QList<QUrl> urls;
    urls.append( QUrl( QLatin1String( "http://www.kde.org" ) ) );
    urls.append( QUrl( QLatin1String( "http://wstephenson:secret@example.com/path" ) ) );
    urls.append( QUrl( QLatin1String( "file:///home/dfaure/konqtests/Mat%C3%A9riel" ) ) );
    QMap<QString, QString> metaData;
    metaData[QLatin1String("key")] = QLatin1String("value");
    metaData[QLatin1String("key2")] = QLatin1String("value2");

    KUrlMimeData::setUrls(QList<QUrl>(), urls, mimeData);
    KUrlMimeData::setMetaData(metaData, mimeData);

    QVERIFY(mimeData->hasUrls());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QVERIFY(mimeData->hasText());
#endif

    QMap<QString, QString> decodedMetaData;
    QList<QUrl> decodedURLs = KUrlMimeData::urlsFromMimeData(mimeData, KUrlMimeData::PreferKdeUrls, &decodedMetaData);
    QVERIFY( !decodedURLs.isEmpty() );
    QList<QUrl> expectedUrls = urls;
    expectedUrls[1] = QUrl(QLatin1String("http://wstephenson:secret@example.com/path")); // password kept, unlike in KDE4, but that's okay, it's not displayed
    QCOMPARE( expectedUrls, decodedURLs );

    const QList<QUrl> qurls = mimeData->urls();
    QCOMPARE(qurls.count(), urls.count());
    for (int i = 0; i < qurls.count(); ++i )
        QCOMPARE(qurls[i], decodedURLs[i]);

    QVERIFY( !decodedMetaData.isEmpty() );
    QCOMPARE( decodedMetaData[QLatin1String("key")], QString::fromLatin1( "value" ) );
    QCOMPARE( decodedMetaData[QLatin1String("key2")], QString::fromLatin1( "value2" ) );

    delete mimeData;
}

void KUrlMimeDataTest::testOneURL()
{
    QUrl oneURL( QLatin1String("file:///tmp") );
    QList<QUrl> oneEltList;
    oneEltList.append(oneURL);
    QMimeData* mimeData = new QMimeData();

    KUrlMimeData::setUrls(QList<QUrl>(), oneEltList, mimeData);
    QVERIFY(mimeData->hasUrls());

    QMap<QString, QString> decodedMetaData;
    QList<QUrl> decodedURLs = KUrlMimeData::urlsFromMimeData(mimeData, KUrlMimeData::PreferKdeUrls, &decodedMetaData);
    QVERIFY( !decodedURLs.isEmpty() );
    QCOMPARE( decodedURLs.count(), 1 );
    QCOMPARE( decodedURLs[0], oneURL );
    QVERIFY( decodedMetaData.isEmpty() );
    delete mimeData;
}

void KUrlMimeDataTest::testFromQUrl()
{
    QList<QUrl> qurls;
    qurls.append( QUrl( QLatin1String("http://www.kde.org") ) );
    qurls.append( QUrl( QLatin1String("file:///home/dfaure/konqtests/Mat%C3%A9riel") ) );
    QMimeData* mimeData = new QMimeData();
    KUrlMimeData::setUrls(QList<QUrl>(), qurls, mimeData);
    QVERIFY(mimeData->hasUrls());

    QMap<QString, QString> decodedMetaData;
    QList<QUrl> decodedURLs = KUrlMimeData::urlsFromMimeData(mimeData, KUrlMimeData::PreferKdeUrls, &decodedMetaData);
    QVERIFY( !decodedURLs.isEmpty() );
    QCOMPARE( decodedURLs.count(), 2 );
    QCOMPARE( decodedURLs[0], qurls[0] );
    QCOMPARE( decodedURLs[1], qurls[1] );
    QVERIFY( decodedMetaData.isEmpty() );
    delete mimeData;
}

void KUrlMimeDataTest::testMostLocalUrlList()
{
    QMimeData* mimeData = new QMimeData();
    QList<QUrl> urls;
    urls.append(QUrl(QLatin1String("desktop:/foo")));
    urls.append(QUrl(QLatin1String("desktop:/bar")));
    QList<QUrl> localUrls;
    localUrls.append(QUrl(QLatin1String("file:/home/dfaure/Desktop/foo")));
    localUrls.append(QUrl(QLatin1String("file:/home/dfaure/Desktop/bar")));

    KUrlMimeData::setUrls(urls, localUrls, mimeData);

    QVERIFY(mimeData->hasUrls());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QVERIFY(mimeData->hasText());
    // The support for urls is done in hasText, a direct call to hasFormat will say false.
    //QVERIFY(mimeData->hasFormat(QLatin1String("text/plain")));
#endif

    // urlsFromMimeData decodes the real "kde" urls by default
    QList<QUrl> decodedURLs = KUrlMimeData::urlsFromMimeData(mimeData);
    QVERIFY(!decodedURLs.isEmpty());
    QCOMPARE(decodedURLs, urls );

    // urlsFromMimeData can also be told to decode the "most local" urls
    decodedURLs = KUrlMimeData::urlsFromMimeData(mimeData, KUrlMimeData::PreferLocalUrls);
    QVERIFY(!decodedURLs.isEmpty());
    QCOMPARE(decodedURLs, localUrls );

    // QMimeData decodes the "most local" urls
    const QList<QUrl> qurls = mimeData->urls();
    QCOMPARE(qurls.count(), localUrls.count());
    for (int i = 0; i < qurls.count(); ++i )
        QCOMPARE(qurls[i], static_cast<QUrl>(localUrls[i]));

}
