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

#include "categorytest.h"
#include <kunitconversion/unitcategory.h>

using namespace KUnitConversion;

void CategoryTest::initTestCase()
{
}

void CategoryTest::testInfo()
{
    UnitCategory* cg = c.category(AreaCategory);
    QCOMPARE(cg->name(), QString("Area"));
    QCOMPARE(cg->description(), QString(""));
    QCOMPARE(cg->url(), QUrl());
    QCOMPARE(cg->id(), (int)AreaCategory);
}

void CategoryTest::testUnits()
{
    UnitCategory* cg = c.category(MassCategory);
    QCOMPARE(cg->defaultUnit()->symbol(), QString("kg"));
    QCOMPARE(cg->hasUnit(QString("g")), true);
    QCOMPARE(cg->unit(QString("g"))->symbol(), QString("g"));
    QCOMPARE(cg->unit(Kilogram)->symbol(), QString("kg"));
    QVERIFY(cg->units().size() > 0);
    QVERIFY(cg->allUnits().size() > 0);
}

void CategoryTest::testConvert()
{
    UnitCategory* cg = c.category(LengthCategory);
    Value v = cg->convert(Value(3.14, Kilometer), "m");
    QCOMPARE(v.number(), 3140.0);
    v = cg->convert(v, "cm");
    QCOMPARE(v.number(), 314000.0);
    v = cg->convert(v, cg->defaultUnit());
    QCOMPARE(v.number(), 3140.0);
}

void CategoryTest::testInvalid()
{
    UnitCategory* cg = c.category(99999);
    QCOMPARE(cg->name(), QString("Invalid"));
    cg = c.category("don't exist");
    QCOMPARE(cg->name(), QString("Invalid"));
}

QTEST_MAIN(CategoryTest)

