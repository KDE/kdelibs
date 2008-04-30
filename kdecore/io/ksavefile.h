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

#include <kdecore_export.h>

#include <QtCore/QFile>
#include <QtCore/QString>
#include <kglobal.h>

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
 */
class KDECORE_EXPORT KSaveFile : public QFile
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
     * @param componentData The KComponentData to use for the temporary file.
     */
    explicit KSaveFile(const QString &filename, const KComponentData &componentData = KGlobal::mainComponent());

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
     * @brief Static method to create a backup file before saving.
     *
     * If empty (the default), the backup will be in the same directory as @p filename.
     * The backup type (simple, rcs, or numbered), extension string, and maximum
     * number of backup files are read from the user's global configuration.
     * Use simpleBackupFile() or numberedBackupFile() to force one of these
     * specific backup styles.
     * You can use this method even if you don't use KSaveFile.
     * @param filename the file to backup
     * @param backupDir optional directory where to save the backup file in.
     * @return true if successful, or false if an error has occurred.
     */
    static bool backupFile( const QString& filename,
                            const QString& backupDir = QString() );

    /**
     * @brief Static method to create a backup file for a given filename.
     *
     * This function creates a backup file from the given filename.
     * You can use this method even if you don't use KSaveFile.
     * @param filename the file to backup
     * @param backupDir optional directory where to save the backup file in.
     * If empty (the default), the backup will be in the same directory as @p filename.
     * @param backupExtension the extension to append to @p filename, "~" by default.
     * @return true if successful, or false if an error has occurred.
     */
    static bool simpleBackupFile( const QString& filename,
                                  const QString& backupDir = QString(),
                                  const QString& backupExtension = QLatin1String( "~" ) );

    /**
     * @brief Static method to create a backup file for a given filename.
     *
     * This function creates a series of numbered backup files from the
     * given filename.
     *
     * The backup file names will be of the form:
     *     \<name\>.\<number\>\<extension\>
     * for instance
     *     \verbatim chat.3.log \endverbatim
     *
     * The new backup file will be have the backup number 1.
     * Each existing backup file will have its number incremented by 1.
     * Any backup files with numbers greater than the maximum number
     * permitted (@p maxBackups) will be removed.
     * You can use this method even if you don't use KSaveFile.
     *
     * @param filename the file to backup
     * @param backupDir optional directory where to save the backup file in.
     * If empty (the default), the backup will be in the same directory as
     * @p filename.
     * @param backupExtension the extension to append to @p filename,
     * which is "~" by default.  Do not use an extension containing digits.
     * @param maxBackups the maximum number of backup files permitted.
     * For best performance a small number (10) is recommended.
     * @return true if successful, or false if an error has occurred.
     */
    static bool numberedBackupFile( const QString& filename,
                                    const QString& backupDir = QString(),
                                    const QString& backupExtension = QString::fromLatin1( "~" ),
                                    const uint maxBackups = 10
        );


    /**
     * @brief Static method to create an rcs backup file for a given filename.
     *
     * This function creates a rcs-formatted backup file from the
     * given filename.
     *
     * The backup file names will be of the form:
     *     \<name\>,v
     * for instance
     *     \verbatim photo.jpg,v \endverbatim
     *
     * The new backup file will be in RCS format.
     * Each existing backup file will be committed as a new revision.
     * You can use this method even if you don't use KSaveFile.
     *
     * @param filename the file to backup
     * @param backupDir optional directory where to save the backup file in.
     * If empty (the default), the backup will be in the same directory as
     * @p filename.
     * @param backupMessage is the RCS commit message for this revision.
     * @return true if successful, or false if an error has occurred.
     */
    static bool rcsBackupFile( const QString& filename,
                               const QString& backupDir = QString(),
                               const QString& backupMessage = QString()
        );

private:
    Q_DISABLE_COPY(KSaveFile)

    class Private;
    Private *const d;
};

#endif
