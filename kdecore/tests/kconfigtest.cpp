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
#include <kdesktopfile.h>
#include <kstandarddirs.h>
#include "kconfigtest.moc"

#include <kconfig.h>
#include <kdebug.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>

#include <QtNetwork/QHostInfo>

#ifdef Q_OS_UNIX
#include <utime.h>
#endif

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
#define TRANSLATEDSTRINGENTRY1 "bonjour"
#define BYTEARRAYENTRY QByteArray( "\x00\xff\x7f\x3c abc\x00\x00", 10 )
#define ESCAPEKEY " []\0017[]==]"
#define ESCAPEENTRY "[]\170[]]=3=]\\] "
#define DOUBLEENTRY 123456.78912345
#define FLOATENTRY 123.567f
#define POINTENTRY QPoint( 4351, 1235 )
#define SIZEENTRY QSize( 10, 20 )
#define RECTENTRY QRect( 10, 23, 5321, 13 )
#define DATETIMEENTRY QDateTime( QDate( 2002, 06, 23 ), QTime( 12, 55, 40 ) )
#define STRINGLISTENTRY (QStringList( "Hello," ) << " World")
#define STRINGLISTEMPTYENTRY QStringList()
#define STRINGLISTJUSTEMPTYELEMENT QStringList(QString())
#define STRINGLISTEMPTYTRAINLINGELEMENT (QStringList( "Hi" ) << QString())
#define STRINGLISTESCAPEODDENTRY (QStringList( "Hello\\\\\\" ) << "World")
#define STRINGLISTESCAPEEVENENTRY (QStringList( "Hello\\\\\\\\" ) << "World")
#define STRINGLISTESCAPECOMMAENTRY (QStringList( "Hel\\\\\\,\\\\,\\,\\\\\\\\,lo" ) << "World")
#define INTLISTENTRY1 QList<int>() << 1 << 2 << 3 << 4
#define BYTEARRAYLISTENTRY1 QList<QByteArray>() << "" << "1,2" << "end"
#define VARIANTLISTENTRY (QVariantList() << true << false << QString("joe") << 10023)
#define VARIANTLISTENTRY2 (QVariantList() << POINTENTRY << SIZEENTRY)
#define HOMEPATH QDir::homePath()+"/foo"
#define HOMEPATHESCAPE QDir::homePath()+"/foo/$HOME"
#define SUBGROUPLIST (QStringList() << "SubGroup/3" << "SubGroup1" << "SubGroup2")
#define PARENTGROUPKEYS (QStringList() << "parentgrpstring")
#define SUBGROUP3KEYS (QStringList() << "sub3string")
#define DOLLARGROUP "$i"

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
  cg.writeEntry( "floatEntry1", FLOATENTRY );

  sc.deleteGroup("deleteMe"); // deleting a nonexistant group

  cg = KConfigGroup(&sc, "Complex Types");
  cg.writeEntry( "rectEntry", RECTENTRY );
  cg.writeEntry( "pointEntry", POINTENTRY );
  cg.writeEntry( "sizeEntry", SIZEENTRY );
  cg.writeEntry( "dateTimeEntry", DATETIMEENTRY );
  cg.writeEntry( "dateEntry", DATETIMEENTRY.date() );

  KConfigGroup ct = cg;
  cg = KConfigGroup(&ct, "Nested Group 1");
  cg.writeEntry("stringentry1", STRINGENTRY1);

  cg = KConfigGroup(&ct, "Nested Group 2");
  cg.writeEntry( "stringEntry2", STRINGENTRY2 );

  cg = KConfigGroup(&cg, "Nested Group 2.1");
  cg.writeEntry( "stringEntry3", STRINGENTRY3 );

  cg = KConfigGroup(&sc, "List Types" );
  cg.writeEntry( "listOfIntsEntry1", INTLISTENTRY1 );
  cg.writeEntry( "listOfByteArraysEntry1", BYTEARRAYLISTENTRY1 );
  cg.writeEntry( "stringListEntry", STRINGLISTENTRY );
  cg.writeEntry( "stringListEmptyEntry", STRINGLISTEMPTYENTRY );
  cg.writeEntry( "stringListJustEmptyElement", STRINGLISTJUSTEMPTYELEMENT );
  cg.writeEntry( "stringListEmptyTrailingElement", STRINGLISTEMPTYTRAINLINGELEMENT );
  cg.writeEntry( "stringListEscapeOddEntry", STRINGLISTESCAPEODDENTRY );
  cg.writeEntry( "stringListEscapeEvenEntry", STRINGLISTESCAPEEVENENTRY );
  cg.writeEntry( "stringListEscapeCommaEntry", STRINGLISTESCAPECOMMAENTRY );
  cg.writeEntry( "variantListEntry", VARIANTLISTENTRY );

  cg = KConfigGroup(&sc, "Path Type" );
  cg.writePathEntry( "homepath", HOMEPATH );
  cg.writePathEntry( "homepathescape", HOMEPATHESCAPE );

  cg = KConfigGroup(&sc, "Enum Types" );
  writeEntry( cg, "enum-10", Tens );
  writeEntry( cg, "enum-100", Hundreds );
  writeEntry( cg, "flags-bit0", Flags(bit0));
  writeEntry( cg, "flags-bit0-bit1", Flags(bit0|bit1) );

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

  //Setup stuff to test KConfig::addConfigSources()
  KConfig devcfg("specificrc");
  KConfigGroup devonlygrp(&devcfg, "Specific Only Group");
  devonlygrp.writeEntry("ExistingEntry", "DevValue");
  KConfigGroup devandbasegrp(&devcfg, "Shared Group");
  devandbasegrp.writeEntry("SomeSharedEntry", "DevValue");
  devandbasegrp.writeEntry("SomeSpecificOnlyEntry", "DevValue");
  devcfg.sync();
  KConfig basecfg("baserc");
  KConfigGroup basegrp(&basecfg, "Base Only Group");
  basegrp.writeEntry("ExistingEntry", "BaseValue");
  KConfigGroup baseanddevgrp(&basecfg, "Shared Group");
  baseanddevgrp.writeEntry("SomeSharedEntry", "BaseValue");
  baseanddevgrp.writeEntry("SomeBaseOnlyEntry", "BaseValue");
  basecfg.sync();

  KConfig gecfg("groupescapetest", KConfig::SimpleConfig);
  cg = KConfigGroup(&gecfg, DOLLARGROUP);
  cg.writeEntry( "entry", "doesntmatter" );

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

  // make sure groupList() isn't returning something it shouldn't
  foreach(const QString& group, sc2.groupList()) {
      QVERIFY(!group.isEmpty() && group != "<default>");
      QVERIFY(!group.contains(QChar(0x1d)));
  }

  KConfigGroup sc3( &sc2, "AAA");

  QVERIFY( sc3.hasKey( "stringEntry1" ) );
  QVERIFY( !sc3.isEntryImmutable("stringEntry1") );
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
  QCOMPARE( sc3.readEntry( "floatEntry1", 0.0f ), FLOATENTRY );
}

