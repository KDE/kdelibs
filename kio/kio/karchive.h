/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   Moved from ktar.h by Roberto Teixeira <maragato@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __karchive_h
#define __karchive_h

#include <sys/stat.h>
#include <sys/types.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdict.h>

class KArchiveDirectory;
class KArchiveFile;

/**
 * @short generic class for reading/writing archives
 * @author David Faure <faure@kde.org>
 */
class KArchive
{
protected:
    /**
     * Base constructor (protected since this is a pure virtual class).
     * @param dev the I/O device where the archive reads its data
     * Note that this can be a file, but also a data buffer, a compression filter, etc.
     */
    KArchive( QIODevice * dev );

public:
    virtual ~KArchive();

    /**
     * Opens the archive for reading or writing.
     * @param mode may be IO_ReadOnly or IO_WriteOnly
     * Inherited classes might want to reimplement openArchive instead.
     * @see #close
     */
    virtual bool open( int mode );

    /**
     * Closes the archive.
     * Inherited classes might want to reimplement closeArchive instead.
     *
     * @see #open
     */
    virtual void close();

    /**
     * @return true if the archive is opened
     */
    bool isOpened() const { return m_open; }

    /**
     * @return the mode in which the archive was opened (IO_ReadOnly or IO_WriteOnly)
     */
    int mode() const { return m_mode; }

    /**
     * The underlying device.
     */
    QIODevice * device() const { return m_dev; }

    /**
     * If an archive is opened for reading, then the contents
     * of the archive can be accessed via this function.
     */
    const KArchiveDirectory* directory() const;

    /**
     * If an archive is opened for writing then you can add new directories
     * using this function. KArchive won't write one directory twice.
     */
    virtual bool writeDir( const QString& name, const QString& user, const QString& group ) = 0;

    /**
     * If an archive is opened for writing then you can add a new file
     * using this function. If the file name is for example "mydir/test1" then
     * the directory "mydir" is automatically appended first if that did not
     * happen yet.
     */
    virtual bool writeFile( const QString& name, const QString& user, const QString& group, uint size, const char* data );

    /**
     * Here's another way of writing a file into an archive:
     * Call @ref prepareWriting, then call device()->writeBlock() (for tar files)
     * or writeData (for zip files) [NEW VIRTUAL METHOD NEEDED]
     * as many times as wanted then call @ref doneWriting( totalSize )
     * For tar.gz files, you need to know the size before hand, it is needed in the header!
     * For zip files, size isn't used.
     */
    virtual bool prepareWriting( const QString& name, const QString& user, const QString& group, uint size ) = 0;

    /**
     * Call @ref doneWriting after writing the data.
     * @see prepareWriting
     */
    virtual bool doneWriting( uint size ) = 0;

protected:
    /**
     * Opens an archive for reading or writing.
     * Called by @ref open.
     */
    virtual bool openArchive( int mode ) = 0;

    /**
     * Closes the archive.
     * Called by @ref close.
     */
    virtual bool closeArchive() = 0;

    /**
     * Retrieves or create the root directory.
     * The default implementation assumes that openArchive() did the parsing,
     * so it creates a dummy rootdir if none was set (write mode, or no '/' in the archive).
     * Reimplement this to provide parsing/listing on demand.
     */
    virtual KArchiveDirectory* rootDir();

    /**
     * Ensures that @p path exists, create otherwise.
     * This handles e.g. tar files missing directory entries, like mico-2.3.0.tar.gz :)
     */
    KArchiveDirectory * findOrCreate( const QString & path );

    /**
     * @internal for inherited constructors
     */
    void setDevice( QIODevice *dev );

    /**
     * @internal for inherited classes
     */
    void setRootDir( KArchiveDirectory *rootDir );

private:
    QIODevice * m_dev;
    bool m_open;
    char m_mode;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KArchivePrivate;
    KArchivePrivate * d;
};

