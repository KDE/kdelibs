/*
   $Id$

   This file is part of the KDE libraries
   Copyright (C) 1997 Christoph Neerfeld (chris@kde.org)
             (C) 1999 Stephan Kulow (coolo@kde.org)
             (C) 1999 Kurt Granroth (granroth@kde.org)

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qapplication.h>
#include <qdir.h>
#include <qpainter.h>
#include <qwmatrix.h>
#include <qpixmapcache.h>

#include "kiconloader.h"

#include "kpixmap.h"
#include "klocale.h"
#include "kapp.h"
#include "kconfig.h"
#include "kglobal.h"
#include "kinstance.h"
#include "kstddirs.h"

void KIconLoader::initPath()
{
    iconType = "toolbar";

    if (appname.isNull() && library) {
	appname = library->instanceName();
    }

    if (!appname.isNull()) { // may still be the case

	KStandardDirs* dirs = library->dirs();
	
	dirs->addResourceType("toolbar",
			      KStandardDirs::kde_default("data") +
			      appname + "/pics/");
	
	dirs->addResourceType("toolbar",
			      KStandardDirs::kde_default("data") +
			      appname + "/toolbar/");
    }
}

KIconLoader::KIconLoader( const QString &app_name ) :
    appname(app_name)
{
    library = KGlobal::instance();
    initPath();
}

KIconLoader::KIconLoader( const KInstance* _library )
    : library(_library)
{
    initPath();
}

QPixmap KIconLoader::reloadIcon ( const QString& name )
{
    return loadInternal( name, false );
}

QPixmap KIconLoader::loadIcon ( const QString& name, Size size,
                                QString* path_store, bool canReturnNull )
{
    if (name.at(0) == '/')
        return loadInternal(name);

    QPixmap pix;
    QString icon_path;
    QString path;
    // we prefer the correct size.. but we'll slum with a smaller size
    // if necessary
    Size current_size = size;
    bool not_done = true;
    while ((current_size >= Small) && not_done)
    {
        switch (current_size) {
        case Small:
            path = "small/";
            not_done = false;
            break;
        case Medium:
            path = "medium/";
            current_size = Small;
            break;
        case Large:
            path = "large/";
            current_size = Medium;
            break;
        default:
            path = "medium/";
            current_size = Small;
            break;
        }

        // if noone wants to know the path, we can just lookup the pixmap
        if (!path_store) {
            if (QPixmapCache::find( path + name, pix)) {
                return pix;
            }
        }

        QString icon;
        if (!name.contains('/'))
            icon = "apps/" + name;
        else
            icon = name;

        if (icon.right(4) == ".xpm") {
            icon.truncate(icon.length() - 4);
        }

        if (icon.right(4) == ".png") {
            icon.truncate(icon.length() - 4);
        }

        if ( QPixmap::defaultDepth() > 8 )
        {
          icon_path = locate("icon", path + "hicolor/" + icon + ".png", library );
          if (!icon_path.isEmpty())
            goto loading;

          icon_path = locate("icon", path + "hicolor/" + icon + ".xpm", library );
          if (!icon_path.isEmpty())
            goto loading;
        }

        icon_path = locate("icon", path + "locolor/" + icon + ".png", library );
        if (!icon_path.isEmpty())
            goto loading;

        icon_path = locate("icon", path + "locolor/" + icon + ".xpm", library );
        if (!icon_path.isEmpty())
            goto loading;

    }

    // one last desparate gasp -- we try to locate our icon using the
    // defaults in 'locate'
    path = name;
    if (path.left(8) == "toolbar/")
        path = path.mid(8);
    icon_path = iconPath(path, false);
    if (icon_path.isEmpty())
        return canReturnNull ? QPixmap() : loadInternal("unknown");

 loading:
    pix = loadInternal(icon_path);
    if (path_store)
        *path_store = icon_path;
    else {
        QPixmapCache::insert(path + name, pix);
    }
    return pix;
}


QString KIconLoader::iconPath( const QString& name, bool always_valid)
{
    if (name.at(0) == '/') // we can't do anything with an absolute path than returning
	return name;

    QString full_path;
    if (!name.isEmpty()) {
	QString path = name;
	
	if (path.right(4) == ".xpm") {
	    path.truncate(path.length() - 4);
	}
	full_path = locate(iconType, path + ".png", library);
	if (full_path.isNull())
	    full_path = locate(iconType, path + ".xpm", library );
	
	if (full_path.isNull())
	    full_path = locate(iconType, path, library);
    }
    if (full_path.isNull() && always_valid)
	full_path = locate(iconType, "unknown.png", library);

    return full_path;
}

QPixmap KIconLoader::loadInternal ( const QString& name, bool hcache )
{
    QString cacheKey = "$kico_";
    cacheKey += name;
    KPixmap pix;

    if ( hcache && QPixmapCache::find( cacheKey, pix ) == true ) {
	return pix;
    }

    pix.load( iconPath(name), 0, KPixmap::LowColor );

    if ( pix.isNull() ) {
	return pix;
    }

    QPixmapCache::insert( cacheKey, pix );

    return pix;
}

QPixmap BarIcon(const QString& pixmap , const KInstance* library )
{
    if (pixmap.at(0) == '/')
        return library->iconLoader()->loadIcon(pixmap);
    else
        return library->iconLoader()->loadIcon("toolbar/" + pixmap,
                                               KIconLoader::Medium);
}

QPixmap AppIcon(const QString& pixmap , const KInstance* library )
{
    if (pixmap.at(0) == '/')
        return library->iconLoader()->loadIcon(pixmap);
    else
        return library->iconLoader()->loadIcon("apps/" + pixmap);
}

QPixmap MimeIcon(const QString& pixmap , const KInstance* library )
{
    if (pixmap.at(0) == '/')
        return library->iconLoader()->loadIcon(pixmap);
    else
        return library->iconLoader()->loadIcon("mimetypes/" + pixmap);
}

QPixmap DevIcon(const QString& pixmap , const KInstance* library )
{
    if (pixmap.at(0) == '/')
        return library->iconLoader()->loadIcon(pixmap);
    else
        return library->iconLoader()->loadIcon("devices/" + pixmap);
}

QPixmap FileIcon(const QString& pixmap , const KInstance* library )
{
    if (pixmap.at(0) == '/')
        return library->iconLoader()->loadIcon(pixmap);
    else
        return library->iconLoader()->loadIcon("filesystems/" + pixmap);
}
