/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

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

#include <qtest_kde.h>
#include "kconfigtest.h"
#include <kstandarddirs.h>
#include "kconfigtest.moc"

#include <kconfig.h>
#include <kdebug.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>

KCONFIGGROUP_DECLARE_ENUM_QOBJECT(KConfigTest,Testing)
KCONFIGGROUP_DECLARE_FLAGS_QOBJECT(KConfigTest,Flags)

QTEST_KDEMAIN_CORE( KConfigTest )

#define BOOLENTRY1 true
#define BOOLENTRY2 false
#define STRINGENTRY1 "hello"
#define STRINGENTRY2 " hello"
#define STRINGENTRY3 "hello "
#define STRINGENTRY4 " hello "
#define STRINGENTRY5 " "
#define STRINGENTRY6 ""
#define UTF8BITENTRY "Hello äöü"
#define BYTEARRAYENTRY QByteArray( "\x00\xff\x7f\x3c abc\x00\x00", 10 )
#define ESCAPEKEY " []\0017[]==]"
#define ESCAPEENTRY "[]\170[]]=3=]\\] "
#define DOUBLEENTRY 123456.78912345
#define POINTENTRY QPoint( 4351, 1235 )
#define SIZEENTRY QSize( 10, 20 )
#define RECTENTRY QRect( 10, 23, 5321, 13 )
#define DATETIMEENTRY QDateTime( QDate( 2002, 06, 23 ), QTime( 12, 55, 40 ) )
#define STRINGLISTENTRY (QStringList( "Hello," ) << " World")
#define STRINGLISTEMPTYENTRY (QStringList())
#define STRINGLISTESCAPEODDENTRY (QStringList( "Hello\\\\\\" ) << "World")
#define STRINGLISTESCAPEEVENENTRY (QStringList( "Hello\\\\\\\\" ) << "World")
#define STRINGLISTESCAPECOMMAENTRY (QStringList( "Hel\\\\\\,\\\\,\\,\\\\\\\\,lo" ) << "World")
#define INTLISTENTRY1 QList<int>() << 1 << 2 << 3 << 4
#define BYTEARRAYLISTENTRY1 QList<QByteArray>() << "" << "1,2" << "end"
#define VARIANTLISTENTRY (QVariantList() << true << false << QString("joe") << 10023)
#define VARIANTLISTENTRY2 (QVariantList() << POINTENTRY << SIZEENTRY)
#define HOMEPATH QDir::homePath()+"/foo"

