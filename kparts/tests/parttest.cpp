/*
    Copyright (c) 2007 David Faure <faure@kde.org>

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
#include "parttest.h"
#include "qtest_kde.h"

#include <kparts/part.h>
#include <QWidget>

QTEST_KDEMAIN( PartTest, GUI )

class TestPart : public KParts::Part
{
public:
    TestPart(QObject* parent, QWidget* parentWidget)
        : KParts::Part(parent) {
        setWidget(new QWidget(parentWidget));
    };
};

void PartTest::testAutoDeletePart()
{
    KParts::Part* part = new TestPart(0, 0);
    QPointer<KParts::Part> partPointer(part);
    delete part->widget();
    QVERIFY(partPointer.isNull());
}

void PartTest::testAutoDeleteWidget()
{
    KParts::Part* part = new TestPart(0, 0);
    QPointer<KParts::Part> partPointer(part);
    QPointer<QWidget> widgetPointer(part->widget());
    delete part;
    QVERIFY(widgetPointer.isNull());
}

void PartTest::testNoAutoDeletePart()
{
    KParts::Part* part = new TestPart(0, 0);
    part->setAutoDeletePart(false);
    QPointer<KParts::Part> partPointer(part);
    delete part->widget();
    QVERIFY(part->widget() == 0);
    QCOMPARE(static_cast<KParts::Part*>(partPointer), part);
    delete part;
}

void PartTest::testNoAutoDeleteWidget()
{
    KParts::Part* part = new TestPart(0, 0);
    part->setAutoDeleteWidget(false);
    QWidget* widget = part->widget();
    QVERIFY(widget);
    QPointer<QWidget> widgetPointer(part->widget());
    delete part;
    QCOMPARE(static_cast<QWidget*>(widgetPointer), widget);
    delete widget;
}

#include "parttest.moc"