void KConfigTest::testDefaults()
{
    KConfig config("defaulttest", KConfig::NoGlobals);
#define DEFAULTS "defaulttest.defaults"
    KConfig defaults(DEFAULTS, KConfig::SimpleConfig);

    const QString Default("Default");
    const QString NotDefault("Not Default");
    const QString Value1(STRINGENTRY1);
    const QString Value2(STRINGENTRY2);

    KConfigGroup group = defaults.group("any group");
    group.writeEntry("entry1", Default);
    group.sync();

    group = config.group("any group");
    group.writeEntry("entry1", Value1);
    group.writeEntry("entry2", Value2);
    group.sync();

    config.addConfigSources(QStringList() << KStandardDirs::locateLocal("config", DEFAULTS));

    config.setReadDefaults(true);
    QCOMPARE(group.readEntry("entry1", QString()), Default);
    QCOMPARE(group.readEntry("entry2", NotDefault), NotDefault); // no default for entry2

    config.setReadDefaults(false);
    QCOMPARE(group.readEntry("entry1", Default), Value1);
    QCOMPARE(group.readEntry("entry2", NotDefault), Value2);

    group.revertToDefault("entry1");
    QCOMPARE(group.readEntry("entry1", QString()), Default);
    group.revertToDefault("entry2");
    QCOMPARE(group.readEntry("entry2", QString()), QString());
#undef DEFAULTS
}

