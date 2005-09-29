/* This file is part of the KDE project
   Copyright (c) 2001 David Faure <david@mandrakesoft.com>
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
#include <qobject.h>

#include <kdelibs_export.h>

class KDirWatch;

/**
 * @internal
 * Do not use, ever.
 */
class KFileSharePrivate : public QObject
{
  Q_OBJECT

public:
  KFileSharePrivate();
  ~KFileSharePrivate();
  KDirWatch* m_watchFile;
  static KFileSharePrivate *self();
  static KFileSharePrivate *_self;
protected slots: // this is why this class needs to be in the .h
 void slotFileChange(const QString &);
};

/**
 * Common functionality for the file sharing
 * (communication with the backend)
 * @since 3.1
 */
class KIO_EXPORT KFileShare
{

  
public:
    /**
     * Reads the file share configuration file
     */
    static void readConfig();
    
    /**
     * Reads the list of shared folders
     */
    static void readShareList();


    /**
     * Call this to know if a directory is currently shared
     */
    static bool isDirectoryShared( const QString& path );

    enum Authorization { NotInitialized, ErrorNotFound, Authorized, UserNotAllowed };
    /**
     * Call this to know if the current user is authorized to share directories
     */
    static Authorization authorization();

    static QString findExe( const char* exeName );

    /**
     * Uses a suid perl script to share the given path 
     * with NFS and Samba
     * @param path the path to share
     * @param shared whether the path should be shared or not
     * @returns whether the perl script was successful
     */
    static bool setShared( const QString& path, bool shared );
    
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
    static bool sharingEnabled();
    
    /**
     * Returns whether file sharing is restricted.
     * If it is not restricted every user can shar files.
     * If it is restricted only users in the configured
     * file share group can share files.
     */
    static bool isRestricted();
    
    /**
     * Returns the group that is used for file sharing.
     * That is, all users in that group are allowed to
     * share files if file sharing is restricted.
     */
    static QString fileShareGroup();
    
    /**
     * Returns the configured share mode
     */
    static ShareMode shareMode();
    
    /**
     * Returns whether Samba is enabled
     */
    static bool sambaEnabled();
    
    /** 
     * Returns whether NFS is enabled
     */
    static bool nfsEnabled();

private:
    static Authorization s_authorization;
    static QStringList* s_shareList;
    static ShareMode s_shareMode;
    static bool s_sambaEnabled;
    static bool s_nfsEnabled;
    static bool s_restricted;
    static QString s_fileShareGroup;
    static bool s_sharingEnabled;
    
};

#endif
