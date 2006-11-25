/* This file is part of the KDE libraries
    Copyright (c) 2006 David Faure <faure@kde.org>

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

#include "qtest_kde.h"
#include "kglobalsettingstest.h"
#include "kglobalsettingstest.moc"

QTEST_KDEMAIN( KGlobalSettingsTest, GUI )

#include <kglobalsettings.h>
#include <kdebug.h>
#include <kprocess.h>
#include <QEventLoop>

/**
 * The strategy of this test is:
 * We install QSignalSpy instances on many signals from KGlobalSettings::self(),
 * and then we get another process (kglobalsettingsclient) to call emitChange(),
 * and we check that the corresponding signals are emitted, i.e. that our process
 * got the dbus signal.
 *
 * As a nice side-effect we automatically test a bit of KProcess as well :)
 */

#define CREATE_ALL_SPYS \
    KGlobalSettings* settings = KGlobalSettings::self(); \
    QSignalSpy palette_spy( settings, SIGNAL(kdisplayPaletteChanged()) ); \
    QSignalSpy font_spy( settings, SIGNAL(kdisplayFontChanged()) ); \
    QSignalSpy style_spy( settings, SIGNAL(kdisplayStyleChanged()) ); \
    QSignalSpy settings_spy( settings, SIGNAL(settingsChanged(int)) ); \
    QSignalSpy appearance_spy( settings, SIGNAL(appearanceChanged()) )

static void callClient( const QString& opt ) {
    KProcess proc;
#ifdef Q_OS_WIN
    proc << "kglobalsettingsclient.exe";
#else
    if (QFile::exists("./kglobalsettingsclient.shell"))
        proc << "./kglobalsettingsclient.shell";
    else {
        QVERIFY(QFile::exists("./kglobalsettingsclient"));
        proc << "./kglobalsettingsclient";
    }
#endif
    proc << opt;
    kDebug() << k_funcinfo << proc.args() << endl;
    bool ok = proc.start( KProcess::Block );
    QVERIFY(ok);

    QEventLoop().processEvents( QEventLoop::AllEvents, 20 ); // need to process DBUS signal
}

void KGlobalSettingsTest::testPaletteChange()
{
    CREATE_ALL_SPYS;
    callClient("-p");
    QCOMPARE(palette_spy.size(), 1);
    QCOMPARE(font_spy.size(), 0);
    QCOMPARE(style_spy.size(), 0);
    QCOMPARE(settings_spy.size(), 0);
    QCOMPARE(appearance_spy.size(), 1);
}

void KGlobalSettingsTest::testFontChange()
{
    CREATE_ALL_SPYS;
    callClient("-f");
    QCOMPARE(palette_spy.size(), 0);
    QCOMPARE(font_spy.size(), 1);
    QCOMPARE(style_spy.size(), 0);
    QCOMPARE(settings_spy.size(), 0);
    QCOMPARE(appearance_spy.size(), 1);
}

void KGlobalSettingsTest::testSettingsChange()
{
    CREATE_ALL_SPYS;
    callClient("--ps");
    QCOMPARE(palette_spy.size(), 0);
    QCOMPARE(font_spy.size(), 0);
    QCOMPARE(style_spy.size(), 0);
    QCOMPARE(settings_spy.size(), 1);
    QCOMPARE(settings_spy.at(0).at(0).toInt(), (int)KGlobalSettings::SETTINGS_PATHS);
    QCOMPARE(appearance_spy.size(), 0);
}