void KConfigTest::testLocale()
{
    KConfig config("kconfigtest.locales", KConfig::SimpleConfig);
    const QString Translated(TRANSLATEDSTRINGENTRY1);
    const QString Untranslated(STRINGENTRY1);

    KConfigGroup group = config.group("Hello");
    group.writeEntry("stringEntry1", Untranslated);
    config.setLocale("fr");
    group.writeEntry("stringEntry1", Translated, KConfig::Localized|KConfig::Persistent);
    config.sync();

    QCOMPARE(group.readEntry("stringEntry1", QString()), Translated);
    QCOMPARE(group.readEntryUntranslated("stringEntry1"), Untranslated);

    config.setLocale("C"); // strings written in the "C" locale are written as nonlocalized
    group.writeEntry("stringEntry1", Untranslated, KConfig::Localized|KConfig::Persistent);
    config.sync();

    QCOMPARE(group.readEntry("stringEntry1", QString()), Untranslated);
}

void KConfigTest::testLists()
{
  KConfig sc2( "kconfigtest" );
  KConfigGroup sc3(&sc2, "List Types");

  QCOMPARE( sc3.readEntry( QString("stringListEntry"), QStringList()),
            STRINGLISTENTRY );

  QCOMPARE( sc3.readEntry( QString("stringListEmptyEntry"), QStringList("wrong") ),
            STRINGLISTEMPTYENTRY );

  QCOMPARE( sc3.readEntry( QString("stringListJustEmptyElement"), QStringList() ),
            STRINGLISTJUSTEMPTYELEMENT );

  QCOMPARE( sc3.readEntry( QString("stringListEmptyTrailingElement"), QStringList() ),
            STRINGLISTEMPTYTRAINLINGELEMENT );

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
  QCOMPARE( sc3.readPathEntry( "homepath", QString() ), HOMEPATH );
  QCOMPARE( sc3.readPathEntry( "homepathescape", QString() ), HOMEPATHESCAPE );

  {
      QFile file(KStandardDirs::locateLocal("config", "pathtest"));
      file.open(QIODevice::WriteOnly|QIODevice::Text);
      QTextStream out(&file);
      out.setCodec("UTF-8");
      out << "[Test Group]" << endl
              << "homePath=$HOME/foo" << endl
              << "homePath2=file://$HOME/foo" << endl
              << "URL[$e]=file://$HOME/foo" << endl
              << "hostname[$e]=$(hostname)" << endl;
  }
  KConfig cf2("pathtest");
  KConfigGroup group = cf2.group("Test Group");
  QVERIFY(group.hasKey("homePath"));
  QCOMPARE(group.readPathEntry("homePath", QString()), HOMEPATH);
  QVERIFY(group.hasKey("homePath2"));
  QCOMPARE(group.readPathEntry("homePath2", QString()), QString("file://") + HOMEPATH );
  QVERIFY(group.hasKey("URL"));
  QCOMPARE(group.readEntry("URL", QString()), QString("file://") + HOMEPATH );
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
  // I don't know if this will work on windows
  // This test hangs on OS X
  QVERIFY(group.hasKey("hostname"));
  QCOMPARE(group.readEntry("hostname", QString()), QHostInfo::localHostName());
#endif
}

void KConfigTest::testPersistenceOfExpandFlagForPath()
{
  // This test checks that a path entry starting with $HOME is still flagged
  // with the expand flag after the config was altered without rewriting the
  // path entry.

  // 1st step: Open the config, add a new dummy entry and then sync the config
  // back to the storage.
  {
  KConfig sc2( "kconfigtest" );
  KConfigGroup sc3(&sc2, "Path Type");
  sc3.writeEntry( "dummy", "dummy" );
  sc2.sync();
  }

  // 2nd step: Call testPath() again. Rewriting the config must not break
  // the testPath() test.
  testPath();
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
  QVERIFY( readEntry( sc3, "enum-100", Ones) != Ones);
  QVERIFY( readEntry( sc3, "enum-100", Ones) != Tens);

  QCOMPARE( sc3.readEntry( "flags-bit0" ), QString("bit0"));
  QVERIFY( readEntry( sc3, "flags-bit0", Flags() ) == bit0 );

  int eid = staticMetaObject.indexOfEnumerator( "Flags" );
  QVERIFY( eid != -1 );
  QMetaEnum me = staticMetaObject.enumerator( eid );
  Flags bitfield = bit0|bit1;

  QCOMPARE( sc3.readEntry( "flags-bit0-bit1" ), QString( me.valueToKeys(bitfield) ) );
  QVERIFY( readEntry( sc3, "flags-bit0-bit1", Flags() ) == bitfield );
}

