/* This file is part of the KDE libraries
 *  Copyright (c) 2012 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License or ( at
 *  your option ) version 3 or, at the discretion of KDE e.V. ( which shall
 *  act as a proxy as in section 14 of the GPLv3 ), any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
*/

#include <kshareddatacache.h>

#include <QtTest/QtTest>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDir>
#include <qstandardpaths.h>
#include <string.h> // strcpy

class KSharedDataCacheTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void simpleInsert();
};

void KSharedDataCacheTest::initTestCase()
{
}

void KSharedDataCacheTest::simpleInsert()
{
    const QLatin1String cacheName("myTestCache");
    const QLatin1String key("mypic");
    // clear the cache
    QString cacheFile = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1String("/") + cacheName + QLatin1String(".kcache");
    QFile file(cacheFile);
    if (file.exists())
        QVERIFY(file.remove());
    // insert something into it
    KSharedDataCache cache(cacheName, 5*1024*1024);
 #ifndef Q_OS_WIN // the windows implementation is currently only memory based and not really shared
    QVERIFY(file.exists()); // make sure we got the cache filename right
 #endif
    QByteArray data;
    data.resize(9228);
    strcpy(data.data(), "Hello world");
    QVERIFY(cache.insert(key, data));
    // read it out again
    QByteArray result;
    QVERIFY(cache.find(key, &result));
    QCOMPARE(result, data);
    // another insert
    strcpy(data.data(), "Hello KDE");
    QVERIFY(cache.insert(key, data));
    // and another read
    QVERIFY(cache.find(key, &result));
    QCOMPARE(result, data);
}

QTEST_MAIN(KSharedDataCacheTest)

#include "kshareddatacachetest.moc"
