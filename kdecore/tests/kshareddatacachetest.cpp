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

#include <qtest_kde.h>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDir>
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
    KSharedDataCache cache("myCache", 5*1024*1024);
    QByteArray data;
    data.resize(9228);
    strcpy(data.data(), "Hello world");
    const QString key = "mypic";
    QVERIFY(cache.insert(key, data));

    QByteArray result;
    QVERIFY(cache.find(key, &result));
#if 0
    QEXPECT_FAIL("", "Bug in findNamedEntry!", Continue);
    QCOMPARE(result, data);
#endif
}

QTEST_KDEMAIN_CORE(KSharedDataCacheTest)

#include "kshareddatacachetest.moc"
