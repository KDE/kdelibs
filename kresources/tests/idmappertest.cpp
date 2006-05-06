/*
    This file is part of libkdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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


#include "idmappertest.h"
#include "idmappertest.moc"
#include "idmapper.h"
#include <qtest_kde.h>

QTEST_KDEMAIN( IdMapperTest, NoGUI )

void IdMapperTest::testIdMapper()
{
  KRES::IdMapper mapper( "test.uidmap" ) ;
  mapper.setIdentifier("testidentifier");

  mapper.setRemoteId( "foo", "bar" );
  mapper.setRemoteId( "yes", "klar" );
  mapper.setRemoteId( "no", "nee" );

  QString mapperAsString = "foo\tbar\t\r\nno\tnee\t\r\nyes\tklar\t\r\n";

  // full
  QCOMPARE( mapper.asString(), mapperAsString );

  mapper.save();

  mapper.clear();
  // empty
  QCOMPARE( mapper.asString(), QString() );

  mapper.load();
  // full again
  QCOMPARE( mapper.asString(), mapperAsString );

  mapper.save();

  mapper.clear();
  // empty
  QCOMPARE( mapper.asString(), QString() );

  mapper.load();
  // full again
  QCOMPARE( mapper.asString(), mapperAsString );
}

