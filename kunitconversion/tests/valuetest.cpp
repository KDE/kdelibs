/*
 *   Copyright (C) 2009 Petri Damstén <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "valuetest.h"

using namespace KUnitConversion;

void ValueTest::initTestCase()
{
    v1 = Value(3.1415, Kilometer);
    v2 = Value(6.1415, "m");
    v3 = Value(9.1415, v1.unit());
}

void ValueTest::testStrings()
{
    QCOMPARE(v1.unit()->symbol(), QString("km"));
    QCOMPARE(v2.toSymbolString(), QString("6.1415 m"));
    QCOMPARE(v3.toString(), QString("9.1415 kilometers"));
}

void ValueTest::testRound()
{
    v1.round(2);
    QCOMPARE(v1.number(), 3.14);
}

void ValueTest::testConvert()
{
    v1 = v1.convertTo(Meter);
    QCOMPARE(v1.number(), 3140.0);
    v1 = v1.convertTo("cm");
    QCOMPARE(v1.number(), 314000.0);
    v1 = v1.convertTo(v2.unit());
    QCOMPARE(v1.number(), 3140.0);
}

void ValueTest::testInvalid()
{
    v1 = v1.convertTo(99999);
    QCOMPARE(v1.number(), 0.0);
    QCOMPARE(v1.toSymbolString(), QString(""));
    v2 = v2.convertTo("don't exist");
    QCOMPARE(v2.number(), 0.0);
    QCOMPARE(v2.toSymbolString(), QString(""));
}

QTEST_MAIN(ValueTest)