void KConfigTest::initTestCase()
{
  // to make sure all files from a previous failed run are deleted
  cleanupTestCase();

  KConfig sc( "kconfigtest" );

  KConfigGroup cg(&sc, "AAA");
  cg.writeEntry("stringEntry1", STRINGENTRY1,
                KConfig::Persistent|KConfig::Global);
  cg.deleteEntry("stringEntry2", KConfig::Global);

  cg = KConfigGroup(&sc, "Hello");
  cg.writeEntry( "boolEntry1", BOOLENTRY1 );
  cg.writeEntry( "boolEntry2", BOOLENTRY2 );

  QByteArray data( UTF8BITENTRY );
  QCOMPARE( data.size(), 12 ); // the source file is in utf8
  QCOMPARE( QString::fromUtf8(data).length(), 9 );
  cg.writeEntry ("Test", data);
  cg.writeEntry( "bytearrayEntry", BYTEARRAYENTRY );
  cg.writeEntry( ESCAPEKEY, ESCAPEENTRY );
  cg.writeEntry( "emptyEntry", "");
  cg.writeEntry( "stringEntry1", STRINGENTRY1 );
  cg.writeEntry( "stringEntry2", STRINGENTRY2 );
  cg.writeEntry( "stringEntry3", STRINGENTRY3 );
  cg.writeEntry( "stringEntry4", STRINGENTRY4 );
  cg.writeEntry( "stringEntry5", STRINGENTRY5 );
  cg.writeEntry( "keywith=equalsign", STRINGENTRY1 );
  cg.deleteEntry( "stringEntry5" );
  cg.deleteEntry( "stringEntry6" ); // deleting a nonexistant entry
  cg.writeEntry( "byteArrayEntry1", QByteArray( STRINGENTRY1 ),
                 KConfig::Global|KConfig::Persistent );
  cg.writeEntry( "doubleEntry1", DOUBLEENTRY );

  sc.deleteGroup("deleteMe"); // deleting a nonexistant group

  cg = KConfigGroup(&sc, "Complex Types");
  cg.writeEntry( "rectEntry", RECTENTRY );
  cg.writeEntry( "pointEntry", POINTENTRY );
  cg.writeEntry( "sizeEntry", SIZEENTRY );
  cg.writeEntry( "dateTimeEntry", DATETIMEENTRY );
  cg.writeEntry( "dateEntry", DATETIMEENTRY.date() );

  cg = KConfigGroup(&sc, "List Types" );
  cg.writeEntry( "listOfIntsEntry1", INTLISTENTRY1 );
  cg.writeEntry( "listOfByteArraysEntry1", BYTEARRAYLISTENTRY1 );
  cg.writeEntry( "stringListEntry", STRINGLISTENTRY );
  cg.writeEntry( "stringListEmptyEntry", STRINGLISTEMPTYENTRY );
  cg.writeEntry( "stringListEscapeOddEntry", STRINGLISTESCAPEODDENTRY );
  cg.writeEntry( "stringListEscapeEvenEntry", STRINGLISTESCAPEEVENENTRY );
  cg.writeEntry( "stringListEscapeCommaEntry", STRINGLISTESCAPECOMMAENTRY );
  cg.writeEntry( "variantListEntry", VARIANTLISTENTRY );

  cg = KConfigGroup(&sc, "Path Type" );
  cg.writePathEntry( "homepath", HOMEPATH );

  cg = KConfigGroup(&sc, "Enum Types" );
  cg.writeEntry( "enum-10", Tens );

#ifndef Q_CC_MSVC
  cg.writeEntry( "enum-100", Hundreds );
#else
  cg.writeEntry( "enum-100", Hundreds, KConfig::Normal );
#endif

  cg.writeEntry( "flags-bit0", Flags(bit0));

#ifndef Q_CC_MSVC
  cg.writeEntry( "flags-bit0-bit1", bit0|bit1);
#else
  cg.writeEntry( "flags-bit0-bit1", Flags(bit0|bit1), KConfig::Normal );
#endif

  cg = KConfigGroup(&sc, "ParentGroup" );
  KConfigGroup cg1(&cg, "SubGroup1" );
  cg1.writeEntry( "somestring", "somevalue" );
  cg.writeEntry( "parentgrpstring", "somevalue" );
  KConfigGroup cg2(&cg, "SubGroup2" );
  cg2.writeEntry( "substring", "somevalue" );
  KConfigGroup cg3(&cg, "SubGroup/3");
  cg3.writeEntry( "sub3string", "somevalue" );

  sc.sync();

  KConfig sc1("kdebugrc", KConfig::SimpleConfig);
  KConfigGroup sg0(&sc1, "0");
  sg0.writeEntry("AbortFatal", false);
  sg0.writeEntry("WarnOutput", 0);
  sg0.writeEntry("FatalOutput", 0);
  sc1.sync();
}

void KConfigTest::cleanupTestCase()
{
  QDir local = QDir::homePath() + "/.kde-unit-test/share/config";

  foreach(const QString &file, local.entryList(QDir::Files))
    if(!local.remove(file))
      qWarning("%s: removing failed", qPrintable( file ));

  QCOMPARE((int)local.entryList(QDir::Files).count(), 0);

  local.cdUp();
  local.rmpath("config");
}

// ### TODO: call this, and test the state of things afterwards
void KConfigTest::revertEntries()
{
//  qDebug("Reverting entries");
  KConfig sc( "kconfigtest" );

  KConfigGroup cg(&sc, "Hello");
  cg.revertToDefault( "boolEntry1" );
  cg.revertToDefault( "boolEntry2" );

  cg.revertToDefault( "Test" );
  cg.revertToDefault( "emptyEntry" );
  cg.revertToDefault( "stringEntry1" );
  cg.revertToDefault( "stringEntry2" );
  cg.revertToDefault( "stringEntry3" );
  cg.revertToDefault( "stringEntry4" );
  cg.revertToDefault( "stringEntry5" );
  sc.sync();
}

void KConfigTest::testRevertAllEntries()
{
    // this tests the case were we revert (delete) all entries in a file,
    // leaving a blank file
    {
        KConfig sc( "konfigtest2" );
        KConfigGroup cg( &sc, "Hello" );
        cg.writeEntry( "Test", "Correct" );
    }

    {
        KConfig sc( "konfigtest2" );
        KConfigGroup cg( &sc, "Hello" );
        QCOMPARE( cg.readEntry( "Test", "Default" ), QString("Correct") );
        cg.revertToDefault( "Test" );
    }

    KConfig sc( "konfigtest2" );
    KConfigGroup cg( &sc, "Hello" );
    QCOMPARE( cg.readEntry( "Test", "Default" ), QString("Default") );
}

