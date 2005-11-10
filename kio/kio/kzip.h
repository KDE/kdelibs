/* This file is part of the KDE libraries
   Copyright (C) 2002 Holger Schroeder <holger-kde@holgis.net>

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
#ifndef __kzip_h
#define __kzip_h

#include <sys/stat.h>
#include <sys/types.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qstringlist.h>
#include <q3dict.h>
#include <q3valuelist.h>
#include <karchive.h>

class KZipFileEntry;
/**
 *   This class implements a kioslave to access zip files from KDE.
 *   You can use it in QIODevice::ReadOnly or in QIODevice::WriteOnly mode, and it
 *   behaves just as expected.
 *   It can also be used in QIODevice::ReadWrite mode, in this case one can
 *   append files to an existing zip archive. When you append new files, which
 *   are not yet in the zip, it works as expected, i.e. the files are appended at the end.
 *   When you append a file, which is already in the file, the reference to the
 *   old file is dropped and the new one is added to the zip - but the
 *   old data from the file itself is not deleted, it is still in the
 *   zipfile. so when you want to have a small and garbage-free zipfile,
 *   just read the contents of the appended zip file and write it to a new one
 *   in QIODevice::WriteOnly mode. This is especially important when you don't want
 *   to leak information of how intermediate versions of files in the zip
 *   were looking.
 *   For more information on the zip fileformat go to
 *   http://www.pkware.com/products/enterprise/white_papers/appnote.html
 * @short A class for reading/writing zip archives.
 * @author Holger Schroeder <holger-kde@holgis.net>
 * @since 3.1
 */
class KIO_EXPORT KZip : public KArchive
{
public:
    /**
     * Creates an instance that operates on the given filename.
     * using the compression filter associated to given mimetype.
     *
     * @param filename is a local path (e.g. "/home/holger/myfile.zip")
     */
    KZip( const QString& filename );

    /**
     * Creates an instance that operates on the given device.
     * The device can be compressed (KFilterDev) or not (QFile, etc.).
     * @warning Do not assume that giving a QFile here will decompress the file,
     * in case it's compressed!
     * @param dev the device to access
     */
    KZip( QIODevice * dev );

    /**
     * If the zip file is still opened, then it will be
     * closed automatically by the destructor.
     */
    virtual ~KZip();

    /**
     * The name of the zip file, as passed to the constructor.
     * Null if you used the QIODevice constructor.
     * @return the zip's file name, or null if a QIODevice is used
     */
    QString fileName() { return m_filename; }

    /**
     * Describes the contents of the "extra field" for a given file in the Zip archive.
     */
    enum ExtraField { NoExtraField = 0,      ///< No extra field
                      ModificationTime = 1,  ///< Modification time ("extended timestamp" header)
                      DefaultExtraField = 1
    };

    /**
     * Call this before writeFile or prepareWriting, to define what the next
     * file to be written should have in its extra field.
     * @param ef the type of "extra field"
     * @see extraField()
     */
    void setExtraField( ExtraField ef );

    /**
     * The current type of "extra field" that will be used for new files.
     * @return the current type of "extra field"
     * @see setExtraField()
     */
    ExtraField extraField() const;

    /**
     * Describes the compression type for a given file in the Zip archive.
     */
    enum Compression { NoCompression = 0,     ///< Uncompressed.
		       DeflateCompression = 1 ///< Deflate compression method.
    };


    /**
     * Call this before writeFile or prepareWriting, to define whether the next
     * files to be written should be compressed or not.
     * @param c the new compression mode
     * @see compression()
     */
    void setCompression( Compression c );

    /**
     * The current compression mode that will be used for new files.
     * @return the current compression mode
     * @see setCompression()
     */
    Compression compression() const;

    /**
     * If an archive is opened for writing then you can add a new file
     * using this function.
     * This method takes the whole data at once.
     * @param name can include subdirs e.g. path/to/the/file
     * @param user the user owning the file
     * @param group the group owning the file
     * @param size the size of the file
     * @param data a pointer to the data
     * @return true if successful, false otherwise
     */
    virtual bool writeFile( const QString& name, const QString& user, const QString& group, uint size, const char* data ); // BC: remove reimplementation for KDE-4.0

    /**
     * Alternative method for writing: call prepareWriting(), then feed the data
     * in small chunks using writeData(), and call doneWriting() when done.
     * @param name can include subdirs e.g. path/to/the/file
     * @param user the user owning the file
     * @param group the group owning the file
     * @param size unused argument
     * @return true if successful, false otherwise
     */
    virtual bool prepareWriting( const QString& name, const QString& user, const QString& group, uint size );

