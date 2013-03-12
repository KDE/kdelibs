/*  This file is part of the KDE project
    Copyright (C) 2010 David Faure <faure@kde.org>

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

// Emulate being compiled in debug mode, so that kDebug(0) defaults to "on"
#undef QT_NO_DEBUG

// TODO: a separate test program with #define QT_NO_DEBUG...

#include <QtCore/QCoreApplication>
#include <QtCore/QtGlobal>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <qstandardpaths.h>
#include <kcomponentdata.h>
#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kcrash.h>

// This test is actually called by kdebug_unittest
// in order to see how kDebug behaves when there is no KComponentData.

// Disable drkonqi, to avoid warning about it not being found (which breaks kdebug_unittest)
void disableDrKonqi()
{
    KCrash::setDrKonqiEnabled(false);
}
Q_CONSTRUCTOR_FUNCTION(disableDrKonqi)

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QStandardPaths::enableTestMode(true);
    {
        KConfig config("kdebugrc");
        config.group(QString()).writeEntry("DisableAll", false); // in case of a global kdebugrc with DisableAll=true
    }

    // Test kDebug before and after KComponentData gets created

    // Register dynamic area (e.g. from oxygen style)
    const int myArea = KDebug::registerArea("qcoreapp_myarea");
    kDebug(myArea) << "Test debug using qcoreapp_myarea" << myArea;

    // Another dynamic area, but it was turned off (well, to file) by kdebug_unittest
    const int myFileArea = KDebug::registerArea("myarea");
    kDebug(myFileArea) << "Test logging to file debug using myarea";

    // Test that KDebug works
    kDebug(264) << "Debug in area 264, off by default, no output";
    kDebug(100) << "Debug in area 100"; // unknown area, will use area 0 instead
    // This should appear, but disabling kdebug_qcoreapptest in kdebugdialog should make it go away.";
    kDebug() << "Simple debug";

    if (KComponentData::hasMainComponent()) { abort(); return 2; }

    KConfig config("kdebugrc");
    Q_ASSERT(!config.hasGroup(QString::number(myArea)));
    // only true when called by kdebug_unnittest
    //Q_ASSERT(config.hasGroup("myarea"));
    //Q_ASSERT(config.group("myarea").readEntry("InfoOutput", 2) == 0);

    if (KComponentData::hasMainComponent()) { abort(); return 3; }

    // Test what happens when creating a main component data _now_
    // In KF5: nothing.
    KComponentData mainData("kdebug_qcoreapptest_mainData");
    if (!KComponentData::hasMainComponent()) { abort(); return 4; }
    kDebug() << "This should appear, under the kdebug_qcoreapptest area";
    kDebug(264) << "Debug in area 264, still off by default";
    kDebug(100) << "Debug in area 100"; // unknown area, will use area 0 instead

    return 0;
}

