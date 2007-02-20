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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
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
#include <kconfiggroup.h>

KFileSpeedBar::KFileSpeedBar( QWidget *parent )
    : KUrlBar( true, parent )
{
    KConfigGroup cg( KGlobal::config(), ConfigGroup );
    m_initializeSpeedbar = cg.readEntry( "Set speedbar defaults", true );
    setIconSize(K3Icon::SizeMedium);
    readConfig( KGlobal::config()->group( "KFileDialog Speedbar" ));

    if ( m_initializeSpeedbar )
    {
        KUrl u;
        KUrlBarItem* item = 0;

        u.setPath( KGlobalSettings::desktopPath() );
        item = insertItem( u, i18n("Desktop"), false );
        item->setToolTip( i18n("Files and folders found on your desktop") );

//TODO: win32
        if ((KGlobalSettings::documentPath() != (QDir::homePath()+'/')) &&
            QDir(KGlobalSettings::documentPath()).exists())
        {
            u.setPath( KGlobalSettings::documentPath() );
            item = insertItem( u, i18n("Documents"), false, "document" );
            item->setToolTip( i18n("Your personal documents folder") );
        }

        u.setPath( QDir::homePath() );
        item = insertItem( u, i18n("Home Folder"), false,
                               "folder_home" );
        item->setToolTip( i18n("Your personal home folder for your files and settings") );

#ifdef __GNUC__
#warning "KDE4 - Can we move the Floppy item into 'Storage Media' or lose it entirely?"
#endif

        u = "floppy:/";
        if ( KProtocolInfo::isKnownProtocol( u ) ) {
            item = insertItem( u, i18n("Floppy"), false,
                                   KProtocolInfo::icon( "floppy" ) );
            item->setToolTip( i18n("Files and folders on floppy disks") );
        }

        u = "media:/";
        if ( KProtocolInfo::isKnownProtocol( u ) ) {
            item = insertItem( u, i18n("Storage Media"), false,
                                   KProtocolInfo::icon( "media" ) );
        
            item->setToolTip( i18n("Removable storage devices and media") );
        }

        u = "remote:/";
        if ( KProtocolInfo::isKnownProtocol( u ) ) {
            item = insertItem( u, i18n("Network Folders"), false,
                                   KProtocolInfo::icon( "remote" ) );

            item->setToolTip( i18n("Files and folders on network locations.") );
        }
    }
}

KFileSpeedBar::~KFileSpeedBar()
{
}

void KFileSpeedBar::save(KSharedConfigPtr config)
{
    if ( m_initializeSpeedbar && isModified() )
    {
        KConfigGroup conf( config, ConfigGroup );
        // write to kdeglobals
        conf.writeEntry( "Set speedbar defaults", false ,
                         KConfigBase::Persistent|KConfigBase::Global );
    }

    KConfigGroup sb = config->group("KFileDialog Speedbar");
    writeConfig(sb);
}

#include "kfilespeedbar.moc"