void KConfigTest::testSimple()
{
  KConfig sc2( "kconfigtest" );
  QCOMPARE(sc2.name(), QString("kconfigtest"));

  KConfigGroup sc3( &sc2, "AAA");

  QVERIFY( sc3.hasKey( "stringEntry1" ) );
  QVERIFY( !sc3.entryIsImmutable("stringEntry1") );
  QCOMPARE( sc3.readEntry( "stringEntry1" ), QString( STRINGENTRY1 ) );

  QVERIFY( !sc3.hasKey( "stringEntry2" ) );
  QCOMPARE( sc3.readEntry( "stringEntry2", QString("bla") ), QString( "bla" ) );

  QVERIFY( !sc3.hasDefault( "stringEntry1" ) );

  sc3 = KConfigGroup(&sc2, "Hello");
  QCOMPARE( sc3.readEntry( "Test", QByteArray() ), QByteArray( UTF8BITENTRY ) );
  QCOMPARE( sc3.readEntry( "bytearrayEntry", QByteArray() ), BYTEARRAYENTRY );
  QCOMPARE( sc3.readEntry( ESCAPEKEY ), QString( ESCAPEENTRY ) );
  QCOMPARE( sc3.readEntry( "Test", QString() ), QString::fromUtf8( UTF8BITENTRY ) );
  QCOMPARE( sc3.readEntry( "emptyEntry"/*, QString("Fietsbel")*/), QString("") );
  QCOMPARE( sc3.readEntry("emptyEntry", QString("Fietsbel")).isEmpty(), true );
  QCOMPARE( sc3.readEntry( "stringEntry1" ), QString( STRINGENTRY1 ) );
  QCOMPARE( sc3.readEntry( "stringEntry2" ), QString( STRINGENTRY2 ) );
  QCOMPARE( sc3.readEntry( "stringEntry3" ), QString( STRINGENTRY3 ) );
  QCOMPARE( sc3.readEntry( "stringEntry4" ), QString( STRINGENTRY4 ) );
  QVERIFY( !sc3.hasKey( "stringEntry5" ) );
  QCOMPARE( sc3.readEntry( "stringEntry5", QString("test") ), QString( "test" ) );
  QVERIFY( !sc3.hasKey( "stringEntry6" ) );
  QCOMPARE( sc3.readEntry( "stringEntry6", QString("foo") ), QString( "foo" ) );
  QCOMPARE( sc3.readEntry( "boolEntry1", BOOLENTRY1 ), BOOLENTRY1 );
  QCOMPARE( sc3.readEntry( "boolEntry2", false ), BOOLENTRY2 );
  QCOMPARE( sc3.readEntry("keywith=equalsign", QString("wrong")), QString(STRINGENTRY1));
  QCOMPARE( sc3.readEntry( "byteArrayEntry1", QByteArray() ),
            QByteArray( STRINGENTRY1 ) );
  QCOMPARE( sc3.readEntry( "doubleEntry1", 0.0 ), DOUBLEENTRY );
}

void KConfigTest::testLists()
{
  KConfig sc2( "kconfigtest" );
  KConfigGroup sc3(&sc2, "List Types");

  QCOMPARE( sc3.readEntry( QString("stringListEntry"), QStringList()),
            STRINGLISTENTRY );

  QVERIFY( sc3.hasKey( QString("stringListEmptyEntry") ) );

  QCOMPARE( sc3.readEntry( QString("stringListEscapeOddEntry"), QStringList()),
            STRINGLISTESCAPEODDENTRY );

  QCOMPARE( sc3.readEntry( QString("stringListEscapeEvenEntry"), QStringList()),
            STRINGLISTESCAPEEVENENTRY );

  QCOMPARE( sc3.readEntry( QString("stringListEscapeCommaEntry"), QStringList()),
            STRINGLISTESCAPECOMMAENTRY );

  QCOMPARE( sc3.readEntry( "listOfIntsEntry1" ), QString::fromLatin1( "1,2,3,4" ) );
  QList<int> expectedIntList = INTLISTENTRY1;
  QVERIFY( sc3.readEntry( "listOfIntsEntry1", QList<int>() ) == expectedIntList );

  QCOMPARE( QVariant(sc3.readEntry( "variantListEntry", VARIANTLISTENTRY )).toStringList(),
            QVariant(VARIANTLISTENTRY).toStringList() );

  QCOMPARE( sc3.readEntry( "listOfByteArraysEntry1", QList<QByteArray>()), BYTEARRAYLISTENTRY1 );
}

