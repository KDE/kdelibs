/*
 Simple helper routines for style's use of KStandardDirs with QSettings, etc.

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


 This file is part of the KDE libraries
*/

#include <qfile.h>
#include <kstandarddirs.h>
#include "kstyledirs.h"

KStyleDirs* KStyleDirs::instance = 0;

KStyleDirs::KStyleDirs()
{
    addResourceType( "themepixmap", KStandardDirs::kde_default( "data" ) + "kstyle/pixmaps/" );
    addResourceType( "themerc", KStandardDirs::kde_default( "data" ) + "kstyle/themes/" );
}

KStyleDirs::~KStyleDirs()
{
}

void KStyleDirs::addToSearch( const char* type, QSettings& s ) const
{
    const QStringList & dirs = resourceDirs(type);
    for ( int c = dirs.size()-1; c >= 0 ; c-- )
    {
        s.insertSearchPath( QSettings::Unix, dirs[ c ]);
    }
}

