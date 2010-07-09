// -*- c++ -*-
/*
 * This file is part of the KDE libraries
 *
 * Copyright (c) 2008 David Jarvie <djarvie@kde.org>
 * Copyright (c) 2006-2007 Christian Ehrlicher <Ch.Ehrlicher@gmx.de>
 * Copyright (c) 2000-2007 Stephan Kulow <coolo@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#include <kcmdlineargs.h>
#include <klocale.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <qdir.h>
#include <stdio.h>
#include <kaboutdata.h>
#include <kdeversion.h>
#include <QDir>
#include <QtCore/QLibraryInfo>
#include <QtCore/QCoreApplication>
#include <QProcess>
#include <config.h>
#include <config-prefix.h>
#include <kconfiggroup.h>
#include <kkernel_win.h>
#include <kde_file.h>

static void printResult(const QString &s)
{
    if (s.isEmpty())
        printf("\n");
	else {
		QString path = QDir::toNativeSeparators( s );
        printf("%s\n", path.toLocal8Bit().constData());
	}
}

static QString readXdg( const char* type )
{
    QProcess proc;
    proc.start( "xdg-user-dir", QStringList() << type );
    if (!proc.waitForStarted() || !proc.waitForFinished())
        return QString();
    return QString::fromLocal8Bit( proc.readAll()).trimmed();
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    KAboutData about("kde4-config", "kdelibs4", ki18n("kde4-config"), "1.0",
                     ki18n("A little program to output installation paths"),
                     KAboutData::License_GPL,
                     ki18n("(C) 2000 Stephan Kulow"));
    KCmdLineArgs::init( argc, argv, &about);

    KCmdLineOptions options;
    options.add("expandvars",  ki18n("Left for legacy support"));
    options.add("prefix",      ki18n("Compiled in prefix for KDE libraries"));
    options.add("exec-prefix", ki18n("Compiled in exec_prefix for KDE libraries"));
    options.add("libsuffix",   ki18n("Compiled in library path suffix"));
    options.add("localprefix", ki18n("Prefix in $HOME used to write files"));
    options.add("kde-version", ki18n("Compiled in version string for KDE libraries"));
    options.add("types",       ki18n("Available KDE resource types"));
    options.add("path type",       ki18n("Search path for resource type"));
    options.add("locate filename", ki18n("Find filename inside the resource type given to --path"));
    options.add("userpath type",   ki18n("User path: desktop|autostart|document"));
    options.add("install type",    ki18n("Prefix to install resource files to"));
    options.add("qt-prefix",   ki18n("Installation prefix for Qt"));
    options.add("qt-binaries", ki18n("Location of installed Qt binaries"));
    options.add("qt-libraries", ki18n("Location of installed Qt libraries"));
    options.add("qt-plugins", ki18n("Location of installed Qt plugins"));
    KCmdLineArgs::addCmdLineOptions( options ); // Add my own options.

    KComponentData a(&about);
    (void)KGlobal::dirs(); // trigger the creation
    (void)KGlobal::config();

    // Get application specific arguments
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->isSet("prefix"))
    {
        printResult(KDEDIR);
        return 0;
    }

    if (args->isSet("exec-prefix"))
    {
        printResult(EXEC_INSTALL_PREFIX);
        return 0;
    }

    if (args->isSet("libsuffix"))
    {
        QString tmp(KDELIBSUFF);
        tmp.remove('"');
        printResult(tmp.toLocal8Bit());
        return 0;
    }

    if (args->isSet("localprefix"))
    {
        printResult(KGlobal::dirs()->localkdedir());
        return 0;
    }

    if (args->isSet("kde-version"))
    {
        printf("%s\n", KDE_VERSION_STRING);
        return 0;
    }

    if (args->isSet("types"))
    {
        QStringList types = KGlobal::dirs()->allTypes();
        types.sort();
        const char *helptexts[] = {
            "apps", I18N_NOOP("Applications menu (.desktop files)"),
            "autostart", I18N_NOOP("Autostart directories"),
            "cache", I18N_NOOP("Cached information (e.g. favicons, web-pages)"),
            "cgi", I18N_NOOP("CGIs to run from kdehelp"),
            "config", I18N_NOOP("Configuration files"),
            "data", I18N_NOOP("Where applications store data"),
            "emoticons", I18N_NOOP("Emoticons"),
            "exe", I18N_NOOP("Executables in $prefix/bin"),
            "html", I18N_NOOP("HTML documentation"),
            "icon", I18N_NOOP("Icons"),
            "kcfg", I18N_NOOP("Configuration description files"),
            "lib", I18N_NOOP("Libraries"),
            "include", I18N_NOOP("Includes/Headers"),
            "locale", I18N_NOOP("Translation files for KLocale"),
            "mime", I18N_NOOP("Mime types"),
            "module", I18N_NOOP("Loadable modules"),
            "pixmap", I18N_NOOP("Legacy pixmaps"),
            "qtplugins", I18N_NOOP("Qt plugins"),
            "services", I18N_NOOP("Services"),
            "servicetypes", I18N_NOOP("Service types"),
            "sound", I18N_NOOP("Application sounds"),
            "templates", I18N_NOOP("Templates"),
            "wallpaper", I18N_NOOP("Wallpapers"),
            "xdgdata-apps", I18N_NOOP("XDG Application menu (.desktop files)"),
            "xdgdata-dirs", I18N_NOOP("XDG Menu descriptions (.directory files)"),
            "xdgdata-icon", I18N_NOOP("XDG Icons"),
            "xdgdata-pixmap", I18N_NOOP("Legacy pixmaps"),
            "xdgdata-mime", I18N_NOOP("XDG Mime Types"),
            "xdgconf-menu", I18N_NOOP("XDG Menu layout (.menu files)"),
            "xdgconf-autostart", I18N_NOOP("XDG autostart directory"),
            "tmp", I18N_NOOP("Temporary files (specific for both current host and current user)"),
            "socket", I18N_NOOP("UNIX Sockets (specific for both current host and current user)"),
            0, 0
        };
        Q_FOREACH(const QString &type, types)
        {
            int index = 0;
            while (helptexts[index] && type != helptexts[index]) {
                index += 2;
            }
            if (helptexts[index]) {
                printf("%s - %s\n", helptexts[index], i18n(helptexts[index+1]).toLocal8Bit().data());
            } else {
                printf("%s", i18n("%1 - unknown type\n", type).toLocal8Bit().data());
            }
        }
        return 0;
    }

    QString type = args->getOption("path");
    if (!type.isEmpty())
    {
        QString fileName = args->getOption("locate");
        if (!fileName.isEmpty())
        {
            QString result = KStandardDirs::locate(type.toLatin1(), fileName);
            if (!result.isEmpty())
                printf("%s\n", result.toLocal8Bit().constData());
            return result.isEmpty() ? 1 : 0;
        }

        printResult(KGlobal::dirs()->resourceDirs(type.toLatin1()).join(QString(KPATH_SEPARATOR)));
        return 0;
    }

    type = args->getOption("userpath");
    if (!type.isEmpty())
    {
        //code duplicated with KGlobalSettings::initPath()
        if ( type == "desktop" )
        { // QDesktopServices is QtGui :-/
            QString path = readXdg( "DESKTOP" );
            if (path.isEmpty())
                path = QDir::homePath() + QLatin1String("/Desktop");
            path=QDir::cleanPath( path );
            if ( !path.endsWith('/') )
              path.append(QLatin1Char('/'));
            printResult(path);
        }
        else if ( type == "autostart" )
        {
            KConfigGroup g( KGlobal::config(), "Paths" );
            QString path=QDir::homePath() + "/Autostart/";
            path=g.readPathEntry( "Autostart", path);
            path=QDir::cleanPath( path );
            if ( !path.endsWith('/') )
              path.append(QLatin1Char('/'));
            printResult(path);

        }
        else if ( type == "document" )
        {
            QString path = readXdg( "DOCUMENTS" );
            if ( path.isEmpty())
                path = QDir::homePath() + QLatin1String("/Documents");
            path=QDir::cleanPath( path );
            if ( !path.endsWith('/') )
              path.append(QLatin1Char('/'));
            printResult(path);
        }
        else
            fprintf(stderr, "%s", i18n("%1 - unknown type of userpath\n", type).toLocal8Bit().data() );
        return 0;
    }

    type = args->getOption("install");
    if (!type.isEmpty())
    {
        printResult( KGlobal::dirs()->installPath(type.toLocal8Bit()) );
    }

    if (args->isSet("qt-prefix"))
    {
        printResult(QLibraryInfo::location(QLibraryInfo::PrefixPath));
        return 0;
    }
    if (args->isSet("qt-binaries"))
    {
        printResult(QLibraryInfo::location(QLibraryInfo::BinariesPath));
        return 0;
    }
    if (args->isSet("qt-libraries"))
    {
        printResult(QLibraryInfo::location(QLibraryInfo::LibrariesPath));
        return 0;
    }
    if (args->isSet("qt-plugins"))
    {
        printResult(QLibraryInfo::location(QLibraryInfo::PluginsPath));
        return 0;
    }
    return 0;
}
