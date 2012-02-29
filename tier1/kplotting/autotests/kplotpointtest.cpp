/*
    Copyright (C) 2012 Benjamin Port <benjamin.port@ben2367.fr>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include <kplotpoint.h>

#include <QtTest>

class KPlotPointTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testConstructor()
    {
        KPlotPoint *p1 = new KPlotPoint(2.0, 3.0, QString::fromLatin1("label"), 5.0);
        KPlotPoint *p2 = new KPlotPoint(QPointF(2.0, 3.0), QString::fromLatin1("label"), 5.0);

        QCOMPARE(p1->x(), 2.0);
        QCOMPARE(p2->x(), 2.0);

        QCOMPARE(p1->y(), 3.0);
        QCOMPARE(p2->y(), 3.0);

        QCOMPARE(p1->barWidth(), 5.0);
        QCOMPARE(p2->barWidth(), 5.0);

        QCOMPARE(p1->label(), QString::fromLatin1("label"));
        QCOMPARE(p2->label(), QString::fromLatin1("label"));

        delete p1;
        delete p2;
    }

    void testPosition()
    {
        KPlotPoint *p1 = new KPlotPoint(2.0, 3.0, QString::fromLatin1("label"), 5.0);

        p1->setX(4.0);
        QCOMPARE(p1->x(), 4.0);

        p1->setY(6.0);
        QCOMPARE(p1->y(), 6.0);

        QCOMPARE(p1->position(), QPointF(4.0, 6.0));

        p1->setPosition(QPointF(1.0, 7.0));
        QCOMPARE(p1->position(), QPointF(1.0, 7.0));
        QCOMPARE(p1->x(), 1.0);
        QCOMPARE(p1->y(), 7.0);

        delete p1;
    }

    void testLabel()
    {
        KPlotPoint *p1 = new KPlotPoint(2.0, 3.0, QString::fromLatin1("label"), 5.0);

        p1->setLabel(QString::fromLatin1("newLabel"));
        QCOMPARE(p1->label(), QString::fromLatin1("newLabel"));

        delete p1;
    }

    void testBarWidth()
    {
        KPlotPoint *p1 = new KPlotPoint(2.0, 3.0, QString::fromLatin1("label"), 5.0);

        p1->setBarWidth(5.0);
        QCOMPARE(p1->barWidth(), 5.0);

        delete p1;
    }
};

QTEST_MAIN(KPlotPointTest)

#include "kplotpointtest.moc"
