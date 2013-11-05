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
#include <k4timezonewidget.h>
#include <ksystemtimezone.h>
#include <kconfiggroup.h>
#include "ktimezonestest_p.h"

class K4TimeZoneWidgetTest : public QObject
{
    Q_OBJECT

public:
    void init() { initTestCase(); }

private Q_SLOTS:

    void initTestCase()
    {
        //mTestData.setupTimeZoneTest(); // see ktimezonestest_p.h
    }

    void cleanupTestCase()
    {
        //mTestData.cleanupTimeZoneTest();
    }

    void testSetSelected()
    {
        if (!KSystemTimeZones::isTimeZoneDaemonAvailable()) {
            QSKIP("ktimezoned not available, check that kded5 is running and /modules/ktimezoned is available");
        }

        K4TimeZoneWidget tzw;
        QVERIFY(tzw.topLevelItemCount() > 0);
        QVERIFY(tzw.selectedItems().isEmpty());

        // Single selection mode (default)
        QVERIFY(tzw.selectionMode() == K4TimeZoneWidget::SingleSelection);
        tzw.setSelected("Europe/Paris", true);
        QCOMPARE(tzw.selectedItems().count(), 1);
        QCOMPARE(tzw.selection(), QStringList() << "Europe/Paris");
        tzw.setSelected("Africa/Cairo", true);
        QCOMPARE(tzw.selectedItems().count(), 1);
        QCOMPARE(tzw.selection(), QStringList() << "Africa/Cairo");

        // Multiple selections explicitly allowed
        tzw.setSelectionMode(K4TimeZoneWidget::MultiSelection);
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
        if (!KSystemTimeZones::isTimeZoneDaemonAvailable()) {
            QSKIP("ktimezoned not available, check that kded5 is running and /modules/ktimezoned is available");
        }

        K4TimeZoneWidget tzw;
        tzw.setItemsCheckable(true);
        QVERIFY(tzw.topLevelItemCount() > 0);
        QVERIFY(tzw.selectedItems().isEmpty());
        QVERIFY(tzw.selection().isEmpty());

        // Single selection mode (default)
        QVERIFY(tzw.selectionMode() == K4TimeZoneWidget::SingleSelection);
        tzw.setSelected("Europe/Paris", true);
        QCOMPARE(tzw.selectedItems().count(), 0); // it got checked, not selected
        QCOMPARE(tzw.selection(), QStringList() << "Europe/Paris");
        tzw.setSelected("Africa/Cairo", true);
        QCOMPARE(tzw.selection(), QStringList() << "Africa/Cairo");

        // Multiple selections explicitly allowed
        tzw.setSelectionMode(K4TimeZoneWidget::MultiSelection);
        tzw.clearSelection();
        tzw.setSelected("Europe/Paris", true);
        QCOMPARE(tzw.selection(), QStringList() << "Europe/Paris");
        tzw.setSelected("America/Los_Angeles", true);
        QCOMPARE(tzw.selection(), QStringList() << "America/Los_Angeles" << "Europe/Paris");
    }
private:
    // Because we don't use a separate KDEHOME, we can't use TimeZoneTestData.
    // It would remove ktimezonedrc from the user!
    //TimeZoneTestData mTestData;
};

// Note: no QStandardPaths::enableTestMode(true) here.
// The kded module writes out a config file, but unit tests have
// a different configuration directory so they don't see those config files.
// Ideally unit tests should talk to their own kded instance,
// but that means starting a new DBus session bus for all (each?) unit tests, somehow...

QTEST_MAIN(K4TimeZoneWidgetTest)

#include "k4timezonewidget_unittest.moc"
