/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#include "kglobalsettings.h"

#include <qdir.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>

QString* KGlobalSettings::s_desktopPath = 0;
QString* KGlobalSettings::s_autostartPath = 0;
QString* KGlobalSettings::s_trashPath = 0;

int KGlobalSettings::dndEventDelay()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, "General" );
    return c->readNumEntry("DndDelay", 2);
}

bool KGlobalSettings::singleClick()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, "KDE" );
    return c->readBoolEntry("SingleClick", KDE_DEFAULT_SINGLECLICK);
}

bool KGlobalSettings::insertTearOffHandle()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, "KDE" );
    return c->readBoolEntry("InsertTearOffHandle", KDE_DEFAULT_INSERTTEAROFFHANDLES);
}

bool KGlobalSettings::changeCursorOverIcon()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, "KDE" );
    return c->readBoolEntry("ChangeCursor", KDE_DEFAULT_CHANGECURSOR);
}

int KGlobalSettings::autoSelectDelay()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, "KDE" );
    return c->readNumEntry("AutoSelectDelay", KDE_DEFAULT_AUTOSELECTDELAY);
}

KGlobalSettings::Completion KGlobalSettings::completionMode()
{
    int completion;
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, "General" );
    completion = c->readNumEntry("completionMode", -1);
    if ((completion < (int) CompletionNone) ||
	(completion > (int) CompletionShell))
      {
	completion = (int) CompletionShell; // Default
      }
  return (Completion) completion;
}

bool KGlobalSettings::honorGnome()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, "General" );
    return c->readBoolEntry("honorGnome", false);
}

void KGlobalSettings::initStatic()
{
    if ( s_desktopPath != 0 )
	return;

    s_desktopPath = new QString();
    s_autostartPath = new QString();
    s_trashPath = new QString();

    KConfig *config = KGlobal::config();
    bool dollarExpansion = config->isDollarExpansion();
    config->setDollarExpansion(true);
    KConfigGroupSaver cgs( config, "Paths" );

    // Desktop Path
    *s_desktopPath = QDir::homeDirPath() + "/Desktop/";
    *s_desktopPath = config->readEntry( "Desktop", *s_desktopPath);
    if ( (*s_desktopPath)[0] != '/' )
      s_desktopPath->prepend( QDir::homeDirPath() + "/" );
    *s_desktopPath = QDir::cleanDirPath( *s_desktopPath );
    if ( s_desktopPath->right(1) != "/")
	*s_desktopPath += "/";

    // Trash Path
    *s_trashPath = *s_desktopPath + "Trash/";
    *s_trashPath = config->readEntry( "Trash" , *s_trashPath);
    *s_trashPath = QDir::cleanDirPath( *s_trashPath );
    if ( s_trashPath->right(1) != "/")
	*s_trashPath += "/";

    // Autostart Path
    *s_autostartPath = KGlobal::dirs()->localkdedir() + "Autostart/";
    *s_autostartPath = config->readEntry( "Autostart" , *s_autostartPath);
    if ( (*s_autostartPath)[0] != '/' )
      s_autostartPath->prepend( QDir::homeDirPath() + "/" );
    *s_autostartPath = QDir::cleanDirPath( *s_autostartPath );
    if ( s_autostartPath->right(1) != "/")
	*s_autostartPath += "/";

    config->setDollarExpansion(dollarExpansion);
}

QColor KGlobalSettings::toolBarHighlightColor()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("Toolbar style") );
    return c->readColorEntry("HighlightColor", &Qt::blue);
}
