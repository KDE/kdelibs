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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __kzip_h
#define __kzip_h

#include <sys/stat.h>
#include <sys/types.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdict.h>
#include <qvaluelist.h>
#include <karchive.h>

class KZipFileEntry;
/**
 *   This class implements a kioslave to acces ZIP files from KDE.
 *   You can use it in IO_ReadOnly or in IO_WriteOnly mode, and it 
 *   behaves just as expected.
 *   It can also be used in IO_ReadWrite mode, in this case one can
 *   append files to an existing zip archive. when you append new files, which
 *   are not yet in the zip, it works as expected, they are appended at the end.
 *   when you append a file, which is already in the file, the reference to the
 *   old file is dropped and the new one is added to the zip. but the
 *   old data from the file itself is not deleted, it is still in the
 *   zipfile. so when you want to have a small and garbagefree zipfile,
 *   just read the contents of the appended zipfile and write it to a new one
 *   in IO_WriteOnly mode. exspecially take care of this, when you don´t want
 *   to leak information of how intermediate versions of files in the zip
 *   were looking.
 *   for more information on the zip fileformat go to
 *   http://www.pkware.com/support/appnote.html .
 * @short A class for reading/writing zip archives.
 * @author Holger Schroeder <holger-kde@holgis.net>
 * @since 3.1
 */
class KZip : public KArchive
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
     * WARNING: don't assume that giving a QFile here will decompress the file,
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
     * The name of the zip file, as passed to the constructor
     * Null if you used the QIODevice constructor.
     * @return the zip's file name, or null if a QIODevice is used
     */
    QString fileName() { return m_filename; }

    /**
     * Describes the Zip's compression type.
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
     * in small chunks using @ref writeData(), and call doneWriting() when done.
     * @param name can include subdirs e.g. path/to/the/file
     * @param user the user owning the file
     * @param group the group owning the file
     * @param size unused argument
     * @return true if successful, false otherwise
     */
    virtual bool prepareWriting( const QString& name, const QString& user, const QString& group, uint size );

    /**
     * Write data to a file that has been created using @ref prepareWriting().
     * @param data a pointer to the data
     * @param size the size of the chunk
     * @return true if successful, false otherwise
     */
    bool writeData( const char* data, uint size ); // TODO make virtual

    /**
     * Write data to a file that has been created using @ref prepareWriting().
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
    virtual bool openArchive( int mode );
    /// Closes the archive
    virtual bool closeArchive();

    /**
     * @internal Not needed for zip
     */
    virtual bool writeDir( const QString&, const QString&, const QString& ) { return true; }

protected:
    virtual void virtual_hook( int id, void* data );
    /** @internal for virtual_hook */
    // from KArchive
    bool writeData_impl( const char* data, uint size );
private:
    QString m_filename;
    class KZipPrivate;
    KZipPrivate * d;
};


/**
 * @internal
 */
class KZipFileEntry : public KArchiveFile
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

    // Only used when writing
    void setCompressedSize(Q_LONG compressedSize) { m_compressedSize = compressedSize; }

    // Header start: only used when writing
    void setHeaderStart(Q_LONG headerstart) { m_headerStart = headerstart; }
    Q_LONG headerStart() const {return m_headerStart; }

    // CRC: only used when writing
    unsigned long crc32() const { return m_crc; }
    void setCRC32(unsigned long crc32) { m_crc=crc32; }

    // Name with complete path - KArchiveFile::name() is the filename only (no path)
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
};

#endif