/**
 * @short Base class for the archive-file's directory structure.
 *
 * @see KArchiveFile
 * @see KArchiveDirectory
 */
class KArchiveEntry
{
public:
    KArchiveEntry( KArchive* archive, const QString& name, int access, int date,
               const QString& user, const QString& group,
               const QString &symlink );

    virtual ~KArchiveEntry() { }

    /**
     * Creation date of the file.
     */
    QDateTime datetime() const;
    int date() const { return m_date; }

    /**
     * Name of the file without path.
     */
    QString name() const { return m_name; }
    /**
     * The permissions and mode flags as returned by the stat() function
     * in st_mode.
     */
    mode_t permissions() const { return m_access; }
    /**
     * User who created the file.
     */
    QString user() const { return m_user; }
    /**
     * Group of the user who created the file.
     */
    QString group() const { return m_group; }

    /**
     * Symlink if there is one
     */
    QString symlink() const { return m_symlink; }

    /**
     * @return true if this entry is a file
     */
    virtual bool isFile() const { return false; }
    /**
     * @return true if this entry is a directory
     */
    virtual bool isDirectory() const { return false; }

protected:
    KArchive* archive() const { return m_archive; }

private:
    QString m_name;
    int m_date;
    mode_t m_access;
    QString m_user;
    QString m_group;
    QString m_symlink;
    KArchive* m_archive;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KArchiveEntryPrivate* d;
};

/**
 * @short A file in an archive.
 *
 * @see KArchive
 * @see KArchiveDirectory
 */
class KArchiveFile : public KArchiveEntry
{
public:
    KArchiveFile( KArchive* archive, const QString& name, int access, int date,
              const QString& user, const QString& group, const QString &symlink,
              int pos, int size );

    virtual ~KArchiveFile() { }

    /**
     * Position of the data in the [uncompressed] archive.
     */
    int position() const; // TODO use Q_LONG in KDE-4.0
    /**
     * Size of the data.
     */
    int size() const; // TODO use Q_LONG in KDE-4.0
    /**
     * Set size of data, usually after writing the file.
     */
    void setSize( int s ) { m_size = s; }

    /**
     * @return the content of this file.
     * Call data() with care (only once per file), this data isn't cached.
     */
    virtual QByteArray data() const;

    /**
     * This method returns QIODevice (internal class: KLimitedIODevice)
     * on top of the underlying QIODevice. This is obviously for reading only.
     * Note that the ownership of the device is being transferred to the caller,
     * who will have to delete it.
     * The returned device auto-opens (in readonly mode), no need to open it.
     */
    QIODevice *device() const; // TODO make virtual

    /**
     * @return true, since this entry is a file
     */
    virtual bool isFile() const { return true; }

private:
    int m_pos; // TODO use Q_LONG in KDE-4.0
    int m_size; // TODO use Q_LONG in KDE-4.0
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KArchiveFilePrivate* d;
};

/**
 * @short A directory in an archive.
 *
 * @see KArchive
 * @see KArchiveFile
 */
class KArchiveDirectory : public KArchiveEntry
{
public:
    KArchiveDirectory( KArchive* archive, const QString& name, int access, int date,
                   const QString& user, const QString& group,
                   const QString& symlink);

    virtual ~KArchiveDirectory() { }

    /**
     * @return the names of all entries in this directory (filenames, no path).
     */
    QStringList entries() const;
    /**
     * @return a pointer to the entry in the directory.
     *
     * @param name may be "test1", "mydir/test3", "mydir/mysubdir/test3", etc.
     */
    KArchiveEntry* entry( QString name );
    const KArchiveEntry* entry( QString name ) const;

    /**
     * @internal
     * Adds a new entry to the directory.
     */
    void addEntry( KArchiveEntry* );

    /**
     * @return true, since this entry is a directory
     */
    virtual bool isDirectory() const { return true; }

private:
    QDict<KArchiveEntry> m_entries;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KArchiveDirectoryPrivate* d;
};

#endif
