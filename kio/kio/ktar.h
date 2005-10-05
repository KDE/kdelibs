/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2003 Leo Savernik <l.savernik@aon.at>

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
#ifndef __ktar_h
#define __ktar_h

#include <sys/stat.h>
#include <sys/types.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qstringlist.h>
#include <q3dict.h>

#include <karchive.h>

/**
 * A class for reading / writing (optionally compressed) tar archives.
 *
 * KTar allows you to read and write tar archives, including those 
 * that are compressed using gzip or bzip2.
 * 
 * @author Torben Weis <weis@kde.org>, David Faure <faure@kde.org>
 */
class KIO_EXPORT KTar : public KArchive
{
public:
    /**
     * Creates an instance that operates on the given filename
     * using the compression filter associated to given mimetype.
     *
     * @param filename is a local path (e.g. "/home/weis/myfile.tgz")
     * @param mimetype "application/x-gzip" or "application/x-bzip2"
     * Do not use application/x-tgz or similar - you only need to
     * specify the compression layer !  If the mimetype is omitted, it
     * will be determined from the filename.
     */
    KTar( const QString& filename, const QString & mimetype = QString::null );

    /**
     * Creates an instance that operates on the given device.
     * The device can be compressed (KFilterDev) or not (QFile, etc.).
     * @warning Do not assume that giving a QFile here will decompress the file,
     * in case it's compressed!
     * @param dev the device to read from. If the source is compressed, the
     * QIODevice must take care of decompression
     */
    KTar( QIODevice * dev );

    /**
     * If the tar ball is still opened, then it will be
     * closed automatically by the destructor.
     */
    virtual ~KTar();

    /**
     * The name of the tar file, as passed to the constructor
     * Null if you used the QIODevice constructor.
     * @return the name of the file, or QString::null if unknown
     */
    QString fileName() const { return m_filename; } 

    /**
     * Special function for setting the "original file name" in the gzip header,
     * when writing a tar.gz file. It appears when using in the "file" command,
     * for instance. Should only be called if the underlying device is a KFilterDev!
     * @param fileName the original file name
     */
    void setOrigFileName( const QByteArray & fileName );

    // TODO(BIC) make virtual. For now it must be implemented by virtual_hook.
    bool writeSymLink(const QString &name, const QString &target,
    			const QString &user, const QString &group,
    			mode_t perm, time_t atime, time_t mtime, time_t ctime);
    virtual bool writeDir( const QString& name, const QString& user, const QString& group );
    // TODO(BIC) make virtual. For now it must be implemented by virtual_hook.
    bool writeDir( const QString& name, const QString& user, const QString& group,
    			mode_t perm, time_t atime, time_t mtime, time_t ctime );
    virtual bool prepareWriting( const QString& name, const QString& user, const QString& group, uint size );
    // TODO(BIC) make virtual. For now it must be implemented by virtual_hook.
    bool prepareWriting( const QString& name, const QString& user,
    			const QString& group, uint size, mode_t perm,
       			time_t atime, time_t mtime, time_t ctime );
    virtual bool doneWriting( uint size );

protected:
    /**
     * Opens the archive for reading.
     * Parses the directory listing of the archive
     * and creates the KArchiveDirectory/KArchiveFile entries.
     * @param mode the mode of the file
     */
    virtual bool openArchive( QIODevice::OpenMode mode );
    virtual bool closeArchive();

private:
    /**
     * @internal
     */
    void prepareDevice( const QString & filename, const QString & mimetype, bool forced = false );

    /**
     * @internal
     * Fills @p buffer for writing a file as required by the tar format
     * Has to be called LAST, since it does the checksum
     * (normally, only the name has to be filled in before)
     * @param mode is expected to be 6 chars long, [uname and gname 31].
     */
    void fillBuffer( char * buffer, const char * mode, int size, time_t mtime,
    		char typeflag, const char * uname, const char * gname );

    /**
     * @internal
     * Writes an overlong name into a special longlink entry. Call this
     * if the file name or symlink target (or both) are longer than 99 chars.
     * @p buffer buffer at least 0x200 bytes big to be used as a write buffer
     * @p name 8-bit encoded file name to be written
     * @p typeflag specifying the type of the entry, 'L' for filenames or
     *		'K' for symlink targets.
     * @p uname user name
     * @p gname group name
     */
    void writeLonglink(char *buffer, const QByteArray &name, char typeflag,
			const char *uname, const char *gname);

    Q_LONG readRawHeader(char *buffer);
    bool readLonglink(char *buffer,QByteArray &longlink);
    Q_LONG readHeader(char *buffer,QString &name,QString &symlink);

    QString m_filename;
protected:
    virtual void virtual_hook( int id, void* data );
    bool prepareWriting_impl(const QString& name, const QString& user,
    			const QString& group, uint size, mode_t perm,
    			time_t atime, time_t mtime, time_t ctime);
    bool writeDir_impl(const QString& name, const QString& user,
    			const QString& group, mode_t perm,
    			time_t atime, time_t mtime, time_t ctime );
    bool writeSymLink_impl(const QString &name, const QString &target,
    			const QString &user, const QString &group,
    			mode_t perm, time_t atime, time_t mtime, time_t ctime);
private:
    class KTarPrivate;
    KTarPrivate * d;
};

/**
 * Old, deprecated naming
 */
#define KTarGz KTar
#define KTarEntry KArchiveEntry
#define KTarFile KArchiveFile
#define KTarDirectory KArchiveDirectory

#endif
