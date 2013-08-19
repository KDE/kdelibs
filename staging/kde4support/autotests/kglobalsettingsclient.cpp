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

//#include <kaboutdata.h>
#include <QApplication>
#include <kglobalsettings.h>
#include <qdebug.h>

int main(int argc, char **argv)
{
    //KCmdLineOptions options;
    //options.add("p", qi18n("emit paletteChanged()"));
    //options.add("f", qi18n("emit fontChanged()"));
    //options.add("ps", qi18n("emit settingsChanged(SETTINGS_PATH)"));

    QApplication app(argc, argv, false);

    QByteArray opt = argc > 1 ? argv[1] : "";
    if (opt == "-p") {
        qDebug() << "emitChange(PaletteChanged)";
        KGlobalSettings::self()->emitChange(KGlobalSettings::PaletteChanged);
	return 0;
    } else if (opt == "-f") {
        qDebug() << "emitChange(FontChanged)";
        KGlobalSettings::self()->emitChange(KGlobalSettings::FontChanged);
        return 0;
    } else if (opt == "--ps") {
        qDebug() << "emitChange(SettingsChanged)";
        KGlobalSettings::self()->emitChange(KGlobalSettings::SettingsChanged, KGlobalSettings::SETTINGS_PATHS);
        return 0;
    }

    qWarning() << "No action specified";
    return 1;
}
