/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation, version 2.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kfilespeedbar.h"
#include "config-kfile.h"

#include <qdir.h>

#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kprotocolinfo.h>
#include <kstandarddirs.h>
#include <kurl.h>

KFileSpeedBar::KFileSpeedBar( QWidget *parent, const char *name )
    : KURLBar( true, parent, name )
{
    KConfig *config = KGlobal::config();
    KConfigGroupSaver cs( config, ConfigGroup );
    m_initializeSpeedbar = config->readBoolEntry( "Set speedbar defaults",
                                                   true );

    readConfig( KGlobal::config(), "KFileDialog Speedbar" );

    if ( m_initializeSpeedbar )
    {
        KURL u;
        u.setPath( KGlobalSettings::desktopPath() );
        insertItem( u, i18n("Desktop"), false );

        if (KGlobalSettings::documentPath() != QDir::homeDirPath())
        {
            u.setPath( KGlobalSettings::documentPath() );
            insertItem( u, i18n("Documents"), false, "document" );
        }

        u.setPath( QDir::homeDirPath() );
        insertItem( u, i18n("Home Directory"), false,
                               "folder_home" );
        u = "floppy:/";
        if ( KProtocolInfo::isKnownProtocol( u ) )
            insertItem( u, i18n("Floppy"), false,
                                   KProtocolInfo::icon( "floppy" ) );
        QStringList tmpDirs = KGlobal::dirs()->resourceDirs( "tmp" );
        u.setProtocol( "file" );
        u.setPath( tmpDirs.isEmpty() ? QString("/tmp") : tmpDirs.first() );
        insertItem( u, i18n("Temporary Files"), false,
                               "file_temporary" );
        u = "lan:/";
        if ( KProtocolInfo::isKnownProtocol( u ) )
            insertItem( u, i18n("Network"), false,
                                   "network_local" );
    }
}

KFileSpeedBar::~KFileSpeedBar()
{
}

void KFileSpeedBar::save( KConfig *config )
{
    if ( m_initializeSpeedbar && isModified() )
    {
        KConfigGroup conf( config, ConfigGroup );
        // write to kdeglobals
        conf.writeEntry( "Set speedbar defaults", false, true, true );
    }

    writeConfig( config, "KFileDialog Speedbar" );
}


#include "kfilespeedbar.moc"
