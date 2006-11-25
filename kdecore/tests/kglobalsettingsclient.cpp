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

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <QApplication>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <kinstance.h>

KCmdLineOptions options[] =
{
    { "p", "emit paletteChanged()", 0 },
    { "f", "emit fontChanged()", 0 },
    { "ps", "emit settingsChanged(SETTINGS_PATH)", 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KAboutData about("kglobalsettingsclient", "kglobalsettingsclient", "version");
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KInstance instance(&about); // for KConfig
    QApplication app( *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv(), false );

    if (args->isSet("p")) {
        kDebug() << k_funcinfo << "emitChange(PaletteChanged)" << endl;
        KGlobalSettings::self()->emitChange(KGlobalSettings::PaletteChanged);
	return 0;
    } else if (args->isSet("f")) {
        kDebug() << k_funcinfo << "emitChange(FontChanged)" << endl;
        KGlobalSettings::self()->emitChange(KGlobalSettings::FontChanged);
        return 0;
    } else if (args->isSet("ps")) {
        kDebug() << k_funcinfo << "emitChange(SettingsChanged)" << endl;
        KGlobalSettings::self()->emitChange(KGlobalSettings::SettingsChanged, KGlobalSettings::SETTINGS_PATHS);
        return 0;
    }

    KCmdLineArgs::usage("No action specified");
    return 1; //notreached
}
