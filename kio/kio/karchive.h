/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2003 Leo Savernik <l.savernik@aon.at>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef __karchive_h
#define __karchive_h

#include <sys/stat.h>
#include <sys/types.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qstringlist.h>
#include <q3dict.h>

#include <kdelibs_export.h>

class KArchiveDirectory;
class KArchiveFile;

/**
 * KArchive is a base class for reading and writing archives.
 * @short generic class for reading/writing archives
 * @author David Faure <faure@kde.org>
 */
class KIO_EXPORT KArchive
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
     * Inherited classes might want to reimplement openArchive instead.
     * @param mode may be IO_ReadOnly or IO_WriteOnly
     * @see close
     */
    virtual bool open( QIODevice::OpenMode mode );

    /**
     * Closes the archive.
     * Inherited classes might want to reimplement closeArchive instead.
     *
     * @see open
     */
    virtual void close();

    /**
     * Use to check if close had any problem
     * @return true if close succeded without problems
     * @since 3.5
     */
    // TODO KDE4 merge with above
    bool closeSucceeded() const;

    /**
     * Checks whether the archive is open.
     * @return true if the archive is opened
     */
    bool isOpened() const { return m_open; }

    /**
     * Returns the mode in which the archive was opened
     * @return the mode in which the archive was opened (IO_ReadOnly or IO_WriteOnly)
     * @see open()
     */
    int mode() const { return m_mode; }

    /**
     * The underlying device.
     * @return the underlying device.
     */
    QIODevice * device() const { return m_dev; }

    /**
     * If an archive is opened for reading, then the contents
     * of the archive can be accessed via this function.
     * @return the directory of the archive
     */
    const KArchiveDirectory* directory() const;

    /**
     * Writes a local file into the archive. The main difference with writeFile,
     * is that this method minimizes memory usage, by not loading the whole file
     * into memory in one go.
     *
     * If @p fileName is a symbolic link, it will be written as is, i. e.
     * it will not be resolved before.
     * @param fileName full path to an existing local file, to be added to the archive.
     * @param destName the resulting name (or relative path) of the file in the archive.
     */
    bool addLocalFile( const QString& fileName, const QString& destName );

    /**
     * Writes a local directory into the archive, including all its contents, recursively.
     * Calls addLocalFile for each file to be added.
     *
     * Since KDE 3.2 it will also add a @p path that is a symbolic link to a
     * directory. The symbolic link will be dereferenced and the content of the
     * directory it is pointing to added recursively. However, symbolic links
     * *under* @p path will be stored as is.
     * @param path full path to an existing local directory, to be added to the archive.
     * @param destName the resulting name (or relative path) of the file in the archive.
     */
    bool addLocalDirectory( const QString& path, const QString& destName );

    /**
     * If an archive is opened for writing then you can add new directories
     * using this function. KArchive won't write one directory twice.
     *
     * @param name the name of the directory
     * @param user the user that owns the directory
     * @param group the group that owns the directory
     *
     * @todo TODO(BIC): make this a thin wrapper around
     * writeDir(name,user,group,perm,atime,mtime,ctime)
     * or eliminate it
     */
    virtual bool writeDir( const QString& name, const QString& user, const QString& group ) = 0;

    /**
     * If an archive is opened for writing then you can add new directories
     * using this function. KArchive won't write one directory twice.
     *
     * This method also allows some file metadata to be
     * set. However, depending on the archive type not all metadata might be
     * regarded.
     * @param name the name of the directory
     * @param user the user that owns the directory
     * @param group the group that owns the directory
     * @param perm permissions of the directory
     * @param atime time the file was last accessed
     * @param mtime modification time of the file
     * @param ctime creation time of the file
     * @since 3.2
     * @todo TODO(BIC): make this virtual. For now use virtual hook
     */
    bool writeDir( const QString& name, const QString& user, const QString& group,
    			mode_t perm, time_t atime, time_t mtime, time_t ctime );

    /**
     * Writes a symbolic link to the archive if the archive must be opened for
     * writing.
     * @param name name of symbolic link
     * @param target target of symbolic link
     * @param user the user that owns the directory
     * @param group the group that owns the directory
     * @param perm permissions of the directory
     * @param atime time the file was last accessed
     * @param mtime modification time of the file
     * @param ctime creation time of the file
     * @since 3.2
     * @todo TODO(BIC) make virtual. For now it must be implemented by virtual_hook.
     */
    bool writeSymLink(const QString &name, const QString &target,
    			const QString &user, const QString &group,
    			mode_t perm, time_t atime, time_t mtime, time_t ctime);

    /**
     * If an archive is opened for writing then you can add a new file
     * using this function. If the file name is for example "mydir/test1" then
     * the directory "mydir" is automatically appended first if that did not
     * happen yet.
     * @param name the name of the file
     * @param user the user that owns the file
     * @param group the group that owns the file
     * @param size the size of the file
     * @param data the data to write (@p size bytes)
     * @todo TODO(BIC): make this a thin non-virtual wrapper around
     *   writeFile(name,user,group,size,perm,atime,mtime,ctime,data)
     */
    virtual bool writeFile( const QString& name, const QString& user, const QString& group, uint size, const char* data );

    /**
     * If an archive is opened for writing then you can add a new file
     * using this function. If the file name is for example "mydir/test1" then
     * the directory "mydir" is automatically appended first if that did not
     * happen yet.
     *
     * This method also allows some file metadata to be
     * set. However, depending on the archive type not all metadata might be
     * regarded.
     * @param name the name of the file
     * @param user the user that owns the file
     * @param group the group that owns the file
     * @param size the size of the file
     * @param perm permissions of the file
     * @param atime time the file was last accessed
     * @param mtime modification time of the file
     * @param ctime creation time of the file
     * @param data the data to write (@p size bytes)
     * @since 3.2
     * @todo TODO(BIC): make virtual. For now use virtual hook
     */
    bool writeFile( const QString& name, const QString& user, const QString& group,
    			uint size, mode_t perm, time_t atime, time_t mtime,
       			time_t ctime, const char* data );

    /**
     * Here's another way of writing a file into an archive:
     * Call prepareWriting, then call writeData()
     * as many times as wanted then call doneWriting( totalSize ).
     * For tar.gz files, you need to know the size before hand, since it is needed in the header.
     * For zip files, size isn't used.
     *
     * @param name the name of the file
     * @param user the user that owns the file
     * @param group the group that owns the file
     * @param size the size of the file
     *
     * @todo TODO(BIC): make this a thin non-virtual wrapper around
     * prepareWriting(name,user,group,size,perm,atime,mtime,ctime)
     * or eliminate it.
     */
    virtual bool prepareWriting( const QString& name, const QString& user, const QString& group, uint size ) = 0;

    /**
     * Here's another way of writing a file into an archive:
     * Call prepareWriting, then call writeData()
     * as many times as wanted then call doneWriting( totalSize ).
     * For tar.gz files, you need to know the size before hand, it is needed in the header!
     * For zip files, size isn't used.
     *
     * This method also allows some file metadata to be
     * set. However, depending on the archive type not all metadata might be
     * regarded.
     * @param name the name of the file
     * @param user the user that owns the file
     * @param group the group that owns the file
     * @param size the size of the file
     * @param perm permissions of the file
     * @param atime time the file was last accessed
     * @param mtime modification time of the file
     * @param ctime creation time of the file
     * @since 3.2
     * @todo TODO(BIC): make this virtual. For now use virtual hook.
     */
    bool prepareWriting( const QString& name, const QString& user,
    			const QString& group, uint size, mode_t perm,
       			time_t atime, time_t mtime, time_t ctime );

    /**
     * Write data into the current file - to be called after calling prepareWriting
     * @todo TODO(BIC) make virtual. For now virtual_hook allows reimplementing it.
     */
    bool writeData( const char* data, uint size );

    /**
     * Call doneWriting after writing the data.
     * @param size the size of the file
     * @see prepareWriting()
     */
    virtual bool doneWriting( uint size ) = 0;