void KConfigTest::testPath()
{
  KConfig sc2( "kconfigtest" );
  KConfigGroup sc3(&sc2, "Path Type");
  QString p = sc3.readPathEntry("homepath");
  QCOMPARE( sc3.readPathEntry( "homepath", QString() ), HOMEPATH );
}

void KConfigTest::testComplex()
{
  KConfig sc2( "kconfigtest" );
  KConfigGroup sc3(&sc2, "Complex Types");

  QCOMPARE( sc3.readEntry( "pointEntry", QPoint() ), POINTENTRY );
  QCOMPARE( sc3.readEntry( "sizeEntry", SIZEENTRY ), SIZEENTRY);
  QCOMPARE( sc3.readEntry( "rectEntry", QRect(1,2,3,4) ), RECTENTRY );
  QCOMPARE( sc3.readEntry( "dateTimeEntry", QDateTime() ).toString(Qt::ISODate),
            DATETIMEENTRY.toString(Qt::ISODate) );
  QCOMPARE( sc3.readEntry( "dateEntry", QDate() ).toString(Qt::ISODate),
            DATETIMEENTRY.date().toString(Qt::ISODate) );
  QCOMPARE( sc3.readEntry( "dateTimeEntry", QDate() ), DATETIMEENTRY.date() );
}

void KConfigTest::testEnums()
{
  KConfig sc("kconfigtest");
  KConfigGroup sc3(&sc, "Enum Types" );

  QCOMPARE( sc3.readEntry( "enum-10" ), QString("Tens"));
  QVERIFY( sc3.readEntry( "enum-100", Ones) != Ones);
  QVERIFY( sc3.readEntry( "enum-100", Ones) != Tens);

  QCOMPARE( sc3.readEntry( "flags-bit0" ), QString("bit0"));
  QVERIFY( sc3.readEntry( "flags-bit0", Flags() ) == bit0 );

  int eid = staticMetaObject.indexOfEnumerator( "Flags" );
  QVERIFY( eid != -1 );
  QMetaEnum me = staticMetaObject.enumerator( eid );
  Flags bitfield = bit0|bit1;

  QCOMPARE( sc3.readEntry( "flags-bit0-bit1" ), QString( me.valueToKeys(bitfield) ) );
  QVERIFY( sc3.readEntry( "flags-bit0-bit1", Flags() ) == bitfield );
}

void KConfigTest::testInvalid()
{
  KConfig sc( "kconfigtest" );

  // all of these should print a message to the kdebug.dbg file
  KConfigGroup sc3(&sc, "Invalid Types" );
  sc3.writeEntry( "badList", VARIANTLISTENTRY2 );

  QList<int> list;

  // 1 element list
  list << 1;
  sc3.writeEntry( QString("badList"), list);

  QVERIFY( sc3.readEntry( "badList", QPoint() ) == QPoint() );
  QVERIFY( sc3.readEntry( "badList", QRect() ) == QRect() );
  QVERIFY( sc3.readEntry( "badList", QSize() ) == QSize() );
  QVERIFY( sc3.readEntry( "badList", QDate() ) == QDate() );
  QVERIFY( sc3.readEntry( "badList", QDateTime() ) == QDateTime() );

  // 2 element list
  list << 2;
  sc3.writeEntry( "badList", list);

  QVERIFY( sc3.readEntry( "badList", QRect() ) == QRect() );
  QVERIFY( sc3.readEntry( "badList", QDate() ) == QDate() );
  QVERIFY( sc3.readEntry( "badList", QDateTime() ) == QDateTime() );

  // 3 element list
  list << 303;
  sc3.writeEntry( "badList", list);

  QVERIFY( sc3.readEntry( "badList", QPoint() ) == QPoint() );
  QVERIFY( sc3.readEntry( "badList", QRect() ) == QRect() );
  QVERIFY( sc3.readEntry( "badList", QSize() ) == QSize() );
  QVERIFY( sc3.readEntry( "badList", QDate() ) == QDate() ); // out of bounds
  QVERIFY( sc3.readEntry( "badList", QDateTime() ) == QDateTime() );

  // 4 element list
  list << 4;
  sc3.writeEntry( "badList", list );

  QVERIFY( sc3.readEntry( "badList", QPoint() ) == QPoint() );
  QVERIFY( sc3.readEntry( "badList", QSize() ) == QSize() );
  QVERIFY( sc3.readEntry( "badList", QDate() ) == QDate() );
  QVERIFY( sc3.readEntry( "badList", QDateTime() ) == QDateTime() );

  // 5 element list
  list[2] = 3;
  list << 5;
  sc3.writeEntry( "badList", list);

  QVERIFY( sc3.readEntry( "badList", QPoint() ) == QPoint() );
  QVERIFY( sc3.readEntry( "badList", QRect() ) == QRect() );
  QVERIFY( sc3.readEntry( "badList", QSize() ) == QSize() );
  QVERIFY( sc3.readEntry( "badList", QDate() ) == QDate() );
  QVERIFY( sc3.readEntry( "badList", QDateTime() ) == QDateTime() );

  // 6 element list
  list << 6;
  sc3.writeEntry( "badList", list);

  QVERIFY( sc3.readEntry( "badList", QPoint() ) == QPoint() );
  QVERIFY( sc3.readEntry( "badList", QRect() ) == QRect() );
  QVERIFY( sc3.readEntry( "badList", QSize() ) == QSize() );
}

