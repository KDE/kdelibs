/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

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
#include "kasciitest.h"
#include "qtest_kde.h"

#include <kascii.h>

QTEST_KDEMAIN_CORE( KAsciiTest )

void KAsciiTest::testkasciistricmp()
{
  QVERIFY( kasciistricmp( "test", "test" ) == 0 );
  QVERIFY( kasciistricmp( "test", "Test" ) == 0 );
  QVERIFY( kasciistricmp( "TeSt", "tEst" ) == 0 );

  QVERIFY( kasciistricmp( 0, 0 ) == 0 );
  QVERIFY( kasciistricmp( "", "" ) == 0 );
  QVERIFY( kasciistricmp( 0, "" ) < 0 );
  QVERIFY( kasciistricmp( "", 0 ) > 0 );

  QVERIFY( kasciistricmp( "", "foo" ) < 0 );
  QVERIFY( kasciistricmp( "foo", "" ) > 0 );

  QVERIFY( kasciistricmp( "test", "testtest" ) < 0 );
  QVERIFY( kasciistricmp( "testtest", "test" ) > 0 );

  QVERIFY( kasciistricmp( "a", "b" ) < 0 );
  QVERIFY( kasciistricmp( "b", "a" ) > 0 );
  QVERIFY( kasciistricmp( "A", "b" ) < 0 );
  QVERIFY( kasciistricmp( "b", "A" ) > 0 );
  QVERIFY( kasciistricmp( "a", "B" ) < 0 );
  QVERIFY( kasciistricmp( "B", "a" ) > 0 );
  QVERIFY( kasciistricmp( "A", "B" ) < 0 );
  QVERIFY( kasciistricmp( "B", "A" ) > 0 );
}

#include "kasciitest.moc"
