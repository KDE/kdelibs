/* This file is part of the KDE project
   Copyright (c) 2001 David Faure <david@mandrakesoft.com>
   Copyright (c) 2001 Laurent Montel <lmontel@mandrakesoft.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef kfileshare_h
#define kfileshare_h
#include <qobject.h>

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
 */
class KFileShare
{
public:
    /**
     * Called on application startup, to initialize authorization and shareList
     * Can also be called later, when the configuration changes
     */
    static void readConfig();

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

    static bool setShared( const QString& path, bool shared );

private:
    static Authorization s_authorization;
    static QStringList* s_shareList;
};

#endif