void KConfigTest::testChangeGroup()
{
    KConfig sc( "kconfigtest" );
    KConfigGroup sc3(&sc, "Hello");
    QCOMPARE(sc3.name(), QString("Hello"));
    KConfigGroup newGroup(sc3);
    newGroup.changeGroup("FooBar");
    QCOMPARE(newGroup.name(), QString("FooBar"));
    QCOMPARE(sc3.name(), QString("Hello")); // unchanged
}

void KConfigTest::testDelete()
{
  KConfig sc( "kconfigtest" );
  KConfigGroup sc3(&sc, "Hello");

  sc3.deleteEntry("Test");
  QCOMPARE( sc3.readEntry("Test", QString("Fietsbel")), QString("Fietsbel") );

  sc.deleteGroup("Complex Types");

  KConfigGroup cg(&sc , "AAA" );
  cg.deleteGroup();
  QVERIFY( sc.entryMap("Complex Types").isEmpty() );
  QVERIFY( sc.entryMap("AAA").isEmpty() );
  QVERIFY( !sc.entryMap("Hello").isEmpty() ); //not deleted group
  QVERIFY( sc.entryMap("FooBar").isEmpty() ); //inexistant group
}

void KConfigTest::testDefaultGroup()
{
    KConfig sc( "kconfigtest" );
    KConfigGroup defaultGroup(&sc, "<default>");
    QCOMPARE(defaultGroup.name(), QString("<default>"));
    QVERIFY(!defaultGroup.exists());
    defaultGroup.writeEntry("TestKey", "defaultGroup");
    QVERIFY(defaultGroup.exists());
    QCOMPARE(defaultGroup.readEntry("TestKey", QString()), QString("defaultGroup"));
    sc.sync();

    {
        // Test reading it
        KConfig sc2("kconfigtest");
        KConfigGroup defaultGroup2(&sc2, "<default>");
        QCOMPARE(defaultGroup2.name(), QString("<default>"));
        QVERIFY(defaultGroup2.exists());
        QCOMPARE(defaultGroup2.readEntry("TestKey", QString()), QString("defaultGroup"));
    }
    {
        // Test reading it
        KConfig sc2("kconfigtest");
        KConfigGroup emptyGroup(&sc2, "");
        QCOMPARE(emptyGroup.name(), QString("<default>"));
        QVERIFY(emptyGroup.exists());
        QCOMPARE(emptyGroup.readEntry("TestKey", QString()), QString("defaultGroup"));
    }

#ifdef Q_OS_UNIX
    QList<QByteArray> lines = readLines();
    QVERIFY(!lines.contains("[]"));
    QCOMPARE(lines.first(), QByteArray("TestKey=defaultGroup\n"));
#endif

    defaultGroup.deleteGroup();
    sc.sync();

    // Test if deleteGroup worked
#ifdef Q_OS_UNIX
    lines = readLines();
    QVERIFY(lines.first() != QByteArray("TestKey=defaultGroup\n"));
#endif
}

