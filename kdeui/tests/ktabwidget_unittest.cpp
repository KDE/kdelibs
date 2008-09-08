/* This file is part of the KDE libraries

    Copyright (c) 2008 David Faure <faure@kde.org>

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

#include <qtest_kde.h>
#include <qtestevent.h>
#include <ktabwidget.h>

class KTabWidget_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    // Test calling tabText() from slotCurrentChanged().
    // Because QTabBar::insertTab() emits currentChanged before
    // QTabWidget calls tabInserted(), we were getting an assert in tabText().
    void testTabTextFromSlotCurrentChanged()
    {
        KTabWidget w;
        w.setAutomaticResizeTabs(true);
        const QString firstTitle = "First post!";
        connect(&w, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChanged(int)));
        const int zero = w.insertTab(0, new QWidget, firstTitle); // calls slotCurrentChanged
        QCOMPARE(zero, 0);
        QCOMPARE(w.tabText(0), firstTitle);
    }

private Q_SLOTS:
    void slotCurrentChanged(int index)
    {
        QCOMPARE(index, 0);
        KTabWidget* w = qobject_cast<KTabWidget *>(sender());
        QVERIFY(w);
        QCOMPARE(w->tabText(0), QString("First post!"));
    }
};

QTEST_KDEMAIN(KTabWidget_UnitTest, GUI)

#include "ktabwidget_unittest.moc"
