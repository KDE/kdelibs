/* This file is part of the KDE libraries
    Copyright (c) 2005 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kstandarddirs.h"
#include <kdebug.h>

#include <qtest_kde.h>

#include "kautostarttest.h"
#include "kautostarttest.moc"

#include <QFile>

QTEST_KDEMAIN( KAutostartTest, NoGUI )

#include <kautostart.h>

void KAutostartTest::testStartDetection_data()
{
    QTest::addColumn<QString>("service");
    QTest::addColumn<bool>("doesAutostart");
    QTest::newRow("kicker") << "panel" << true;
    QTest::newRow("khotkeys") << "khotkeys" << false;
    QTest::newRow("does not exist") << "doesnotexist" << false;
}

void KAutostartTest::testStartDetection()
{
    QFETCH(QString, service);
    QFETCH(bool, doesAutostart);

    KAutostart autostart(service);
    QCOMPARE(autostart.autostarts(), doesAutostart);
}

void KAutostartTest::testStartInEnvDetection_data()
{
    QTest::addColumn<QString>("env");
    QTest::addColumn<bool>("doesAutostart");
    QTest::newRow("kde") << "KDE" << true;
    QTest::newRow("xfce") << "XFCE" << false;
}

void KAutostartTest::testStartInEnvDetection()
{
    QFETCH(QString, env);
    QFETCH(bool, doesAutostart);

    KAutostart autostart("panel");
    QCOMPARE(autostart.autostarts(env), doesAutostart);
}

void KAutostartTest::testStartphase_data()
{
    QTest::addColumn<QString>("service");
    QTest::addColumn<int>("startPhase");
    QTest::newRow("kicker") << "panel" << (int)KAutostart::BaseDesktop;
    QTest::newRow("klipper") << "klipper" << (int)KAutostart::DesktopServices;
    QTest::newRow("khotkeys") << "ktip" << (int)KAutostart::Applications;
    QTest::newRow("does not exist") << "doesnotexist"
                                    << (int)KAutostart::Applications;
}

void KAutostartTest::testStartphase()
{
    QFETCH(QString, service);
    QFETCH(int, startPhase);

    QEXPECT_FAIL("kicker",
                 "Waiting for Lubos to finish the new autostart levels",
                 Continue);

    QEXPECT_FAIL("klipper",
                 "Waiting for Lubos to finish the new autostart levels",
                 Continue);

    KAutostart autostart(service);
    QCOMPARE((int)autostart.startPhase(), startPhase);
}

void KAutostartTest::testStartName()
{
    KAutostart autostart("panel");
    QCOMPARE(autostart.name(), QString("KDE Panel"));
}

void KAutostartTest::testServiceRegistered()
{
    KAutostart autostart;
    QCOMPARE(KAutostart::serviceRegistered("panel"), true);
    QCOMPARE(KAutostart::serviceRegistered("doesnotexist"), false);
}

void KAutostartTest::testRegisteringAndManipulatingANewService()
{
    QFile::remove(locateLocal("autostart", "doesnotexist.desktop"));
    {
        // need to clean up the KAutostart object before QFile can remove it
        KAutostart autostart("doesnotexist");
        QCOMPARE(autostart.autostarts(), false);
        autostart.setCommand("aseigo");
        autostart.setCommandToCheck("/bin/ls");
        autostart.setName("doesnotexisttest");
        autostart.setStartPhase(KAutostart::BaseDesktop);
        autostart.setAllowedEnvironments(QStringList("KDE"));
        autostart.addToAllowedEnvironments("XFCE");
        autostart.addToAllowedEnvironments("GNOME");
        autostart.removeFromAllowedEnvironments("GNOME");
        autostart.setExcludedEnvironments(QStringList("GNOME"));
        autostart.addToExcludedEnvironments("XFCE");
        autostart.addToExcludedEnvironments("KDE");
        autostart.removeFromExcludedEnvironments("KDE");
    }

    {
        QStringList allowedEnvs;
        allowedEnvs << "KDE" << "XFCE";

        QStringList excludedEnvs;
        excludedEnvs << "GNOME" << "XFCE";

        KAutostart autostart("doesnotexist");
        QCOMPARE(autostart.command(), QString("aseigo"));
        QCOMPARE(autostart.autostarts(), true);
        QCOMPARE(autostart.autostarts("KDE"), true);
        QCOMPARE(autostart.autostarts("GNOME"), false);
        QCOMPARE(autostart.autostarts("XFCE"), true);
        QCOMPARE(autostart.autostarts("XFCE", KAutostart::CheckCommand), true);
        QCOMPARE(autostart.name(), QString("doesnotexisttest"));
        QCOMPARE(autostart.commandToCheck(), QString("/bin/ls"));
        QCOMPARE((int)autostart.startPhase(), (int)KAutostart::BaseDesktop);
        QCOMPARE(autostart.allowedEnvironments(), allowedEnvs);
        QCOMPARE(autostart.excludedEnvironments(), excludedEnvs);

        autostart.setCommandToCheck("/bin/whozitwhat");
    }

    {
        KAutostart autostart("doesnotexist");
        QCOMPARE(autostart.autostarts("KDE", KAutostart::CheckCommand), false);
    }
}

void KAutostartTest::testRemovalOfNewServiceFile()
{
    QCOMPARE(QFile::remove(locateLocal("autostart", "doesnotexist.desktop")), true);
}