void KConfigTest::testEmptyGroup()
{
    KConfig sc( "kconfigtest" );
    KConfigGroup emptyGroup(&sc, "");
    QCOMPARE(emptyGroup.name(), QString("<default>")); // confusing, heh?
    QVERIFY(!emptyGroup.exists());
    emptyGroup.writeEntry("TestKey", "emptyGroup");
    QVERIFY(emptyGroup.exists());
    QCOMPARE(emptyGroup.readEntry("TestKey", QString()), QString("emptyGroup"));
    sc.sync();

    {
        // Test reading it
        KConfig sc2("kconfigtest");
        KConfigGroup defaultGroup(&sc2, "<default>");
        QCOMPARE(defaultGroup.name(), QString("<default>"));
        QVERIFY(defaultGroup.exists());
        QCOMPARE(defaultGroup.readEntry("TestKey", QString()), QString("emptyGroup"));
    }
    {
        // Test reading it
        KConfig sc2("kconfigtest");
        KConfigGroup emptyGroup2(&sc2, "");
        QCOMPARE(emptyGroup2.name(), QString("<default>"));
        QVERIFY(emptyGroup2.exists());
        QCOMPARE(emptyGroup2.readEntry("TestKey", QString()), QString("emptyGroup"));
    }

#ifdef Q_OS_UNIX
    QList<QByteArray> lines = readLines();
    QVERIFY(!lines.contains("[]")); // there's no support for the [] group, in fact.
    QCOMPARE(lines.first(), QByteArray("TestKey=emptyGroup\n"));
#endif

    emptyGroup.deleteGroup();
    sc.sync();

    // Test if deleteGroup worked
#ifdef Q_OS_UNIX
    lines = readLines();
    QVERIFY(lines.first() != QByteArray("TestKey=defaultGroup\n"));
#endif
}

void KConfigTest::testSubGroup()
{
    KConfig sc( "kconfigtest" );
    KConfigGroup cg( &sc, "ParentGroup" );
    QVERIFY(cg.readEntry( "parentgrpstring", "") == QString("somevalue") );
    KConfigGroup subcg1( &cg, "SubGroup1");
    QVERIFY(subcg1.readEntry( "somestring", "") == QString("somevalue") );
    KConfigGroup subcg2( &cg, "SubGroup2");
    QVERIFY(subcg2.readEntry( "substring", "") == QString("somevalue") );
    KConfigGroup subcg3( &cg, "SubGroup/3");
    QVERIFY(subcg3.readEntry( "sub3string", "") == QString("somevalue") );
    KConfigGroup subcg4( &subcg3, "3");
    QVERIFY(!subcg4.exists());
}

void KConfigTest::testKdeglobals()
{
    const QString kdeglobals = KStandardDirs::locateLocal("config", "kdeglobals");
    KConfig sc("kdeglobals");
    QVERIFY(sc.forceGlobal());
    QVERIFY(sc.name() == QLatin1String("kdeglobals"));

    KConfig sc2(QString(), KConfig::IncludeGlobals);
    QVERIFY(sc2.forceGlobal());
    QVERIFY(sc2.name() == QLatin1String("kdeglobals"));

    KConfig sc3("kdeglobals", KConfig::SimpleConfig);
    QVERIFY(sc3.forceGlobal());
    QVERIFY(sc3.name() == QLatin1String("kdeglobals"));
}

void KConfigTest::testKAboutDataOrganizationDomain()
{
    KAboutData data( "app", 0, ki18n("program"), "version",
                     ki18n("description"), KAboutData::License_LGPL,
                     ki18n("copyright"), ki18n("hello world"),
                     "http://www.koffice.org" );
    QCOMPARE( data.organizationDomain(), QString::fromLatin1( "koffice.org" ) );

    KAboutData data2( "app", 0, ki18n("program"), "version",
                      ki18n("description"), KAboutData::License_LGPL,
                      ki18n("copyright"), ki18n("hello world"),
                      "http://edu.kde.org/kig" );
    QCOMPARE( data2.organizationDomain(), QString::fromLatin1( "kde.org" ) );
}

QList<QByteArray> KConfigTest::readLines()
{
    const QString path = KStandardDirs::locateLocal("config", "kconfigtest");
    Q_ASSERT(!path.isEmpty());
    QFile file(path);
    Q_ASSERT(file.open(QIODevice::ReadOnly));
    QList<QByteArray> lines;
    QByteArray line;
    do {
        line = file.readLine();
        if (!line.isEmpty())
            lines.append(line);
    } while(!line.isEmpty());
    return lines;
}