    // TODO(BIC) make virtual. For now it must be implemented by virtual_hook.
    bool writeSymLink(const QString &name, const QString &target,
                        const QString &user, const QString &group,
                        mode_t perm, time_t atime, time_t mtime, time_t ctime);
    // TODO(BIC) make virtual. For now it must be implemented by virtual_hook.
    bool prepareWriting( const QString& name, const QString& user,
                        const QString& group, uint size, mode_t perm,
                        time_t atime, time_t mtime, time_t ctime );
    // TODO(BIC) make virtual. For now it must be implemented by virtual_hook.
    bool writeFile( const QString& name, const QString& user, const QString& group,
                        uint size, mode_t perm, time_t atime, time_t mtime,
                        time_t ctime, const char* data );
    /**
     * Write data to a file that has been created using prepareWriting().
     * @param data a pointer to the data
     * @param size the size of the chunk
     * @return true if successful, false otherwise
     */
    bool writeData( const char* data, uint size ); // TODO make virtual

    /**
     * Write data to a file that has been created using prepareWriting().
     * @param size the size of the file
     * @return true if successful, false otherwise
     */
    virtual bool doneWriting( uint size );

protected:
    /**
     * Opens the archive for reading.
     * Parses the directory listing of the archive
     * and creates the KArchiveDirectory/KArchiveFile entries.
     * @param mode the mode of the file
     */
    virtual bool openArchive( QIODevice::OpenMode mode );
    /// Closes the archive
    virtual bool closeArchive();

    /**
     * @internal Not needed for zip
     */
    virtual bool writeDir( const QString& name, const QString& user, const QString& group) { Q_UNUSED(name); Q_UNUSED(user); Q_UNUSED(group); return true; }
    // TODO(BIC) uncomment and make virtual for KDE 4.
//    bool writeDir( const QString& name, const QString& user, const QString& group,
//                        mode_t perm, time_t atime, time_t mtime, time_t ctime );

protected:
    virtual void virtual_hook( int id, void* data );
    /** @internal for virtual_hook */
    // from KArchive
    bool writeData_impl( const char* data, uint size );
    bool prepareWriting_impl(const QString& name, const QString& user,
                        const QString& group, uint size, mode_t perm,
                        time_t atime, time_t mtime, time_t ctime);
    bool writeSymLink_impl(const QString &name, const QString &target,
                        const QString &user, const QString &group,
                        mode_t perm, time_t atime, time_t mtime, time_t ctime);
private:
    void abort();

private:
    QString m_filename;
    class KZipPrivate;
    KZipPrivate * d;
};


/**
 * @internal
 */
class KIO_EXPORT KZipFileEntry : public KArchiveFile
{
public:
    /*KZipFileEntry() : st(-1)
      {}*/
    KZipFileEntry( KZip* zip, const QString& name, int access, int date,
                   const QString& user, const QString& group, const QString& symlink,
                   const QString& path, Q_LONG start, Q_LONG uncompressedSize,
                   int encoding, Q_LONG compressedSize) :
        KArchiveFile( zip, name, access, date, user, group, symlink,
                      start, uncompressedSize ),
        m_crc(0),
        m_compressedSize(compressedSize),
        m_headerStart(0),
        m_encoding(encoding),
        m_path( path )
    {}
    int encoding() const { return m_encoding; }
    Q_LONG compressedSize() const { return m_compressedSize; }

    /// Only used when writing
    void setCompressedSize(Q_LONG compressedSize) { m_compressedSize = compressedSize; }

    /// Header start: only used when writing
    void setHeaderStart(Q_LONG headerstart) { m_headerStart = headerstart; }
    Q_LONG headerStart() const {return m_headerStart; }

    /// CRC: only used when writing
    unsigned long crc32() const { return m_crc; }
    void setCRC32(unsigned long crc32) { m_crc=crc32; }

    /// Name with complete path - KArchiveFile::name() is the filename only (no path)
    QString path() const { return m_path; }

    /**
     * @return the content of this file.
     * Call data() with care (only once per file), this data isn't cached.
     */
    virtual QByteArray data() const;

    /**
     * This method returns a QIODevice to read the file contents.
     * This is obviously for reading only.
     * Note that the ownership of the device is being transferred to the caller,
     * who will have to delete it.
     * The returned device auto-opens (in readonly mode), no need to open it.
     */
    QIODevice* device() const; // WARNING, not virtual!

private:
    unsigned long m_crc;
    Q_LONG m_compressedSize;
    Q_LONG m_headerStart;
    int m_encoding;
    QString m_path;
    // KDE4: d pointer or at least some int for future extensions
};

#endif
