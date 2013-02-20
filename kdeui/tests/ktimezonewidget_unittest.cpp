/* This file is part of the KDE libraries
   Copyright (C) 2007 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QtTest/QtTest>
#include <QtTestWidgets>
#include <ktimezonewidget.h>
#include <kconfiggroup.h>
#include <QtDBus/QtDBus>
#include "../../kdecore/tests/ktimezonestest_p.h"

class KTimeZoneWidgetTest : public QObject
{
    Q_OBJECT

public:
    void init() { initTestCase(); }

private Q_SLOTS:

    void initTestCase()
    {
        mTestData.setupTimeZoneTest(); // see ktimezonestest_p.h
    }

    void cleanupTestCase()
    {
        mTestData.cleanupTimeZoneTest();
    }

    void testSetSelected()
    {
        if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kded5")) {
            QSKIP("kded5 not running");
        }

        KTimeZoneWidget tzw;
        QVERIFY(tzw.topLevelItemCount() > 0);
        QVERIFY(tzw.selectedItems().isEmpty());

        // Single selection mode (default)
        QVERIFY(tzw.selectionMode() == KTimeZoneWidget::SingleSelection);
        tzw.setSelected("Europe/Paris", true);
        QCOMPARE(tzw.selectedItems().count(), 1);
        QCOMPARE(tzw.selection(), QStringList() << "Europe/Paris");
        tzw.setSelected("Africa/Cairo", true);
        QCOMPARE(tzw.selectedItems().count(), 1);
        QCOMPARE(tzw.selection(), QStringList() << "Africa/Cairo");

        // Multiple selections explicitly allowed
        tzw.setSelectionMode(KTimeZoneWidget::MultiSelection);
        tzw.clearSelection();
        QVERIFY(tzw.selectedItems().isEmpty());
        tzw.setSelected("Europe/Paris", true);
        QCOMPARE(tzw.selectedItems().count(), 1);
        QCOMPARE(tzw.selection(), QStringList() << "Europe/Paris");
        tzw.setSelected("America/Los_Angeles", true);
        QCOMPARE(tzw.selectedItems().count(), 2);
        QCOMPARE(tzw.selection(), QStringList() << "America/Los_Angeles" << "Europe/Paris");
    }

    void testCheckableItems()
    {
        //if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kded5")) {
        //    QSKIP("kded5 not running");
        //}

        KTimeZoneWidget tzw;
        tzw.setItemsCheckable(true);
        QVERIFY(tzw.topLevelItemCount() > 0);
        QVERIFY(tzw.selectedItems().isEmpty());
        QVERIFY(tzw.selection().isEmpty());

        // Single selection mode (default)
        QVERIFY(tzw.selectionMode() == KTimeZoneWidget::SingleSelection);
        tzw.setSelected("Europe/Paris", true);
        QCOMPARE(tzw.selectedItems().count(), 0); // it got checked, not selected
        QCOMPARE(tzw.selection(), QStringList() << "Europe/Paris");
        tzw.setSelected("Africa/Cairo", true);
        QCOMPARE(tzw.selection(), QStringList() << "Africa/Cairo");

        // Multiple selections explicitly allowed
        tzw.setSelectionMode(KTimeZoneWidget::MultiSelection);
        tzw.clearSelection();
        tzw.setSelected("Europe/Paris", true);
        QCOMPARE(tzw.selection(), QStringList() << "Europe/Paris");
        tzw.setSelected("America/Los_Angeles", true);
        QCOMPARE(tzw.selection(), QStringList() << "America/Los_Angeles" << "Europe/Paris");
    }
private:
    TimeZoneTestData mTestData;
};

// Note: no QStandardPaths::enableTestMode(true) here.
// The kded module writes out a config file, but unit tests have
// a different configuration directory so they don't see those config files.
// Ideally unit tests should talk to their own kded instance,
// but that means starting a new DBus session bus for all (each?) unit tests, somehow...

QTEST_MAIN(KTimeZoneWidgetTest)

#include "ktimezonewidget_unittest.moc"