void KConfigTest::testEntryMap()
{
    KConfig sc("kconfigtest");
    KConfigGroup cg(&sc, "Hello");
    QMap<QString, QString> entryMap = cg.entryMap();
    qDebug() << entryMap.keys();
    QCOMPARE(entryMap.value("stringEntry1"), QString(STRINGENTRY1));
    QCOMPARE(entryMap.value("stringEntry2"), QString(STRINGENTRY2));
    QCOMPARE(entryMap.value("stringEntry3"), QString(STRINGENTRY3));
    QCOMPARE(entryMap.value("stringEntry4"), QString(STRINGENTRY4));
    QVERIFY(!entryMap.contains("stringEntry5"));
    QVERIFY(!entryMap.contains("stringEntry6"));
    QCOMPARE(entryMap.value("Test"), QString::fromUtf8(UTF8BITENTRY));
    QCOMPARE(entryMap.value("bytearrayEntry"), QString::fromUtf8(BYTEARRAYENTRY));
    QCOMPARE(entryMap.value("emptyEntry"), QString());
    QVERIFY(entryMap.contains("emptyEntry"));
    QCOMPARE(entryMap.value("boolEntry1"), QString(BOOLENTRY1?"true":"false"));
    QCOMPARE(entryMap.value("boolEntry2"), QString(BOOLENTRY2?"true":"false"));
    QCOMPARE(entryMap.value("keywith=equalsign"), QString(STRINGENTRY1));
    QCOMPARE(entryMap.value("byteArrayEntry1"), QString(STRINGENTRY1));
    QCOMPARE(entryMap.value("doubleEntry1"), QString::number(DOUBLEENTRY, 'g', 15));
    QCOMPARE(entryMap.value("floatEntry1"), QString::number(FLOATENTRY, 'g', 8));
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
    newGroup.changeGroup("FooBar"); // deprecated!
    QCOMPARE(newGroup.name(), QString("FooBar"));
    QCOMPARE(sc3.name(), QString("Hello")); // unchanged
    KConfigGroup rootGroup(sc.group(""));
    QCOMPARE(rootGroup.name(), QString("<default>"));
    KConfigGroup sc32(rootGroup.group("Hello"));
    QCOMPARE(sc32.name(), QString("Hello"));
    KConfigGroup newGroup2(sc32);
    newGroup2.changeGroup("FooBar"); // deprecated!
    QCOMPARE(newGroup2.name(), QString("FooBar"));
    QCOMPARE(sc32.name(), QString("Hello")); // unchanged
}

