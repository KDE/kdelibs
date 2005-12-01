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

#include <QtTest/qttest_kde.h>
#include "kconfigtest.h"
#include "kconfigtest.moc"
#include <qrect.h>

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
#define LOCAL8BITENTRY "Hello הצü"
#define POINTENTRY QPoint( 4351, 1235 )
#define SIZEENTRY QSize( 10, 20 )
#define RECTENTRY QRect( 10, 23, 5321, 13 )
#define DATETIMEENTRY QDateTime( QDate( 2002, 06, 23 ), QTime( 12, 55, 40 ) )
#define STRINGLISTENTRY QStringList( "Hello," )

void KConfigTest::writeConfigFile()
{
  KConfig sc( "kconfigtest" );

  sc.setGroup("AAA");
  sc.writeEntry("stringEntry1", STRINGENTRY1, true, true);
  sc.deleteEntry("stringEntry2", false, true);
  sc.writeEntry("byteArrayEntry1", QByteArray( STRINGENTRY1 ), true, true);

  sc.setGroup("Hello");
  sc.writeEntry( "boolEntry1", BOOLENTRY1 );
  sc.writeEntry( "boolEntry2", BOOLENTRY2 );

  sc.writeEntry( "Test", QString::fromLocal8Bit( LOCAL8BITENTRY ) );
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

  sc.setGroup("Bye");
  sc.writeEntry( "rectEntry", RECTENTRY );
  sc.writeEntry( "pointEntry", POINTENTRY );
  sc.writeEntry( "sizeEntry", SIZEENTRY );
  sc.writeEntry( "dateTimeEntry", DATETIMEENTRY );
  sc.writeEntry( "stringListEntry", STRINGLISTENTRY );
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

  VERIFY( !bImmutable );
  //qWarning("sc3.entryIsImmutable() 1: %s", bImmutable ? "true" : "false");

  sc2.setGroup("AAA");
  VERIFY( sc2.hasKey( "stringEntry1" ) );
  COMPARE( sc2.readEntry( "stringEntry1" ), QString( STRINGENTRY1 ) );
  COMPARE( sc2.entryIsImmutable("stringEntry1"), bImmutable );
  VERIFY( !sc2.hasKey( "stringEntry2" ) );
  COMPARE( sc2.readEntry( "stringEntry2", "bla" ), QString( "bla" ) );
  COMPARE( sc2.readEntry( "byteArrayEntry1" ).toLatin1(), QByteArray( STRINGENTRY1 ) );

  VERIFY( !sc2.hasDefault( "stringEntry1" ) );

  sc2.setGroup("Hello");
  COMPARE( sc2.readEntry( "Test" ), QString::fromLocal8Bit( LOCAL8BITENTRY ) );
  COMPARE( sc2.readEntry("Test2", "Fietsbel").isEmpty(), true );
  COMPARE( sc2.readEntry( "stringEntry1" ), QString( STRINGENTRY1 ) );
  COMPARE( sc2.readEntry( "stringEntry2" ), QString( STRINGENTRY2 ) );
  COMPARE( sc2.readEntry( "stringEntry3" ), QString( STRINGENTRY3 ) );
  COMPARE( sc2.readEntry( "stringEntry4" ), QString( STRINGENTRY4 ) );
  VERIFY( !sc2.hasKey( "stringEntry5" ) );
  COMPARE( sc2.readEntry( "stringEntry5", "test" ), QString( "test" ) );
  VERIFY( !sc2.hasKey( "stringEntry6" ) );
  COMPARE( sc2.readEntry( "stringEntry6", "foo" ), QString( "foo" ) );
  COMPARE( sc2.readBoolEntry( "boolEntry1" ), BOOLENTRY1 );
  COMPARE( sc2.readBoolEntry( "boolEntry2" ), BOOLENTRY2 );

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

  sc2.setGroup("Bye");

  COMPARE( sc2.readPointEntry( "pointEntry" ), POINTENTRY );
  COMPARE( sc2.readSizeEntry( "sizeEntry" ), SIZEENTRY);
  COMPARE( sc2.readRectEntry( "rectEntry" ), RECTENTRY );
  COMPARE( sc2.readDateTimeEntry( "dateTimeEntry" ).toString(), DATETIMEENTRY.toString() );
  COMPARE( sc2.readListEntry( "stringListEntry").join( "," ), STRINGLISTENTRY.join( "," ) );
}
