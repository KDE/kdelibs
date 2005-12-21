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

#include <qttest_kde.h>
#include "kconfigtest.h"
#include "kconfigtest.moc"

#include <kconfig.h>
#include <kdebug.h>

QTTEST_KDEMAIN( KConfigTest, NoGUI )

#define BOOLENTRY1 true
#define BOOLENTRY2 false
#define STRINGENTRY1 "hello"
#define STRINGENTRY2 " hello"
#define STRINGENTRY3 "hello "
#define STRINGENTRY4 " hello "
#define STRINGENTRY5 " "
#define STRINGENTRY6 ""
#define LOCAL8BITENTRY "Hello äöü"
#define POINTENTRY QPoint( 4351, 1235 )
#define SIZEENTRY QSize( 10, 20 )
#define RECTENTRY QRect( 10, 23, 5321, 13 )
#define DATETIMEENTRY QDateTime( QDate( 2002, 06, 23 ), QTime( 12, 55, 40 ) )
#define STRINGLISTENTRY (QStringList( "Hello," ) << " World")
#define INTLISTENTRY1 QList<int>() << 1 << 2 << 3 << 4
#define BYTEARRAYLISTENTRY1 QList<QByteArray>() << "" << "1,2" << "end"
#define COLORENTRY QColor("steelblue")
#define FONTENTRY QFont("Times", 16, QFont::Normal)

void KConfigTest::writeConfigFile()
{
  KConfig sc( "kconfigtest" );

  sc.setGroup("AAA");
  sc.writeEntry("stringEntry1", STRINGENTRY1, true, true);
  sc.deleteEntry("stringEntry2", false, true);

  sc.setGroup("Hello");
  sc.writeEntry( "boolEntry1", BOOLENTRY1 );
  sc.writeEntry( "boolEntry2", BOOLENTRY2 );

  sc.writeEntry( "Test", QByteArray( LOCAL8BITENTRY ) );
  sc.writeEntry( "Test2", "");
  sc.writeEntry( "stringEntry1", STRINGENTRY1 );
  sc.writeEntry( "stringEntry2", STRINGENTRY2 );
  sc.writeEntry( "stringEntry3", STRINGENTRY3 );
  sc.writeEntry( "stringEntry4", STRINGENTRY4 );
  sc.writeEntry( "stringEntry5", STRINGENTRY5 );
//  sc.writeEntry( "stringEntry6", STRINGENTRY6 );
  sc.writeEntry( "keywith=equalsign", STRINGENTRY1 );
  sc.deleteEntry( "stringEntry5" );
  sc.deleteEntry( "stringEntry6" );

  sc.deleteGroup("deleteMe", true);

  sc.setGroup("OtherTypes");
  sc.writeEntry( "rectEntry", RECTENTRY );
  sc.writeEntry( "pointEntry", POINTENTRY );
  sc.writeEntry( "sizeEntry", SIZEENTRY );
  sc.writeEntry( "dateTimeEntry", DATETIMEENTRY );
  sc.writeEntry( "stringListEntry", STRINGLISTENTRY );
  sc.writeEntry( "byteArrayEntry1", QByteArray( STRINGENTRY1 ), true, true );
  sc.writeEntry( "listOfIntsEntry1", INTLISTENTRY1 );
  sc.writeEntry( "listOfByteArraysEntry1", BYTEARRAYLISTENTRY1 );
  sc.writeEntry( "colorEntry", COLORENTRY );
  sc.writeEntry( "fontEntry", FONTENTRY );
  sc.sync();
}

// ### TODO: call this, and test the state of things afterwards
void KConfigTest::revertEntries()
{
  qWarning("Reverting entries");
  KConfig sc( "kconfigtest" );

  sc.setGroup("Hello");
  sc.revertToDefault( "boolEntry1");
  sc.revertToDefault( "boolEntry2");

  sc.revertToDefault( "Test" );
  sc.revertToDefault( "Test2" );
  sc.revertToDefault( "stringEntry1" );
  sc.revertToDefault( "stringEntry2" );
  sc.revertToDefault( "stringEntry3" );
  sc.revertToDefault( "stringEntry4" );
  sc.revertToDefault( "stringEntry5" );
  sc.sync();
}

