/* This file is part of the Nepomuk-KDE libraries
    Copyright (c) 2007 Sebastian Trueg <trueg@kde.org>

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

#include "entitytest.h"
#include "../class.h"
#include "../property.h"

#include <kdebug.h>
#include <kstandarddirs.h>
#include <qtest_kde.h>
#include <QtDBus/QtDBus>

#include <Soprano/Vocabulary/RDFS>

using namespace Nepomuk::Types;

static QByteArray origKdeHome;

// Getting the real KDEHOME has to be done before main() overwrites $KDEHOME
int kInitSocket()
{
    origKdeHome = getenv("KDEHOME");
    return 1;
}
Q_CONSTRUCTOR_FUNCTION(kInitSocket)

// Make symlink from ~/.kde-unit-test/share/apps/nepomuk/socket to the real socket for the running server
void EntityTest::initTestCase()
{
    QString realSocket = !origKdeHome.isEmpty() ? QFile::encodeName(origKdeHome) : QDir::homePath() + QLatin1String("/.kde");
    realSocket += QLatin1String("/share/apps/nepomuk/socket");
    //kDebug() << realSocket;
    if (QFile::exists(realSocket)) {
        const QString socketLink = KGlobal::dirs()->locateLocal( "data", "nepomuk/socket" );
        //kDebug() << socketLink << realSocket;
        if (!QFile::exists(socketLink)) {
            ::symlink(QFile::encodeName(realSocket), QFile::encodeName(socketLink));
        }
    }
}

void EntityTest::testClass()
{
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.nepomuk.services.nepomukstorage")) {
        QSKIP("nepomuk not running", SkipAll);
    }

    Class c1( Soprano::Vocabulary::RDFS::Resource() );
    Class c2( Soprano::Vocabulary::RDFS::Resource() );

    QVERIFY( c1.isValid() );
    QVERIFY( c2.isValid() );

    QVERIFY( c1.isAvailable() );
    QVERIFY( c2.isAvailable() );

    QCOMPARE( c1.uri(), c2.uri() );
    QCOMPARE( c1.name(), c2.name() );
    QCOMPARE( c1.label(), c2.label() );
    QCOMPARE( c1.comment(), c2.comment() );

    QVERIFY( !c1.label().isEmpty() );

    qDebug() << c1.label() << c1.comment();
}


void EntityTest::testProperty()
{
}


QTEST_KDEMAIN(EntityTest, NoGUI)

#include "entitytest.moc"
