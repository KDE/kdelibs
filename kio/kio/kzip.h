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

/**
 * @internal
 */
class KZipFileEntry
{
public:
    KZipFileEntry() : st(-1)
    {}
    KZipFileEntry(Q_LONG start, int encoding, Q_LONG csize) :
		st(start) , enc(encoding) , cs(csize)
    {}
    Q_LONG start() const {return st; }
    int encoding() const {return enc; }
    Q_LONG csize() const {return cs; }
    unsigned long crc32() const {return crc; }

    QString filename() const {return fn; }
    Q_LONG usize() const {return us; }
    Q_LONG headerstart() const {return hst; }

    void setStart(Q_LONG start) { st=start; }
    void setEncoding(int encoding) { enc=encoding; }
    void setCSize(Q_LONG csize) { cs=csize; }
    void setCRC32(unsigned long crc32) {crc=crc32; }

    void setFilename(QString filename) { fn=filename; }
    void setUSize(Q_LONG usize) { us=usize; }
    void setHeaderStart(Q_LONG headerstart) { hst=headerstart; }

private:
    Q_LONG st;
    int enc;
    Q_LONG cs;
    unsigned long crc;

    QString fn;
    Q_LONG us;
    Q_LONG hst;

};


/**
 * @short A class for reading/writing zip archives.
 * @author Holger Schroeder <holger-kde@holgis.net>
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
     */
    QString fileName() { return m_filename; }

    //void setOrigFileName( const QCString & fileName );

    virtual bool prepareWriting( const QString& name, const QString& user, const QString& group, uint size );
    virtual bool doneWriting( uint size );

protected:
    /**
     * Opens the archive for reading.
     * Parses the directory listing of the archive
     * and creates the KArchiveDirectory/KArchiveFile entries.
     *
     */
    virtual bool openArchive( int mode );
    virtual bool closeArchive();

    /**
     * @internal Not needed for zip
     */
    virtual bool writeDir( const QString&, const QString&, const QString& ) { return true; }

private:
    /////// Temporary
    Q_LONG readBlock(char *, long unsigned int);
    Q_LONG writeBlock(const char *, long unsigned int);
protected:
    virtual void virtual_hook( int id, void* data );
private:
    QString m_filename;
    class KZipPrivate;
    KZipPrivate * d;
    typedef QValueList<KZipFileEntry> KZipFileList;
    KZipFileList list;
    KZipFileList::iterator actualFile;
};

#endif