protected:
    /**
     * Opens an archive for reading or writing.
     * Called by open.
     * @param mode may be IO_ReadOnly or IO_WriteOnly
     */
    virtual bool openArchive( QIODevice::OpenMode mode ) = 0;

    /**
     * Closes the archive.
     * Called by close.
     */
    virtual bool closeArchive() = 0;

    /**
     * Retrieves or create the root directory.
     * The default implementation assumes that openArchive() did the parsing,
     * so it creates a dummy rootdir if none was set (write mode, or no '/' in the archive).
     * Reimplement this to provide parsing/listing on demand.
     * @return the root directory
     */
    virtual KArchiveDirectory* rootDir();

    /**
     * Ensures that @p path exists, create otherwise.
     * This handles e.g. tar files missing directory entries, like mico-2.3.0.tar.gz :)
     * @param path the path of the directory
     * @return the directory with the given @p path
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
    /* @internal for virtual_hook */
    enum { VIRTUAL_WRITE_DATA = 1, VIRTUAL_WRITE_SYMLINK, VIRTUAL_WRITE_DIR,
    	VIRTUAL_WRITE_FILE, VIRTUAL_PREPARE_WRITING };
    bool prepareWriting_impl( const QString& name, const QString& user,
    			const QString& group, uint size, mode_t perm,
    			time_t atime, time_t mtime, time_t ctime );
    struct PrepareWritingParams {
	const QString *name;
	const QString *user;
	const QString *group;
	uint size;
	mode_t perm;
	time_t atime, mtime, ctime;
	bool retval;
    };
    bool writeFile_impl( const QString& name, const QString& user,
    			const QString& group, uint size, mode_t perm,
    			time_t atime, time_t mtime, time_t ctime,
       			const char* data );
    struct WriteFileParams {
	const QString *name;
	const QString *user;
	const QString *group;
	uint size;
	mode_t perm;
	time_t atime, mtime, ctime;
	const char *data;
	bool retval;
    };
    bool writeDir_impl(const QString& name, const QString& user,
    			const QString& group, mode_t perm,
    			time_t atime, time_t mtime, time_t ctime);
    struct WriteDirParams {
	const QString *name;
	const QString *user;
	const QString *group;
	mode_t perm;
	time_t atime, mtime, ctime;
	bool retval;
    };
    bool writeSymLink_impl(const QString &name, const QString &target,
    			const QString &user, const QString &group,
    			mode_t perm, time_t atime, time_t mtime, time_t ctime);
    struct WriteSymlinkParams {
	const QString *name;
	const QString *target;
	const QString *user;
	const QString *group;
	mode_t perm;
	time_t atime, mtime, ctime;
	bool retval;
    };
    bool writeData_impl( const char* data, uint size );
    struct WriteDataParams {
        const char* data;
        uint size;
        bool retval;
    };
