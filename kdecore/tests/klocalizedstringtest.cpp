/*  This file is part of the KDE libraries
    Copyright (C) 2006 Chusslove Illich <caslav.ilic@gmx.net>

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

#include "config.h"

#include "klocalizedstringtest.h"
#include "qtest_kde.h"

#include "kglobal.h"
#include "klocale.h"
#include "klocalizedstring.h"
#include <qstring.h>

#include "klocalizedstringtest.moc"

void KLocalizedStringTest::initTestCase ()
{
    KGlobal::locale()->setLanguage(QString("en_US"));
    KGlobal::locale()->setCountry(QString("C"));
    KGlobal::locale()->setThousandsSeparator(QString(","));
}

void KLocalizedStringTest::correctSubs ()
{
    // Warm up.
    QCOMPARE(i18n("Daisies, daisies"),
             QString("Daisies, daisies"));

    // Placeholder in the middle, start, end.
    QCOMPARE(i18n("Fault in %1 unit", QString("AE35")),
             QString("Fault in AE35 unit"));
    QCOMPARE(i18n("%1, Tycho Magnetic Anomaly 1", QString("TMA-1")),
             QString("TMA-1, Tycho Magnetic Anomaly 1"));
    QCOMPARE(i18n("...odd things happening at %1", QString("Clavius")),
             QString("...odd things happening at Clavius"));

    // Two placeholders.
    QCOMPARE(i18n("%1 and %2", QString("Bowman"), QString("Poole")),
             QString("Bowman and Poole"));
    QCOMPARE(i18n("%28 and %11", QString("Poole"), QString("Bowman")),
             QString("Bowman and Poole"));

    // % which is not of placeholder.
    QCOMPARE(i18n("It's going to go %1% failure in 72 hours.", 100),
             QString("It's going to go 100% failure in 72 hours."));

    // Usual plurals.
    QCOMPARE(i18np("%n pod", "%n pods", 1),
             QString("1 pod"));
    QCOMPARE(i18np("1 pod", "%n pods", 1),
             QString("1 pod"));
    QCOMPARE(i18np("1 pod", "%n pods", 10),
             QString("10 pods"));

    // No %n in singular or plural.
    QCOMPARE(i18np("A pod", "Few pods", 1),
             QString("A pod"));
    QCOMPARE(i18np("A pod", "Few pods", 10),
             QString("Few pods"));
}

void KLocalizedStringTest::wrongSubs ()
{
    #ifndef NDEBUG
    // Too many arguments.
    QVERIFY(i18n("Europa", 1)
            != QString("Europa"));

    // Too few arguments.
    QVERIFY(i18n("%1, %2 and %3", QString("Hunter"), QString("Kimball"))
            != QString("Hunter, Kimball and %3"));

    // Plural argument in wrong position.
    QVERIFY(ki18np("1 pod %1", "%n pods %1").subs("ready").subs(10).toString()
            != QString("10 pods ready"));

    // Plural argument not supplied.
    QVERIFY(ki18np("1 pod", "%n pods").toString()
            != QString("1 pod"));
    QVERIFY(ki18np("1 pod", "%n pods").toString()
            != QString("%n pods"));
    #endif
}

void KLocalizedStringTest::miscMethods ()
{
    KLocalizedString k;
    QVERIFY(k.isEmpty());
}

QTEST_KDEMAIN(KLocalizedStringTest, NoGUI)
