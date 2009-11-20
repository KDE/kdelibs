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

#include "qtest_kde.h"
#include <ktimezonewidget.h>
#include <kconfiggroup.h>
#include <QtDBus/QtDBus>

class KTimeZoneWidgetTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSetSelected()
    {
        if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kded")) {
            QSKIP("kded not running", SkipSingle);
        }

        KTimeZoneWidget tzw;
        QVERIFY(tzw.topLevelItemCount() > 0);
        QVERIFY(tzw.selectedItems().isEmpty());

        // Single selection mode (default)
        QVERIFY(tzw.selectionMode() == KTimeZoneWidget::SingleSelection);
        tzw.setSelected("Europe/Zurich", true);
        QCOMPARE(tzw.selectedItems().count(), 1);
        QCOMPARE(tzw.selection(), QStringList() << "Europe/Zurich");
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
        tzw.setSelected("America/New_York", true);
        QCOMPARE(tzw.selectedItems().count(), 2);
        QCOMPARE(tzw.selection(), QStringList() << "America/New_York" << "Europe/Paris");
    }

    void testCheckableItems()
    {
        if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kded")) {
            QSKIP("kded not running", SkipSingle);
        }

        KTimeZoneWidget tzw;
        tzw.setItemsCheckable(true);
        QVERIFY(tzw.topLevelItemCount() > 0);
        QVERIFY(tzw.selectedItems().isEmpty());
        QVERIFY(tzw.selection().isEmpty());

        // Single selection mode (default)
        QVERIFY(tzw.selectionMode() == KTimeZoneWidget::SingleSelection);
        tzw.setSelected("Europe/Zurich", true);
        QCOMPARE(tzw.selectedItems().count(), 0); // it got checked, not selected
        QCOMPARE(tzw.selection(), QStringList() << "Europe/Zurich");
        tzw.setSelected("Africa/Cairo", true);
        QCOMPARE(tzw.selection(), QStringList() << "Africa/Cairo");

        // Multiple selections explicitly allowed
        tzw.setSelectionMode(KTimeZoneWidget::MultiSelection);
        tzw.clearSelection();
        tzw.setSelected("Europe/Paris", true);
        QCOMPARE(tzw.selection(), QStringList() << "Europe/Paris");
        tzw.setSelected("America/New_York", true);
        QCOMPARE(tzw.selection(), QStringList() << "America/New_York" << "Europe/Paris");
    }
};

// Tricky problem. The kded module writes out a config file, but unit tests have
// a different KDEHOME so they don't see those config files.
// Ideally unit tests should talk to their own kded instance,
// but that means starting a new DBus session bus for all (each?) unit tests, somehow...
//QTEST_KDEMAIN( KTimeZoneWidgetTest, GUI )

int main(int argc, char *argv[])
{
    setenv("LC_ALL", "C", 1);
    // The difference with QTEST_KDEMAIN is here: not setting $KDEHOME
    unsetenv("KDE_COLOR_DEBUG");
    KAboutData aboutData( "ktimezonewidgettest", 0, ki18n("qttest"), "version" );
    KComponentData cData(&aboutData);
    QApplication app( argc, argv );
    app.setApplicationName( "ktimezonewidgettest" );

#if 0
    KTimeZoneWidget tzw;
    tzw.setItemsCheckable(true);
    tzw.setSelectionMode(KTimeZoneWidget::MultiSelection);
    tzw.setSelected("Europe/Paris", true);
    tzw.show();
    return app.exec();
#else
    KTimeZoneWidgetTest tc;
    return QTest::qExec( &tc, argc, argv );
#endif
}

#include "ktimezonewidget_unittest.moc"