private:
    class KArchivePrivate;
    KArchivePrivate * d;
};

/**
 * A base class for entries in an KArchive.
 * @short Base class for the archive-file's directory structure.
 *
 * @see KArchiveFile
 * @see KArchiveDirectory
 */
class KIO_EXPORT KArchiveEntry
{
public:
    /**
     * Creates a new entry.
     * @param archive the entries archive
     * @param name the name of the entry
     * @param access the permissions in unix format
     * @param date the date (in seconds since 1970)
     * @param user the user that owns the entry
     * @param group the group that owns the entry
     * @param symlink the symlink, or QString::null
     */
    KArchiveEntry( KArchive* archive, const QString& name, int access, int date,
               const QString& user, const QString& group,
               const QString &symlink );

    virtual ~KArchiveEntry() { }

    /**
     * Creation date of the file.
     * @return the creation date
     */
    QDateTime datetime() const;

    /**
     * Creation date of the file.
     * @return the creation date in seconds since 1970
     */
    int date() const { return m_date; }

    /**
     * Name of the file without path.
     * @return the file name without path
     */
    QString name() const { return m_name; }
    /**
     * The permissions and mode flags as returned by the stat() function
     * in st_mode.
     * @return the permissions
     */
    mode_t permissions() const { return m_access; }
    /**
     * User who created the file.
     * @return the owner of the file
     */
    QString user() const { return m_user; }
    /**
     * Group of the user who created the file.
     * @return the group of the file
     */
    QString group() const { return m_group; }

    /**
     * Symlink if there is one.
     * @return the symlink, or QString::null
     */
    QString symlink() const { return m_symlink; }

    /**
     * Checks whether the entry is a file.
     * @return true if this entry is a file
     */
    virtual bool isFile() const { return false; }