void KConfigTest::testDelete()
{
  KConfig sc( "kconfigtest" );
  KConfigGroup sc3(&sc, "Hello");

  sc3.deleteEntry("Test");
  QCOMPARE( sc3.readEntry("Test", QString("Fietsbel")), QString("Fietsbel") );

  KConfigGroup ct(&sc, "Complex Types");
  KConfigGroup ng(&ct, "Nested Group 2");
  sc.deleteGroup("Complex Types");
  QCOMPARE(sc.group("Complex Types").keyList().count(), 0);
  QVERIFY(sc.group("Complex Types").exists()); // yep, we deleted it, but it still "exists"...
  QCOMPARE(ct.group("Nested Group 1").keyList().count(), 0);
  QCOMPARE(ct.group("Nested Group 2").keyList().count(), 0);
  QCOMPARE(ng.group("Nested Group 2.1").keyList().count(), 0);

  KConfigGroup cg(&sc , "AAA" );
  cg.deleteGroup();
  QVERIFY( sc.entryMap("Complex Types").isEmpty() );
  QVERIFY( sc.entryMap("AAA").isEmpty() );
  QVERIFY( !sc.entryMap("Hello").isEmpty() ); //not deleted group
  QVERIFY( sc.entryMap("FooBar").isEmpty() ); //inexistant group

  // test for entries that are marked as deleted when there is no default
  KConfig cf("kconfigtest", KConfig::SimpleConfig); // make sure there are no defaults
  cg = cf.group("Portable Devices");
  cg.writeEntry("devices|manual|(null)", "whatever");
  cg.writeEntry("devices|manual|/mnt/ipod", "/mnt/ipod");
  cf.sync();

  int count=0;
  foreach(const QByteArray& item, readLines())
      if (item.startsWith("devices|"))
          count++;
  QVERIFY(count == 2);
  cg.deleteEntry("devices|manual|/mnt/ipod");
  cf.sync();
  foreach(const QByteArray& item, readLines())
      QVERIFY(!item.contains("ipod"));
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

void KConfigTest::testMerge()
{
    KConfig config("mergetest", KConfig::SimpleConfig);

    KConfigGroup cg = config.group("some group");
    cg.writeEntry("entry", " random entry");
    cg.writeEntry("another entry", "blah blah blah");

    { // simulate writing by another process
        QFile file(KStandardDirs::locateLocal("config", "mergetest"));
        file.open(QIODevice::WriteOnly|QIODevice::Text);
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << "[Merged Group]" << endl
                << "entry1=Testing" << endl
                << "entry2=More Testing" << endl
                << "[some group]" << endl
                << "entry[fr]=French" << endl
                << "entry[es]=Spanish" << endl
                << "entry[de]=German" << endl;
    }
    config.sync();

    {
        QList<QByteArray> lines;
        // this is what the file should look like
        lines << "[Merged Group]\n"
                << "entry1=Testing\n"
                << "entry2=More Testing\n"
                << "\n"
                << "[some group]\n"
                << "another entry=blah blah blah\n"
                << "entry=\\srandom entry\n"
                << "entry[de]=German\n"
                << "entry[es]=Spanish\n"
                << "entry[fr]=French\n";
        QFile file(KStandardDirs::locateLocal("config", "mergetest"));
        file.open(QIODevice::ReadOnly|QIODevice::Text);
        foreach (const QByteArray& line, lines) {
            QCOMPARE(line, file.readLine());
        }
    }
}

void KConfigTest::testImmutable()
{
    {
        QFile file(KStandardDirs::locateLocal("config", "immutabletest"));
        file.open(QIODevice::WriteOnly|QIODevice::Text);
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << "[$i]" << endl
                << "entry1=Testing" << endl
            << "[group][$i]" << endl
            << "[group][subgroup][$i]" << endl;
    }

    KConfig config("immutabletest", KConfig::SimpleConfig);
    QVERIFY(config.isGroupImmutable(QByteArray()));
    KConfigGroup cg = config.group(QByteArray());
    QVERIFY(cg.isEntryImmutable("entry1"));
    KConfigGroup cg1 = config.group("group");
    QVERIFY(cg1.isImmutable());
    KConfigGroup cg1a = cg.group("group");
    QVERIFY(cg1a.isImmutable());
    KConfigGroup cg2 = cg1.group("subgroup");
    QVERIFY(cg2.isImmutable());
}

void KConfigTest::testOptionOrder()
{
    {
        QFile file(KStandardDirs::locateLocal("config", "doubleattrtest"));
        file.open(QIODevice::WriteOnly|QIODevice::Text);
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << "[group3]" << endl
            << "entry2=unlocalized" << endl
            << "entry2[$i][de_DE]=t2" << endl;
    }
    KConfig config("doubleattrtest", KConfig::SimpleConfig);
    config.setLocale("de_DE");
    KConfigGroup cg3 = config.group("group3");
    QVERIFY(!cg3.isImmutable());
    QCOMPARE(cg3.readEntry("entry2",""), QString("t2"));
    QVERIFY(cg3.isEntryImmutable("entry2"));
    config.setLocale("C");
    QCOMPARE(cg3.readEntry("entry2",""), QString("unlocalized"));
    QVERIFY(!cg3.isEntryImmutable("entry2"));
    cg3.writeEntry("entry2","modified");
    config.sync();

    {
        QList<QByteArray> lines;
        // this is what the file should look like
        lines << "[group3]\n" 
              << "entry2=modified\n"
              << "entry2[de_DE][$i]=t2\n";

        QFile file(KStandardDirs::locateLocal("config", "doubleattrtest"));
        file.open(QIODevice::ReadOnly|QIODevice::Text);
        foreach (const QByteArray& line, lines) {
            QCOMPARE(line, file.readLine());
        }
    }
}


void KConfigTest::testGroupEscape()
{
    KConfig config("groupescapetest", KConfig::SimpleConfig);
    QVERIFY( config.group(DOLLARGROUP).exists() );
}

void KConfigTest::testSubGroup()
{
    KConfig sc( "kconfigtest" );
    KConfigGroup cg( &sc, "ParentGroup" );
    QCOMPARE(cg.readEntry( "parentgrpstring", ""), QString("somevalue") );
    KConfigGroup subcg1( &cg, "SubGroup1");
    QCOMPARE(subcg1.name(), QString("SubGroup1"));
    QCOMPARE(subcg1.readEntry( "somestring", ""), QString("somevalue") );
    KConfigGroup subcg2( &cg, "SubGroup2");
    QCOMPARE(subcg2.name(), QString("SubGroup2"));
    QCOMPARE(subcg2.readEntry( "substring", ""), QString("somevalue") );
    KConfigGroup subcg3( &cg, "SubGroup/3");
    QCOMPARE(subcg3.readEntry( "sub3string", ""), QString("somevalue") );
    QCOMPARE(subcg3.name(), QString("SubGroup/3"));
    KConfigGroup rcg( &sc, "" );
    KConfigGroup srcg( &rcg, "ParentGroup" );
    QCOMPARE(srcg.readEntry( "parentgrpstring", ""), QString("somevalue") );

    QCOMPARE(cg.groupList(), SUBGROUPLIST );

    QCOMPARE(subcg3.keyList(), SUBGROUP3KEYS);
    QCOMPARE(cg.keyList(), PARENTGROUPKEYS);

    QCOMPARE(QStringList(cg.entryMap().keys()), PARENTGROUPKEYS);
    QCOMPARE(QStringList(subcg3.entryMap().keys()), SUBGROUP3KEYS);

    // make sure groupList() isn't returning something it shouldn't
    foreach(const QString& group, sc.groupList()) {
      QVERIFY(!group.isEmpty() && group != "<default>");
      QVERIFY(!group.contains(QChar(0x1d)));
      QVERIFY(!group.contains("subgroup"));
      QVERIFY(!group.contains("SubGroup"));
    }
}

void KConfigTest::testKdeglobals()
{
    const QString kdeglobals = KStandardDirs::locateLocal("config", "kdeglobals");
    KConfig sc("kdeglobals");
    QVERIFY(sc.forceGlobal());
    QVERIFY(sc.name() == QLatin1String("kdeglobals"));

    KConfig sc2("", KConfig::IncludeGlobals);
    QVERIFY(sc2.forceGlobal());
    QVERIFY(sc2.name() == QLatin1String("kdeglobals"));

    KConfig sc3("kdeglobals", KConfig::SimpleConfig);
    QVERIFY(sc3.forceGlobal());
    QVERIFY(sc3.name() == QLatin1String("kdeglobals"));
}

void KConfigTest::testAddConfigSources()
{
    KConfig cf("specificrc");

    cf.addConfigSources(QStringList() << KStandardDirs::locateLocal("config", "baserc"));
    cf.reparseConfiguration();

    KConfigGroup specificgrp(&cf, "Specific Only Group");
    QCOMPARE(specificgrp.readEntry("ExistingEntry", ""), QString("DevValue"));

    KConfigGroup sharedgrp(&cf, "Shared Group");
    QCOMPARE(sharedgrp.readEntry("SomeSpecificOnlyEntry",""), QString("DevValue"));
    QCOMPARE(sharedgrp.readEntry("SomeBaseOnlyEntry",""), QString("BaseValue"));
    QCOMPARE(sharedgrp.readEntry("SomeSharedEntry",""), QString("DevValue"));

    KConfigGroup basegrp(&cf, "Base Only Group");
    QCOMPARE(basegrp.readEntry("ExistingEntry", ""), QString("BaseValue"));
    basegrp.writeEntry("New Entry Base Only", "SomeValue");

    KConfigGroup newgrp(&cf, "New Group");
    newgrp.writeEntry("New Entry", "SomeValue");

    cf.sync();

    KConfig plaincfg("specificrc");

    KConfigGroup newgrp2(&plaincfg, "New Group");
    QCOMPARE(newgrp2.readEntry("New Entry", ""), QString("SomeValue"));

    KConfigGroup basegrp2(&plaincfg, "Base Only Group");
    QCOMPARE(basegrp2.readEntry("New Entry Base Only", ""), QString("SomeValue"));
}

void KConfigTest::testGroupCopyTo()
{
    KConfig cf1("kconfigtest");
    KConfigGroup original = cf1.group("Enum Types");

    KConfigGroup copy = cf1.group("Enum Types Copy");
    original.copyTo(&copy); // copy from one group to another
    QCOMPARE(copy.entryMap(), original.entryMap());

    KConfig cf2("copy_of_kconfigtest", KConfig::SimpleConfig);
    QVERIFY(!cf2.hasGroup(original.name()));
    QVERIFY(!cf2.hasGroup(copy.name()));

    KConfigGroup newGroup = cf2.group(original.name());
    original.copyTo(&newGroup); // copy from one file to another
    QVERIFY(cf2.hasGroup(original.name()));
    QVERIFY(!cf2.hasGroup(copy.name())); // make sure we didn't copy more than we wanted
    QCOMPARE(newGroup.entryMap(), original.entryMap());
}

void KConfigTest::testConfigCopyToSync()
{
    KConfig cf1("kconfigtest");
    // Prepare source file
    KConfigGroup group(&cf1, "CopyToTest");
    group.writeEntry("Type", "Test");
    cf1.sync();

    // Copy to "destination"
    const QString destination = KStandardDirs::locateLocal("config", "kconfigcopytotest");
    QFile::remove(destination);

    KConfig cf2("kconfigcopytotest");
    KConfigGroup group2(&cf2, "CopyToTest");

    group.copyTo(&group2);

    QString testVal = group2.readEntry("Type");
    QCOMPARE(testVal, QString("Test"));
    // should write to disk the copied data from group
    cf2.sync();
    QVERIFY(QFile::exists(destination));
}

void KConfigTest::testConfigCopyTo()
{
    KConfig cf1("kconfigtest");
    {
        // Prepare source file
        KConfigGroup group(&cf1, "CopyToTest");
        group.writeEntry("Type", "Test");
        cf1.sync();
    }

    {
        // Copy to "destination"
        const QString destination = KStandardDirs::locateLocal("config", "kconfigcopytotest");
        QFile::remove(destination);
        KConfig cf2;
        cf1.copyTo(destination, &cf2);
        KConfigGroup group2(&cf2, "CopyToTest");
        QString testVal = group2.readEntry("Type");
        QCOMPARE(testVal, QString("Test"));
        cf2.sync();
        QVERIFY(QFile::exists(destination));
    }

    // Check copied config file on disk
    KConfig cf3("kconfigcopytotest");
    KConfigGroup group3(&cf3, "CopyToTest");
    QString testVal = group3.readEntry("Type");
    QCOMPARE(testVal, QString("Test"));
}

void KConfigTest::testReparent()
{
    KConfig cf("kconfigtest");
    const QString name("Enum Types");
    KConfigGroup group = cf.group(name);
    const QMap<QString, QString> originalMap = group.entryMap();
    KConfigGroup parent = cf.group("Parent Group");

    QVERIFY(!parent.hasGroup(name));

    QVERIFY(group.entryMap() == originalMap);

    group.reparent(&parent); // see if it can be made a sub-group of another group
    QVERIFY(parent.hasGroup(name));
    QCOMPARE(group.entryMap(), originalMap);

    group.reparent(&cf); // see if it can make it a top-level group again
//    QVERIFY(!parent.hasGroup(name));
    QCOMPARE(group.entryMap(), originalMap);
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

static void ageTimeStamp(const QString& path, int nsec)
{
#ifdef Q_OS_UNIX
    QDateTime mtime = QFileInfo(path).lastModified().addSecs(-nsec);
    struct utimbuf utbuf;
    utbuf.actime = mtime.toTime_t();
    utbuf.modtime = utbuf.actime;
    utime(QFile::encodeName(path), &utbuf);
#else
    QTest::qSleep(nsec * 1000);
#endif
}

void KConfigTest::testWriteOnSync()
{
    QDateTime oldStamp, newStamp;
    KConfig sc("kconfigtest", KConfig::IncludeGlobals);

    // Age the timestamp of global config file a few sec, and collect it.
    QString globFile = KStandardDirs::locateLocal("config", "kdeglobals");
    ageTimeStamp(globFile, 2); // age 2 sec
    oldStamp = QFileInfo(globFile).lastModified();

    // Add a local entry and sync the config.
    // Should not rewrite the global config file.
    KConfigGroup cgLocal(&sc, "Locals");
    cgLocal.writeEntry("someLocalString", "whatever");
    sc.sync();

    // Verify that the timestamp of global config file didn't change.
    newStamp = QFileInfo(globFile).lastModified();
    QCOMPARE(newStamp, oldStamp);

    // Age the timestamp of local config file a few sec, and collect it.
    QString locFile = KStandardDirs::locateLocal("config", "kconfigtest");
    ageTimeStamp(locFile, 2); // age 2 sec
    oldStamp = QFileInfo(locFile).lastModified();

    // Add a global entry and sync the config.
    // Should not rewrite the local config file.
    KConfigGroup cgGlobal(&sc, "Globals");
    cgGlobal.writeEntry("someGlobalString", "whatever",
                        KConfig::Persistent|KConfig::Global);
    sc.sync();

    // Verify that the timestamp of local config file didn't change.
    newStamp = QFileInfo(locFile).lastModified();
    QCOMPARE(newStamp, oldStamp);
}

void KConfigTest::testDirtyOnEqual()
{
    QDateTime oldStamp, newStamp;
    KConfig sc("kconfigtest");

    // Initialize value
    KConfigGroup cgLocal(&sc, "random");
    cgLocal.writeEntry("theKey", "whatever");
    sc.sync();

    // Age the timestamp of local config file a few sec, and collect it.
    QString locFile = KStandardDirs::locateLocal("config", "kconfigtest");
    ageTimeStamp(locFile, 2); // age 2 sec
    oldStamp = QFileInfo(locFile).lastModified();

    // Write exactly the same again
    cgLocal.writeEntry("theKey", "whatever");
    // This should be a no-op
    sc.sync();

    // Verify that the timestamp of local config file didn't change.
    newStamp = QFileInfo(locFile).lastModified();
    QCOMPARE(newStamp, oldStamp);
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

void KConfigTest::testCreateDir()
{
    // Test auto-creating the parent directory when needed (KConfigIniBackend::createEnclosing)
    QString kdehome = QDir::home().canonicalPath() + "/.kde-unit-test";
    QString subdir = kdehome + "/newsubdir";
    QString file = subdir + "/foo.desktop";
    QFile::remove(file);
    QDir().rmdir(subdir);
    QVERIFY(!QDir().exists(subdir));
    KDesktopFile desktopFile(file);
    desktopFile.desktopGroup().writeEntry("key", "value");
    desktopFile.sync();
    QVERIFY(QFile::exists(file));
}

void KConfigTest::testSyncOnExit()
{
    // Often, the KGlobalPrivate global static's destructor ends up calling ~KConfig ->
    // KConfig::sync ... and if that code triggers KGlobal code again then things could crash.
    // So here's a test for modifying KGlobal::config() and not syncing, the process exit will sync.
    KConfigGroup grp(KGlobal::config(), "syncOnExit");
    grp.writeEntry("key", "value");
}

void KConfigTest::testSharedConfig()
{
    // Can I use a KConfigGroup even after the KSharedConfigPtr goes out of scope?
    KConfigGroup myConfigGroup;
    {
        KSharedConfigPtr config = KSharedConfig::openConfig("kconfigtest");
        myConfigGroup = KConfigGroup(config, "Hello");
    }
    QCOMPARE(myConfigGroup.readEntry("stringEntry1"), QString(STRINGENTRY1));
}

void KConfigTest::testLocaleConfig()
{
    QDir dir;
    QString subdir = QDir::home().canonicalPath() + "/.kde-unit-test/";
    dir.mkpath(subdir);
    QString file = subdir + "/localized.test";
    QFile::remove(file);
    QFile f(file);
    QVERIFY(f.open(QIODevice::WriteOnly));
    QTextStream ts(&f);
    ts << "[Test]\n";
    ts << "foo[ca]=5\n";
    ts << "foostring[ca]=nice\n";
    ts << "foobool[ca]=true\n";
    f.close();
    QVERIFY(QFile::exists(file));
    KConfig config(file);
    config.setLocale("ca");
    KConfigGroup cg(&config, "Test");
    QCOMPARE(cg.readEntry("foo"), QString("5"));
    QCOMPARE(cg.readEntry("foo", 3), 5);
    QCOMPARE(cg.readEntry("foostring"), QString("nice"));
    QCOMPARE(cg.readEntry("foostring", "ugly"), QString("nice"));
    QCOMPARE(cg.readEntry("foobool"), QString("true"));
    QCOMPARE(cg.readEntry("foobool", false), true);
}

void KConfigTest::testSpecialChars()
{
  QString test = "!TEST!";
  KConfig config( "kconfigtest" );
  KConfigGroup group = config.group( "specialchartest" );
  group.writeEntry( "bar", KStringHandler::obscure( test ) );
  QCOMPARE( KStringHandler::obscure( group.readEntry( "bar" ) ), test );
}
