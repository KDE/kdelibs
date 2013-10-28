/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
/*
   This file is part of the KDE libraries
   Copyright 1999 Waldo Bastian <bastian@kde.org>
   Copyright 2006 Jaison Lee <lee.jaison@gmail.com>

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

#ifndef KSAVEFILE_H
#define KSAVEFILE_H

#include <kde4support_export.h>

#include "kbackup.h"

#include <QtCore/QFile>
#include <QtCore/QString>

/**
 * \class KSaveFile ksavefile.h <KSaveFile>
 *
 * @brief Class to allow for atomic file I/O, as well as utility functions.
 *
 * The KSaveFile class has been made to write out changes to an existing
 * file atomically. This means that either <b>ALL</b> changes will be written
 * to the file, or <b>NO</b> changes have been written, and the original file
 * (if any) has been unchanged. This is useful if you have lots of 
 * time-consuming processing to perform during which an interruption could
 * occur, or if any error in the file structure will cause the entire file
 * to be corrupt.
 *
 * When you create a KSaveFile for a given file, a temporary file is instead
 * created and all your I/O occurs in the save file. Once you call finalize()
 * the temporary file is renamed to the target file, so that all your changes
 * happen at once. If abort() is called then the temporary file is removed and
 * the target file is untouched. KSaveFile derives from QFile so you can use
 * it just as you would a normal QFile.
 *
 * This class also includes several static utility functions available that
 * can help ensure data integrity. See the individual functions for details.
 *
 * Here is a quick example of how to use KSaveFile:
 *
 * First we create the KSaveFile and open it.
 *
 * @code
 * KSaveFile saveFile;
 * saveFile.setFileName("/lib/foo/bar.dat");
 * if ( !saveFile.open() ) {
 *     //Handle error
 * }
 * @endcode
 *
 * At this point the file "/lib/foo/bar.dat" has not been altered in any way.
 * Now, let's write out some data to the file.
 *
 * @code
 * QTextStream stream ( &saveFile );
 * stream << "Add some data.";
 * // Perform long processing
 * stream << "Add some more data.";
 * stream.flush();
 * @endcode
 *
 * Even after writing this data, the target file "/lib/foo/bar.dat" still has
 * not been altered in any way. Now that we are done writing our data, we can
 * write out all the changes that we have made by calling finalize().
 *
 * @code
 * if ( !saveFile.finalize() ) {
 *     //Handle error
 * }
 * @endcode
 *
 * If a user interruption or error occurred while we were writing out our
 * changes, we would instead call abort() to cancel all the I/O without
 * affecting the target file.
 *
 * @see QFile
 *
 * @author Jaison Lee <lee.jaison@gmail.com>
 * @author Waldo Bastian <bastian@kde.org>
 *
 * @deprecated since 5.0, port to QSaveFile.
 */
class KDE4SUPPORT_DEPRECATED_EXPORT KSaveFile : public QFile
{
public:
    /**
     * Default constructor.
     */
    KSaveFile();

    /**
     * Creates a new KSaveFile and sets the target file to @p filename.
     *
     * @param filename the path of the file
     */
    explicit KSaveFile(const QString &filename);

    /**
     * Destructor.
     * @note If the file has been opened but not yet finalized, the
     * destructor will call finalize(). If you do not want the target file
     * to be affected you need to call abort() before destroying the object.
     **/
    virtual ~KSaveFile();

    /**
     * @brief Set the target filename for the save file.
     * You must use this to set the filename of the target file if you do
     * not use the contructor that does so.
     * @param filename Name of the target file.
     */
    void setFileName(const QString &filename);

    /**
     * @brief Returns the name of the target file.
     * This function returns the name of the target file, or an empty
     * QString if it has not yet been set.
     * @returns The name of the target file.
     */
    QString fileName() const;

    /**
     * @brief Returns the last error that occurred.
     * Use this function to check for errors.
     * @returns The last error that occurred, or QFile::NoError.
     */
    QFile::FileError error() const;

    /**
     * @brief Returns a human-readable description of the last error.
     * Use this function to get a human-readable description of the
     * last error that occurred.
     * @return A string describing the last error that occurred.
     */
    QString errorString() const;

    /**
     * @brief Open the save file.
     * This function will open the save file by creating a temporary file to write
     * to. It will also check to ensure that there are sufficient permissions to
     * write to the target file.
     *
     * @param flags Sets the QIODevice::OpenMode. It should contain the write flag, otherwise you
     * have a save file you cannot save to.
     *
     * @return true if successful, or false if an error has occurred.
     */
    virtual bool open(OpenMode flags = QIODevice::ReadWrite);

    /**
     * @brief Discard changes without affecting the target file.
     * This will discard all changes that have been made to this file.
     * The target file will not be altered in any way.
     **/
    void abort();

    /**
     * @brief Finalize changes to the file.
     * This will commit all the changes that have been made to the file.
     * @return true if successful, or false if an error has occurred.
     **/
    bool finalize();

    /**
     * Allows writing over the existing file if necessary.
     *
     * QSaveFile creates a temporary file in the same directory as the final
     * file and atomically renames it. However this is not possible if the
     * directory permissions do not allow creating new files.
     * In order to preserve atomicity guarantees, open() fails when it
     * cannot create the temporary file.
     *
     * In order to allow users to edit files with write permissions in a
     * directory with restricted permissions, call setDirectWriteFallback() with
     * \a enabled set to true, and the following calls to open() will fallback to
     * opening the existing file directly and writing into it, without the use of
     * a temporary file.
     * This does not have atomicity guarantees, i.e. an application crash or
     * for instance a power failure could lead to a partially-written file on disk.
     * It also means cancelWriting() has no effect, in such a case.
     *
     * Typically, to save documents edited by the user, call setDirectWriteFallback(true),
     * and to save application internal files (configuration files, data files, ...), keep
     * the default setting which ensures atomicity.
     *
     * @since 4.10.3
     */
    void setDirectWriteFallback(bool enabled);

    /**
     * Returns true if the fallback solution for saving files in read-only
     * directories is enabled.
     *
     * @since 4.10.3
     */
    bool directWriteFallback() const;

private:
    Q_DISABLE_COPY(KSaveFile)

    class Private;
    Private *const d;
};

#endif