    /**
     * Checks whether the entry is a directory.
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
 * Represents a file entry in a KArchive.
 * @short A file in an archive.
 *
 * @see KArchive
 * @see KArchiveDirectory
 */
class KIO_EXPORT KArchiveFile : public KArchiveEntry
{
public:
    /**
     * Creates a new file entry.
     * @param archive the entries archive
     * @param name the name of the entry
     * @param access the permissions in unix format
     * @param date the date (in seconds since 1970)
     * @param user the user that owns the entry
     * @param group the group that owns the entry
     * @param symlink the symlink, or QString::null
     * @param pos the position of the file in the directory
     * @param size the size of the file
     */
    KArchiveFile( KArchive* archive, const QString& name, int access, int date,
              const QString& user, const QString& group, const QString &symlink,
              int pos, int size );

    virtual ~KArchiveFile() { }

    /**
     * Position of the data in the [uncompressed] archive.
     * @return the position of the file
     */
    int position() const; // TODO use Q_LONG in KDE-4.0
    /**
     * Size of the data.
     * @return the size of the file
     */
    int size() const; // TODO use Q_LONG in KDE-4.0
    /**
     * Set size of data, usually after writing the file.
     * @param s the new size of the file
     */
    void setSize( int s ) { m_size = s; }

    /**
     * Returns the data of the file.
     * Call data() with care (only once per file), this data isn't cached.
     * @return the content of this file.
     */
    virtual QByteArray data() const;

    /**
     * This method returns QIODevice (internal class: KLimitedIODevice)
     * on top of the underlying QIODevice. This is obviously for reading only.
     * Note that the ownership of the device is being transferred to the caller,
     * who will have to delete it.
     * The returned device auto-opens (in readonly mode), no need to open it.
     * @return the QIODevice of the file
     */
    QIODevice *device() const; // TODO make virtual

    /**
     * Checks whether this entry is a file.
     * @return true, since this entry is a file
     */
    virtual bool isFile() const { return true; }

    /**
     * Extracts the file to the directory @p dest
     * @param dest the directory to extract to
     * @since 3.1
     */
    void copyTo(const QString& dest) const;

private:
    int m_pos; // TODO use Q_LONG in KDE-4.0
    int m_size; // TODO use Q_LONG in KDE-4.0
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KArchiveFilePrivate* d;
};

/**
 * Represents a directory entry in a KArchive.
 * @short A directory in an archive.
 *
 * @see KArchive
 * @see KArchiveFile
 */
class KIO_EXPORT KArchiveDirectory : public KArchiveEntry
{
public:
    /**
     * Creates a new directory entry.
     * @param archive the entries archive
     * @param name the name of the entry
     * @param access the permissions in unix format
     * @param date the date (in seconds since 1970)
     * @param user the user that owns the entry
     * @param group the group that owns the entry
     * @param symlink the symlink, or QString::null
     */
    KArchiveDirectory( KArchive* archive, const QString& name, int access, int date,
                   const QString& user, const QString& group,
                   const QString& symlink);

    virtual ~KArchiveDirectory() { }

    /**
     * Returns a list of sub-entries.
     * @return the names of all entries in this directory (filenames, no path).
     */
    QStringList entries() const;
    /**
     * Returns the entry with the given name.
     * @param name may be "test1", "mydir/test3", "mydir/mysubdir/test3", etc.
     * @return a pointer to the entry in the directory.
     */
    KArchiveEntry* entry( QString name );
    /**
     * Returns the entry with the given name.
     * @param name may be "test1", "mydir/test3", "mydir/mysubdir/test3", etc.
     * @return a pointer to the entry in the directory.
     */
    const KArchiveEntry* entry( QString name ) const;

    /**
     * @internal
     * Adds a new entry to the directory.
     */
    void addEntry( KArchiveEntry* );

    /**
     * Checks whether this entry is a directory.
     * @return true, since this entry is a directory
     */
    virtual bool isDirectory() const { return true; }

    /**
     * Extracts all entries in this archive directory to the directory
     * @p dest.
     * @param dest the directory to extract to
     * @param recursive if set to true, subdirectories are extracted as well
     * @since 3.1
     */
     void copyTo(const QString& dest, bool recursive = true) const;

private:
    Q3Dict<KArchiveEntry> m_entries;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KArchiveDirectoryPrivate* d;
};

#endif
