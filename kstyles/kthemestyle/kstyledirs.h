/*
 This file is part of the KDE libraries
 (c) 2002 Maksim Orlovich <mo002j@mail.rochester.edu>,

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

#ifndef KSTYLE_DIRS_H
#define KSTYLE_DIRS_H


#include <qsettings.h>
#include <qstringlist.h>
#include <kstandarddirs.h>

/**
* @short Access to the standard KDE directories for the pixmap style
* @author Maksim Orlovich<mo002j@mail.rochester.edu> is responsible for this file,
    but all the interesting work is done by KStandardDirs
*
* This class provides a this wrapper for styles around KStandardDirs,
* permitting integration with QSettings and easy loading of pixmaps
*
* It add share/apps/kstyle/themes as "themerc",
*    share/apps/kstyle/pixmaps "themepixmap"
*/
class KStyleDirs: public KStandardDirs
{
public:
    static KStyleDirs* dirs()
    {
        if ( !instance)
            instance = new KStyleDirs;
        return instance;
    }

    static void release()
    {
        delete instance;
        instance = 0;
    }

    /**
    Adds all of KDE directories of type type to the seach path of q.

    For example, when one does the following:
    QSettings settings;
    KStyleDirs dirs;
    dirs.addToSearch("config",settings);

    The one can do settings.readEntry("kstyle/KDE/WidgetStyle") to access a settings in kstylerc.
    */
    void addToSearch( const char* type, QSettings& q) const; //Better name?

protected:
    static KStyleDirs* instance;
    /**
    Creates an instance of the class, and calculates the path information.
    */
    KStyleDirs();
    KStyleDirs(const KStyleDirs&);
    KStyleDirs& operator= (const KStyleDirs&);

    virtual ~KStyleDirs();
};

#endif
