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
#include "kconfigtest.moc"

#include <kconfig.h>
#include <kdebug.h>

#define CRASH_ON_VARIANTLIST2 0

QTEST_KDEMAIN( KConfigTest, NoGUI )

#define BOOLENTRY1 true
#define BOOLENTRY2 false
#define STRINGENTRY1 "hello"
#define STRINGENTRY2 " hello"
#define STRINGENTRY3 "hello "
#define STRINGENTRY4 " hello "
#define STRINGENTRY5 " "
#define STRINGENTRY6 ""
#define UTF8BITENTRY "Hello äöü"
#define POINTENTRY QPoint( 4351, 1235 )
#define SIZEENTRY QSize( 10, 20 )
#define RECTENTRY QRect( 10, 23, 5321, 13 )
#define DATETIMEENTRY QDateTime( QDate( 2002, 06, 23 ), QTime( 12, 55, 40 ) )
#define STRINGLISTENTRY (QStringList( "Hello," ) << " World")
#define INTLISTENTRY1 QList<int>() << 1 << 2 << 3 << 4
#define BYTEARRAYLISTENTRY1 QList<QByteArray>() << "" << "1,2" << "end"
#define COLORENTRY1 QColor("steelblue")
#define COLORENTRY2 QColor(235, 235, 100, 125)
#define COLORENTRY3 QColor(234, 234, 127)
#define FONTENTRY QFont("Times", 16, QFont::Normal)
#define VARIANTLISTENTRY (QVariantList() << true << false << QString("joe") << 10023)
#define VARIANTLISTENTRY2 (QVariantList() << POINTENTRY << SIZEENTRY)

void KConfigTest::initTestCase()
{
  KConfig sc( "kconfigtest" );

  sc.setGroup("AAA");
  sc.writeEntry("stringEntry1", STRINGENTRY1,
                KConfigBase::Persistent|KConfigBase::Global);
  sc.deleteEntry("stringEntry2", KConfigBase::Global);

  sc.setGroup("Hello");
  sc.writeEntry( "boolEntry1", BOOLENTRY1 );
  sc.writeEntry( "boolEntry2", BOOLENTRY2 );

  QByteArray data( UTF8BITENTRY );
  QCOMPARE( data.size(), 12 ); // the source file is in utf8
  sc.writeEntry( "Test", QVariant( data ) ); // passing "data" converts it to char* and KConfigBase calls fromLatin1!
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
  sc.writeEntry( "byteArrayEntry1", QByteArray( STRINGENTRY1 ),
                 KConfigBase::Global|KConfigBase::Persistent );

  sc.deleteGroup("deleteMe");

  sc.setGroup("ComplexTypes");
  sc.writeEntry( "rectEntry", RECTENTRY );
  sc.writeEntry( "pointEntry", POINTENTRY );
  sc.writeEntry( "sizeEntry", SIZEENTRY );
  sc.writeEntry( "dateTimeEntry", DATETIMEENTRY );
  sc.writeEntry( "colorEntry1", COLORENTRY1 );
  sc.writeEntry( "colorEntry2", COLORENTRY2 );
  sc.writeEntry( "colorEntry3", (QList<int>() << 234 << 234 << 127));
  sc.writeEntry( "colorEntry4",  (QList<int>() << 235 << 235 << 100 << 125));
  sc.writeEntry( "fontEntry", FONTENTRY );

  sc.setGroup( "ListTypes" );
  sc.writeEntry( "listOfIntsEntry1", INTLISTENTRY1 );
  sc.writeEntry( "listOfByteArraysEntry1", BYTEARRAYLISTENTRY1 );
  sc.writeEntry( "stringListEntry", STRINGLISTENTRY );
  sc.writeEntry( "variantListEntry", VARIANTLISTENTRY );

#if CRASH_ON_VARIANTLIST2
  // if debugging this _should_ cause a crash, otherwise a warning
  sc.writeEntry( "variantListEntry2", VARIANTLISTENTRY2 );
#endif
  sc.sync();
}

// ### TODO: call this, and test the state of things afterwards
void KConfigTest::revertEntries()
{
//  qWarning("Reverting entries");
  KConfig sc( "kconfigtest" );

  sc.setGroup("Hello");
  sc.revertToDefault( "boolEntry1" );
  sc.revertToDefault( "boolEntry2" );

  sc.revertToDefault( "Test" );
  sc.revertToDefault( "Test2" );
  sc.revertToDefault( "stringEntry1" );
  sc.revertToDefault( "stringEntry2" );
  sc.revertToDefault( "stringEntry3" );
  sc.revertToDefault( "stringEntry4" );
  sc.revertToDefault( "stringEntry5" );
  sc.sync();
}

