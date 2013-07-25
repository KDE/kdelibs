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

#ifndef VALUE_TEST_H
#define VALUE_TEST_H

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include <../converter.h>

using namespace KUnitConversion;

class ValueTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testStrings();
    void testRound();
    void testConvert();
    void testInvalid();

private:
    Value v1;
    Value v2;
    Value v3;
};

#endif
