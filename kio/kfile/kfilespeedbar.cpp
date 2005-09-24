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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
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
    setIconSize(KIcon::SizeSmallMedium);
    readConfig( KGlobal::config(), "KFileDialog Speedbar" );

    if ( m_initializeSpeedbar )
    {
        KURL u;
        u.setPath( KGlobalSettings::desktopPath() );
        insertItem( u, i18n("Desktop"), false );

//TODO: win32
        if ((KGlobalSettings::documentPath() != (QDir::homePath()+"/")) &&
            QDir(KGlobalSettings::documentPath()).exists())
        {
            u.setPath( KGlobalSettings::documentPath() );
            insertItem( u, i18n("Documents"), false, "document" );
        }

        u.setPath( QDir::homePath() );
        insertItem( u, i18n("Home Folder"), false,
                               "folder_home" );

        u = "floppy:/";
        if ( KProtocolInfo::isKnownProtocol( u ) )
            insertItem( u, i18n("Floppy"), false,
                                   KProtocolInfo::icon( "floppy" ) );
        u = "media:/";
        if ( KProtocolInfo::isKnownProtocol( u ) )
            insertItem( u, i18n("Storage Media"), false,
                                   KProtocolInfo::icon( "media" ) );

        u = "remote:/";
        if ( KProtocolInfo::isKnownProtocol( u ) )
            insertItem( u, i18n("Network Folders"), false,
                                   KProtocolInfo::icon( "remote" ) );
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

QSize KFileSpeedBar::sizeHint() const
{
    QSize sizeHint = KURLBar::sizeHint();
    int ems = fontMetrics().width("mmmmmmmmmmmm");
    if (sizeHint.width() < ems)
    {
        sizeHint.setWidth(ems);
    }
    return sizeHint;
}

#include "kfilespeedbar.moc"