void KConfigTest::testSimple()
{
//  kdDebug() << k_funcinfo << endl;

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
  QCOMPARE( sc2.readEntry( "Test", QByteArray() ), QByteArray( UTF8BITENTRY ) );
  QCOMPARE( sc2.readEntry( "Test", QString() ), QString::fromUtf8( UTF8BITENTRY ) );
  QCOMPARE( sc2.readEntry("Test2", QString("Fietsbel")).isEmpty(), true );
  QCOMPARE( sc2.readEntry( "stringEntry1" ), QString( STRINGENTRY1 ) );
  QCOMPARE( sc2.readEntry( "stringEntry2" ), QString( STRINGENTRY2 ) );
  QCOMPARE( sc2.readEntry( "stringEntry3" ), QString( STRINGENTRY3 ) );
  QCOMPARE( sc2.readEntry( "stringEntry4" ), QString( STRINGENTRY4 ) );
  QVERIFY( !sc2.hasKey( "stringEntry5" ) );
  QCOMPARE( sc2.readEntry( "stringEntry5", QString("test") ), QString( "test" ) );
  QVERIFY( !sc2.hasKey( "stringEntry6" ) );
  QCOMPARE( sc2.readEntry( "stringEntry6", QString("foo") ), QString( "foo" ) );
  QCOMPARE( sc2.readEntry( "boolEntry1", BOOLENTRY1 ), BOOLENTRY1 );
  QCOMPARE( sc2.readEntry( "boolEntry2", false ), BOOLENTRY2 );

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

  QCOMPARE( sc2.readEntry( "byteArrayEntry1", QByteArray() ),
            QByteArray( STRINGENTRY1 ) );
}

void KConfigTest::testLists()
{
  KConfig sc2( "kconfigtest" );
  sc2.setGroup("ListTypes");

  QCOMPARE( sc2.readEntry( "stringListEntry", QStringList()),
            STRINGLISTENTRY );

  QCOMPARE( sc2.readEntry( "listOfIntsEntry1" ), QString::fromLatin1( "1,2,3,4" ) );
  QList<int> expectedIntList = INTLISTENTRY1;
  QVERIFY( sc2.readEntry( "listOfIntsEntry1", QList<int>() ) == expectedIntList );

  QCOMPARE( QVariant(sc2.readEntry( "variantListEntry", VARIANTLISTENTRY )).toStringList(),
            QVariant(VARIANTLISTENTRY).toStringList() );

  QCOMPARE( sc2.readEntry( "listOfByteArraysEntry1", QList<QByteArray>()), BYTEARRAYLISTENTRY1 );
}

void KConfigTest::testComplex()
{
  KConfig sc2( "kconfigtest" );
  sc2.setGroup("ComplexTypes");

  QCOMPARE( sc2.readEntry( "pointEntry", QPoint() ), POINTENTRY );
  QCOMPARE( sc2.readEntry( "sizeEntry", SIZEENTRY ), SIZEENTRY);
  QCOMPARE( sc2.readEntry( "rectEntry", QRect(1,2,3,4) ), RECTENTRY );
  QCOMPARE( sc2.readEntry( "dateTimeEntry", QDateTime() ).toString(Qt::ISODate),
            DATETIMEENTRY.toString(Qt::ISODate) );
  QCOMPARE( sc2.readEntry( "dateTimeEntry", QDate() ).toString(Qt::ISODate),
            DATETIMEENTRY.date().toString(Qt::ISODate) );
  QCOMPARE( QVariant(sc2.readEntry( "colorEntry1", QColor(Qt::black) )).toString(),
            QVariant(COLORENTRY1).toString() );
  QCOMPARE( sc2.readEntry( "colorEntry1", QColor()), COLORENTRY1 );
  QCOMPARE( sc2.readEntry( "colorEntry2", QColor() ), COLORENTRY2 );
  QCOMPARE( sc2.readEntry( "colorEntry3", QColor() ), COLORENTRY3 );
  QCOMPARE( sc2.readEntry( "colorEntry4", QColor()), COLORENTRY2 );
  QCOMPARE( sc2.readEntry( "fontEntry", QFont() ), FONTENTRY );
}
