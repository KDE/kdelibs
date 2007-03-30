/* This file is part of the KDE project
   Copyright (c) 2001 David Faure <faure@kde.org>
   Copyright (c) 2001 Laurent Montel <lmontel@mandrakesoft.com>

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

#ifndef kfileshare_h
#define kfileshare_h

#include <kio/kio_export.h>
class QString;

/**
 * Common functionality for the file sharing
 * (communication with the backend)
 */
namespace KFileShare
{
    /**
     * Reads the file share configuration file
     */
    KIO_EXPORT void readConfig();

    /**
     * Reads the list of shared folders
     */
    KIO_EXPORT void readShareList();


    /**
     * Call this to know if a directory is currently shared
     */
    KIO_EXPORT bool isDirectoryShared( const QString& path );

    enum Authorization { NotInitialized, ErrorNotFound, Authorized, UserNotAllowed };
    /**
     * Call this to know if the current user is authorized to share directories
     */
    KIO_EXPORT Authorization authorization();

    /**
     * Uses a suid perl script to share the given path 
     * with NFS and Samba
     * @param path the path to share
     * @param shared whether the path should be shared or not
     * @returns whether the perl script was successful
     */
    KIO_EXPORT bool setShared( const QString& path, bool shared );

    /**
     * The used share mode.
     * Simple means that the simple sharing dialog is used and
     * users can share only folders from there HOME folder.
     * Advanced means that the advanced sharing dialog is used and
     * users can share any folder.
     */
    enum ShareMode { Simple, Advanced };

    /**
     * Returns whether sharing is enabled
     * If this is false, file sharing is disabled and
     * nobody can share files.
     */
    KIO_EXPORT bool sharingEnabled();

    /**
     * Returns whether file sharing is restricted.
     * If it is not restricted every user can shar files.
     * If it is restricted only users in the configured
     * file share group can share files.
     */
    KIO_EXPORT bool isRestricted();

    /**
     * Returns the group that is used for file sharing.
     * That is, all users in that group are allowed to
     * share files if file sharing is restricted.
     */
    KIO_EXPORT QString fileShareGroup();

    /**
     * Returns the configured share mode
     */
    KIO_EXPORT ShareMode shareMode();

    /**
     * Returns whether Samba is enabled
     */
    KIO_EXPORT bool sambaEnabled();

    /** 
     * Returns whether NFS is enabled
     */
    KIO_EXPORT bool nfsEnabled();
}

#endif
