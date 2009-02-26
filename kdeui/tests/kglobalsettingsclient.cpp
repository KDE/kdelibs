/* This file is part of the KDE libraries
    Copyright (c) 2006 David Faure <faure@kde.org>

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

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <QtGui/QApplication>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <kcomponentdata.h>

int main(int argc, char **argv)
{
    KAboutData about("kglobalsettingsclient", 0, ki18n("kglobalsettingsclient"), "version");
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    options.add("p", ki18n("emit paletteChanged()"));
    options.add("f", ki18n("emit fontChanged()"));
    options.add("ps", ki18n("emit settingsChanged(SETTINGS_PATH)"));

    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KComponentData componentData(&about); // for KConfig
    QApplication app( KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv(), false );

    if (args->isSet("p")) {
        kDebug() << "emitChange(PaletteChanged)";
        KGlobalSettings::self()->emitChange(KGlobalSettings::PaletteChanged);
	return 0;
    } else if (args->isSet("f")) {
        kDebug() << "emitChange(FontChanged)";
        KGlobalSettings::self()->emitChange(KGlobalSettings::FontChanged);
        return 0;
    } else if (args->isSet("ps")) {
        kDebug() << "emitChange(SettingsChanged)";
        KGlobalSettings::self()->emitChange(KGlobalSettings::SettingsChanged, KGlobalSettings::SETTINGS_PATHS);
        return 0;
    }

    KCmdLineArgs::usage("No action specified");
    return 1; //notreached
}