void KConfigTest::testAll()
{
  kdDebug() << k_funcinfo << endl;
  writeConfigFile();

  KConfig sc2( "kconfigtest" );

  KConfigGroup sc3( &sc2, "AAA");
  bool bImmutable = sc3.entryIsImmutable("stringEntry1");

  QVERIFY( !bImmutable );
  //qWarning("sc3.entryIsImmutable() 1: %s", bImmutable ? "true" : "false");

  sc2.setGroup("AAA");
  QVERIFY( sc2.hasKey( "stringEntry1" ) );
  QCOMPARE( sc2.readEntry( "stringEntry1" ), QString( STRINGENTRY1 ) );
  QCOMPARE( sc2.entryIsImmutable("stringEntry1"), bImmutable );
  QVERIFY( !sc2.hasKey( "stringEntry2" ) );
  QCOMPARE( sc2.readEntry( "stringEntry2", QString("bla") ), QString( "bla" ) );

  QVERIFY( !sc2.hasDefault( "stringEntry1" ) );

  sc2.setGroup("Hello");
  QCOMPARE( sc2.readEntry( "Test" ), QString::fromLocal8Bit( LOCAL8BITENTRY ) );
  QCOMPARE( sc2.readEntry("Test2", QString("Fietsbel")).isEmpty(), true );
  QCOMPARE( sc2.readEntry( "stringEntry1" ), QString( STRINGENTRY1 ) );
  QCOMPARE( sc2.readEntry( "stringEntry2" ), QString( STRINGENTRY2 ) );
  QCOMPARE( sc2.readEntry( "stringEntry3" ), QString( STRINGENTRY3 ) );
  QCOMPARE( sc2.readEntry( "stringEntry4" ), QString( STRINGENTRY4 ) );
  QVERIFY( !sc2.hasKey( "stringEntry5" ) );
  QCOMPARE( sc2.readEntry( "stringEntry5", QString("test") ), QString( "test" ) );
  QVERIFY( !sc2.hasKey( "stringEntry6" ) );
  QCOMPARE( sc2.readEntry( "stringEntry6", QString("foo") ), QString( "foo" ) );
  QCOMPARE( sc2.readEntry( "boolEntry1", BOOLENTRY1 ).toBool(), BOOLENTRY1 );
  QCOMPARE( sc2.readEntry( "boolEntry2", QVariant::Bool ).toBool(), BOOLENTRY2 );

#if 0
  QString s;
  s = sc2.readEntry( "keywith=equalsign" );
  fprintf(stderr, "comparing keywith=equalsign %s with %s -> ", STRINGENTRY1, s.latin1());
  if (s == STRINGENTRY1)
    fprintf(stderr, "OK\n");
  else {
    fprintf(stderr, "not OK\n");
    exit(-1);
  }
#endif

  sc2.setGroup("OtherTypes");

  QCOMPARE( sc2.readEntry( "pointEntry", QPoint() ).toPoint(), POINTENTRY );
  QCOMPARE( sc2.readEntry( "sizeEntry", SIZEENTRY ).toSize(), SIZEENTRY);
  QCOMPARE( sc2.readEntry( "rectEntry", QVariant::Rect ).toRect(), RECTENTRY );
  QCOMPARE( sc2.readEntry( "dateTimeEntry", QDateTime() ).toString(),
            DATETIMEENTRY.toString(Qt::ISODate) );
  QCOMPARE( sc2.readEntry( "dateTimeEntry", QDate() ).toString(),
            DATETIMEENTRY.date().toString(Qt::ISODate) );
  QCOMPARE( sc2.readEntry( "stringListEntry", QStringList()).toStringList().join( "," ),
            STRINGLISTENTRY.join( "," ) );
  QCOMPARE( sc2.readEntry( "colorEntry", QColor(Qt::black) ).toString(),
            QVariant(COLORENTRY).toString() );
  QCOMPARE( qvariant_cast<QColor>(sc2.readEntry( "colorEntry" )), COLORENTRY );
  QCOMPARE( sc2.readEntry( "fontEntry", /*QFont()*/QVariant::Font ).toString(), QVariant(FONTENTRY).toString() );

  QCOMPARE( sc2.readEntry( "byteArrayEntry1", QVariant::ByteArray ).toByteArray(), QByteArray( STRINGENTRY1 ) );
  QCOMPARE( sc2.readEntry( "listOfIntsEntry1" ), QString::fromLatin1( "1,2,3,4" ) );
  QList<int> expectedIntList = INTLISTENTRY1;
  QCOMPARE( sc2.readIntListEntry( "listOfIntsEntry1" ), expectedIntList );

  QCOMPARE( sc2.readEntry( "listOfByteArraysEntry1" ), QString::fromLatin1( ",1\\,2,end" ) );
  QList<QByteArray> baList = sc2.readByteArrayListEntry( "listOfByteArraysEntry1" );
  QList<QByteArray> expectedBaList = BYTEARRAYLISTENTRY1;
  QCOMPARE( baList, expectedBaList );
}
