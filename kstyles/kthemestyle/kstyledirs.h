/* 
   $Id$

   This file is part of the KDE libraries
   Adopted from the KStandardDirs code by
   		Maksim Orlovich <mo002j@mail.rochester.edu>,

   KStandardDirs are Copyright:
	(C) 1999 Sirtaj Singh Kang <taj@kde.org>
	(C) 1999 Stephan Kulow <coolo@kde.org>
	(C) 1999 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __KSTYLE_DIRS_H
#define __KSTYLE_DIRS_H

#include <qsettings.h>
#include <qstringlist.h>

class KStyleDirsPrivate;

/**
* @short Access to the standard KDE directories for the pixmap style, which can't use kdecore
* @author Maksim Orlovich<mo002j@mail.rochester.edu> is responsible for this file, but the significant bits are by Sirtaj Singh Kang, Stephan Kulow, and Waldo Bastian
* @version $Id$
*
* This class provides a small subset of the KStandardDirs functionality that is just enough for a style to locate its
* themerc file, and to load its pixmap or other resources. It knows nothing about standard
* resource location, does not caching, and so on.
*/
class KStyleDirs
{
public:
    /**
    Creates an instance of the class, and calculates the path information.
    */
    KStyleDirs();
    virtual ~KStyleDirs();

    /**
    Returns a QStringList with all the directories KDE files may be in.
    */
    const QStringList& enumerateDirs() const;

    /**
    Adds all of KDE directories with the apnd suffix appended to the search path of the QSettings object q.

    For example, when one does the following:
    QSettings settings;
    KStyleDirs dirs;
    dirs.addToSearch(settings);

    The one can do settings.readEntry("kstyle/KDE/WidgetStyle") to access a settings in kstylerc.
    At this point no guarantees are made about search order of the various rc files.
    */
    void addToSearch(QSettings& q, QString apnd="share/config/") const; //Better name?

    /*
    Finds a file with the given name, which is under the given path under any one of the valid KDE roots..

    Example: QString pixmap_path = dirs.locate("share/apps/kstyle/pixmaps","acqua-graphite/button.png");

    An empty string is returned if nothing is found.		
    */
    QString locate(const QString& path, const QString& file) const;
protected:
    KStyleDirsPrivate* d;
};

#endif
