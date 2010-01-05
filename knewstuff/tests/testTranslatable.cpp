/*
    This file is part of KNewStuff2.
    Copyright (c) 2008 Jeremy Whiting <jpwhiting@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

// unit test for ktranslatable

#include <QtTest>
#include <QtCore>

#include <qtest_kde.h>

#include "../knewstuff2/core/ktranslatable.h"

class testTranslatable: public QObject
{
    Q_OBJECT
private slots:
    void testAddString();
    void testRepresentation();
};

void testTranslatable::testRepresentation()
{
    KNS::KTranslatable translatable("testRepresentation");
    QCOMPARE(translatable.representation(), QString("testRepresentation"));
}

void testTranslatable::testAddString()
{
    KNS::KTranslatable translatable;
    translatable.addString("", "testRepresentation");
    translatable.addString("1", "1");
    QCOMPARE(translatable.representation(), QString("testRepresentation"));
    QCOMPARE(translatable.translated("1"), QString("1"));
}

QTEST_KDEMAIN_CORE(testTranslatable)
#include "testTranslatable.moc"
