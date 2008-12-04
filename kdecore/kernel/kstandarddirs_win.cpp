/* This file is part of the KDE libraries
   Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>
   Copyright (C) 2008 Christian Ehrlicher <ch.ehrlicher@gmx.de>

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

#include <config-prefix.h>
#include <config.h>
#include "kkernel_win.h"

QString getKde4Prefix();

inline QString prefix()        { return getKde4Prefix(); }
inline QString share()         { return getKde4Prefix() + QLatin1String("share/"); }
inline QString kde_moduledir() { return getKde4Prefix() + QLatin1String("lib/kde4/"); }

QString KStandardDirs::installPath(const char *type)
{
    Q_ASSERT(type != NULL);

    switch (type[0]) {
        case 'a':
            if (strcmp("apps", type) == 0)
                return share() + QLatin1String("applnk/");
            break;
        case 'c':
            if (strcmp("config", type) == 0)
                return share() + QLatin1String("config/");
            break;
        case 'k':
            if (strcmp("kcfg", type) == 0)
                return share() + QLatin1String("config.kcfg/");
            if (strcmp("kdedir", type) == 0)
                return prefix();
            break;
        case 'd':
            if (strcmp("data", type) == 0)
                return share() + QLatin1String("apps/");
            break;
        case 'e':
            if (strcmp("exe", type) == 0)
                return prefix() + QLatin1String("bin/");
            break;
        case 'h':
            if (strcmp("html", type) == 0)
                return share() + QLatin1String("doc/HTML/");
            break;
        case 'i':
            if (strcmp("icon", type) == 0)
                return share() + QLatin1String("icons/");
            if (strcmp("include", type) == 0)
                return prefix() + "include";
            break;
        case 'l':
            if (strcmp("lib", type) == 0)
                return prefix() + QString::fromLatin1("lib" KDELIBSUFF "/");
            if (strcmp("libexec", type) == 0)
                return prefix() + QString::fromLatin1("lib" KDELIBSUFF "/kde4/libexec/");
            if (strcmp("locale", type) == 0)
                return share() + QLatin1String("locale/");
            break;
        case 'm':
            if (strcmp("module", type) == 0)
                return kde_moduledir();
            if (strcmp("mime", type) == 0)
                return share() + QLatin1String("mimelnk/");
            break;
        case 'q':
            if (strcmp("qtplugins", type) == 0)
                return kde_moduledir() + QLatin1String("plugins/");
            break;
        case 's':
            if (strcmp("services", type) == 0)
                return share() + QLatin1String("kde4/services/");
            if (strcmp("servicetypes", type) == 0)
                return share() + QLatin1String("kde4/servicetypes/");
            if (strcmp("sound", type) == 0)
                return share() + QLatin1String("sounds/");
            break;
        case 't':
            if (strcmp("templates", type) == 0)
                return share() + QLatin1String("templates/");
            break;
        case 'w':
            if (strcmp("wallpaper", type) == 0)
                return share() + QLatin1String("wallpapers/");
            break;
        case 'x':
            if (strcmp("xdgconf-menu", type) == 0)
                return share() + QLatin1String("xdg/menus/");
            if (strcmp("xdgdata-apps", type) == 0)
                return share() + QLatin1String("applications/kde4/");
            if (strcmp("xdgdata-dirs", type) == 0)
                return share() + QLatin1String("desktop-directories/");
            break;
    }
    return QString();
}
