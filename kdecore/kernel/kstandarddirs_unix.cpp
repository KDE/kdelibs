/* This file is part of the KDE libraries
   Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>

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
#include <QFile>

#ifdef Q_OS_WIN
QString getKde4Prefix();
#endif

QString KStandardDirs::installPath(const char *type)
{
    Q_ASSERT(type != NULL);

    switch (type[0]) {
        case 'a':
            if (strcmp("apps", type) == 0)
                return QFile::decodeName(APPLNK_INSTALL_DIR "/");
            break;
        case 'c':
            if (strcmp("config", type) == 0)
                return QFile::decodeName(CONFIG_INSTALL_DIR "/");
            break;
        case 'k':
            if (strcmp("kcfg", type) == 0)
                return QFile::decodeName(KCFG_INSTALL_DIR "/");
            if (strcmp("kdedir", type) == 0)
#ifdef Q_OS_WIN
                return getKde4Prefix();
#else
                return QFile::decodeName(KDEDIR "/");
#endif
            break;
        case 'd':
            if (strcmp("data", type) == 0)
                return QFile::decodeName(DATA_INSTALL_DIR "/");
            break;
        case 'e':
            if (strcmp("exe", type) == 0)
                return QFile::decodeName(BIN_INSTALL_DIR "/");
            break;
        case 'h':
            if (strcmp("html", type) == 0)
                return QFile::decodeName(HTML_INSTALL_DIR "/");
            break;
        case 'i':
            if (strcmp("icon", type) == 0)
                return QFile::decodeName(ICON_INSTALL_DIR "/");
            if (strcmp("include", type) == 0)
                return QFile::decodeName(INCLUDE_INSTALL_DIR "/");
            break;
        case 'l':
            if (strcmp("lib", type) == 0)
                return QFile::decodeName(LIB_INSTALL_DIR "/");
            if (strcmp("libexec", type) == 0)
                return QFile::decodeName(KDEDIR "/lib" KDELIBSUFF "/kde4/libexec/");
            if (strcmp("locale", type) == 0)
                return QFile::decodeName(LOCALE_INSTALL_DIR "/");
            break;
        case 'm':
            if (strcmp("module", type) == 0)
                return QFile::decodeName(PLUGIN_INSTALL_DIR "/");
            if (strcmp("mime", type) == 0)
                return QFile::decodeName(MIME_INSTALL_DIR "/");
            break;
        case 'q':
            if (strcmp("qtplugins", type) == 0)
                return QFile::decodeName(PLUGIN_INSTALL_DIR "/plugins/");
            break;
        case 's':
            if (strcmp("services", type) == 0)
                return QFile::decodeName(SERVICES_INSTALL_DIR "/");
            if (strcmp("servicetypes", type) == 0)
                return QFile::decodeName(SERVICETYPES_INSTALL_DIR "/");
            if (strcmp("sound", type) == 0)
                return QFile::decodeName(SOUND_INSTALL_DIR "/");
            break;
        case 't':
            if (strcmp("templates", type) == 0)
                return QFile::decodeName(TEMPLATES_INSTALL_DIR "/");
            break;
        case 'w':
            if (strcmp("wallpaper", type) == 0)
                return QFile::decodeName(WALLPAPER_INSTALL_DIR "/");
            break;
        case 'x':
            if (strcmp("xdgconf-menu", type) == 0)
                return QFile::decodeName(SYSCONF_INSTALL_DIR "/xdg/menus/");
            if (strcmp("xdgdata-apps", type) == 0)
                return QFile::decodeName(XDG_APPS_INSTALL_DIR "/");
            if (strcmp("xdgdata-dirs", type) == 0)
                return QFile::decodeName(XDG_DIRECTORY_INSTALL_DIR "/");
            break;
    }
    return QString();
}
