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

#include <kdecore_export.h>
#include <QtCore/QString>

/**
 * \class KTempDir ktempdir.h <KTempDir>
 * 
 * @brief Create a unique directory for temporary use.
 *
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
 * KTempDir(KStandardDirs::locateLocal("tmp", prefix));
 *
 * KTempDir does not create any missing directories, but 
 * KStandardDirs::locateLocal() does.
 *
 * @see KStandardDirs
 * @see KTemporaryFile
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
    *        QString() for the default value
    * @param mode the file permissions,
    * almost always in octal. The first digit selects permissions for
    * the user who owns the file: read (4), write (2), and execute
    * (1); the second selects permissions for other users in the
    * file's group, with the same values; and the third for other
    * users not in the file's group, with the same values.
    *
    **/
   explicit KTempDir(const QString& directoryPrefix=QString(),
                     int mode = 0700 );


   /**
    * The destructor deletes the directory and its contents if autoRemove
    * is set to true.
    * @see setAutoRemove.
    **/
   ~KTempDir();

   /**
    * Turn automatic deletion of the directory on or off.
    * Automatic deletion is on by default.
    * @param autoRemove toggle automatic deletion on or off
    **/
   void setAutoRemove(bool autoRemove);

   /**
    * @return whether auto remove is active
    * @see setAutoRemove
    **/
   bool autoRemove() const;

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
    * @return The name of the directory, or QString() if creating the
    *         directory has failed or the directory has been unlinked
    **/
   QString name() const;

   /**
    * Deletes the directory recursively
    **/
   void unlink();

   /**
    * Returns true if a temporary directory has successfully been created
    * and has not been unlinked yet.
    */
   bool exists() const;

   /**
    * @brief Remove a directory and all its contents
    *
    * Remove recursively a directory, even if it is not empty
    * or contains other directories.
    *
    * However the function works too when the @p path given
    * is a non-directory file. In that case it simply remove that file.
    *
    * The function stops on the first error.
    *
    * @note This function is more meant for removing a directory
    * not created by the user. For user-created directories,
    * using KIO::NetAccess::del is recommended instead,
    * especially as it has user feedback for long operations.
    *
    * @param path Path of the directory to delete
    * @return true if successful, otherwise false
    * (Use errno for more details about the error.)
    * @todo decide how and where this function should be defined in KDE4
    */
    static bool removeDir( const QString& path );

protected:

   /**
    * Creates a "random" directory with specified mode
    * @param directoryPrefix to use when creating temp directory
    *       (the rest is generated randomly)
    * @param mode directory permissions
    * @return true upon success
    */
   bool create(const QString &directoryPrefix,  int mode);

private:
    Q_DISABLE_COPY(KTempDir)
    class Private;
    Private * const d;
};

#endif
