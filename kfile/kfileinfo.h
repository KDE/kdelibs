// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KFILEINFO_H
#define KFILEINFO_H

#include <qstring.h>
#include <qdatetime.h>
#include <qlist.h>
#include <time.h>

class QFileInfo;
class KDirEntry;

/**
  * Provides information about a file that has been examined
  * with KDir.
  *
  * @author rich@kde.org
  * @version $Id$
  */
class KFileInfo {

public:
    /**
      * Construct a KFileInfo object from a KDirEntry.
      */
    KFileInfo(const KDirEntry &);

    /**
      * Constructs a KFileInfo object from a QFileInfo.
      * This is only useful for local files
      */
    KFileInfo(const QFileInfo &);

    /**
      * Constructs a "little" KFileInfo (just for local files)
      **/
    KFileInfo(const char *dir, const char *name = "");

    /**
      * Destroy the KFileInfo object.
      */
    ~KFileInfo(){};

    /**
      * Copy a KFileInfo.
      */
    KFileInfo &operator=(const KFileInfo &);

    /**
      * Returns true if this file is a directory.
      */
    bool isDir() const { return myIsDir; }
    bool isFile() const { return myIsFile; }
    bool isSymLink() const { return myIsSymLink; }
    QString absURL() const { return myBaseURL; }
    QString baseURL();
    QString extension();

    /**
      * Returns the name of the file
      *
      * Note: If this object does not refer to a real file
      * (broken symlink), it will return 0
      **/
    const char *fileName() const { return myName; }
    QString filePath();

    /**
      * Returns the group of the file.
      */
    const char *date() const { return myDate; }

    /**
      * Returns the access permissions for the file as a string.
      */
    const char *access() const { return myAccess; }

    /**
      * Returns the owner of the file.
      */
    const char *owner() const { return myOwner; }

    /**
      * Returns the group of the file.
      */
    const char *group() const { return myGroup; }

    /**
      * Returns the size of the file.
      */
    uint size() const { return mySize; }

    /**
      * Returns true if the specified permission flag is set.
      */
    bool permission(uint permissionSpec);

    // overriding QFileInfo's function
    bool isReadable() const ;

    static QString dateTime(time_t secsSince1Jan1970UTC);

protected:
    void parsePermissions(const char *perms);
    void parsePermissions(uint perm);

private:
    QString myName;
    QString myBaseURL;
    QString myAccess;
    QString myDate;
    QString myOwner;
    QString myGroup;
    bool myIsDir;
    bool myIsFile;
    bool myIsSymLink;
    uint myPermissions;
    int mySize;
    bool myIsReadable;
};

typedef QList<KFileInfo> KFileInfoList;
typedef QListIterator<KFileInfo> KFileInfoListIterator;

inline bool KFileInfo::permission(uint permissionSpec)
{
  return ((myPermissions & permissionSpec) != 0);
}


#endif // KFILEINFO_H
