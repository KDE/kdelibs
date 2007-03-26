/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                2000, 2007 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kdedesktopmimetype.h"
#include <kdesktopfile.h>
#include <kmountpoint.h>

QString KDEDesktopMimeType::icon( const KUrl& _url ) const
{
    if ( _url.isEmpty() || !_url.isLocalFile() )
        return KMimeType::iconName( _url );

    KDesktopFile cfg( _url.path() );
    const KConfigGroup group = cfg.desktopGroup();
    const QString icon = group.readEntry( "Icon" );
    const QString type = group.readEntry( "Type" );

    // need to check FSDev for backwards compatibility
    if ( type == "FSDevice" || type == "FSDev")
    {
        const QString unmount_icon = group.readEntry( "UnmountIcon" );
        const QString dev = group.readEntry( "Dev" );
        if ( !icon.isEmpty() && !unmount_icon.isEmpty() && !dev.isEmpty() )
        {
            KMountPoint::Ptr mountPoint = KMountPoint::currentMountPoints().findByDevice(dev);
            if (!mountPoint) // not mounted?
                return unmount_icon;
        }
    } else if ( type == "Link" ) {
        const QString emptyIcon = group.readEntry( "EmptyIcon" );
        if ( !emptyIcon.isEmpty() ) {
            const QString u = group.readPathEntry( "URL" );
            const KUrl url( u );
            if ( url.protocol() == "trash" ) {
                // We need to find if the trash is empty, preferrably without using a KIO job.
                // So instead kio_trash leaves an entry in its config file for us.
                KConfig trashConfig( "trashrc", KConfig::OnlyLocal );
                if ( trashConfig.group("Status").readEntry( "Empty", true ) ) {
                    return emptyIcon;
                }
            }
        }
    }

    if ( icon.isEmpty() )
        return KMimeType::iconName( _url );

    return icon;
}

QString KDEDesktopMimeType::comment( const KUrl& _url ) const
{
    if ( _url.isEmpty() || !_url.isLocalFile() )
        return KMimeType::comment( _url );

    KDesktopFile cfg( _url.path() );
    QString comment = cfg.desktopGroup().readEntry( "Comment" );
    if ( comment.isEmpty() )
        return KMimeType::comment( _url );

    return comment;
}

void KDEDesktopMimeType::virtual_hook( int id, void* data )
{ KMimeType::virtual_hook( id, data ); }
