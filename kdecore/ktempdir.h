/*
   This file is part of the KDE libraries
   Copyright (c) 2003 Joseph Wenninger <jowenn@kde.org>

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

#ifndef KTEMPDIR_H
#define KTEMPDIR_H

#include "kdelibs_export.h"

class QString;
class QDir;

/**
 * The KTempDir class creates a unique directory for temporary use.
 *
 * This is especially useful if you need to create a directory in a world
 * writable directory like /tmp without being vulnerable to so called
 * symlink attacks.
 *
 * KDE applications, however, shouldn't create files or directories in /tmp
 * in the first place but use the "tmp" resource instead. The standard
 * KTempDir constructor will do that by default.
 *
 * To create a temporary directory that starts with a certain name
 * in the "tmp" resource, one should use:
 * KTempDir(locateLocal("tmp", prefix));
 *
 * KTempFile does not create any missing directories, but locateLocal() does.
 *
 * @see KStandardDirs
 *
 * @since 3.2
 * @author Joseph Wenninger <jowenn@kde.org>
 */
class KDECORE_EXPORT KTempDir
{
public:
   /**
    * Creates a temporary directory with the name:
    *  \p \<directoryPrefix\>\<six letters\>
    *
    * The default \p directoryPrefix is "$KDEHOME/tmp-$HOST/appname"
    * @param directoryPrefix the prefix of the file name, or
    *        QString::null for the default value
    * @param mode the file permissions,
    * almost always in octal. The first digit selects permissions for
    * the user who owns the file: read (4), write (2), and execute
    * (1); the second selects permissions for other users in the
    * file's group, with the same values; and the fourth for other
    * users not in the file's group, with the same values.
    *
    **/
   KTempDir(QString directoryPrefix=QString::null,
             int mode = 0700 );


   /**
    * The destructor deletes the directory and it's contents if autoDelete
    * is set to true.
    * @see setAutoDelete.
    **/
   ~KTempDir();

   /**
    * Turn automatic deletion on or off.
    * Automatic deletion is off by default.
    * @param autoDelete true to turn automatic deletion on
    **/
   void setAutoDelete(bool autoDelete);

   /**
    * Returns the status of the directory creation  based on errno.
    * (see errno.h)
    *
    * @note You should check the status after object creation to check
    * whether the directory could be created.
    *
    * @return the errno status, 0 means ok
    **/
   int status() const;

   /**
    * Returns the full path and name of the directory, including a
    * trailing '/'.
    * @return The name of the directory, or QString::null if creating the
    *         directory has failed or the directory has been unlinked
    **/
   QString name() const;


   /**
    * Returns the QDir* of the temporary directory.
    * @return QDir directory information of the directory or 0 if there is
    *         no managed directory
    * @note The caller has to free the pointer returned.
    **/
   QDir *qDir();

   /**
    * Deletes the directory recursively
    **/
   void unlink();

   /**
    * Returns true if a temporary directory has successfully been created
    * and has not been unlinked yet.
    */
   bool exists() const;

protected:

   /**
    * Creates a "random" directory with specified mode
    * @param directoryPrefix to use when creating temp directory
    *       (the rest is generated randomly)
    * @param mode directory permissions
    * @return true upon sucess
    */
   bool create(const QString &directoryPrefix,  int mode);

   /**
    * Sets the errno value
    * @param error the value to set the status to.
    */
   void setError(int error);

private:
   class Private;
   Private *const d;
};

#endif
