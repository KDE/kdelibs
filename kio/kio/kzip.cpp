
/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>
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

/*
	This class implements a kioslave to acces ZIP files from KDE.
    you can use it in IO_ReadOnly or in IO_WriteOnly mode, and it
    behaves just as expected (i hope ;-) ).
    It can also be used in IO_ReadWrite mode, in this case one can
    append files to an existing zip archive. when you append new files, which
    are not yet in the zip, it works as expected, they are appended at the end.
    when you append a file, which is already in the file, the reference to the
    old file is dropped and the new one is added to the zip. but the
    old data from the file itself is not deleted, it is still in the
    zipfile. so when you want to have a small and garbagefree zipfile,
    just read the contents of the appended zipfile and write it to a new one
    in IO_WriteOnly mode. exspecially take care of this, when you don´t want
    to leak information of how intermediate versions of files in the zip
    were looking.
    for more information on the zip fileformat go to
    http://www.pkware.com/support/appnote.html .

*/



#include <sys/time.h>

#include <qfile.h>
#include <qdir.h>
#include <time.h>
#include <string.h>
#include <qdatetime.h>
#include <kdebug.h>
#include <qptrlist.h>
#include <kmimetype.h>
#include <zlib.h>

#include "kfilterdev.h"
#include "kzip.h"
#include "klimitediodevice.h"

static void transformToMsDos(const QDateTime& dt, char* buffer)
{
    if ( dt.isValid() )
    {
        Q_UINT16 time =
             ( dt.time().hour() << 11 )    // 5 bit hour
           | ( dt.time().minute() << 5 )   // 6 bit minute
           | ( dt.time().second() >> 1 );  // 5 bit double seconds

        buffer[0] = char(time);
        buffer[1] = char(time >> 8);

        Q_UINT16 date =
             ( ( dt.date().year() - 1980 ) << 9 ) // 7 bit year 1980-based
           | ( dt.date().month() << 5 )           // 4 bit month
           | ( dt.date().day() );                 // 5 bit day

        buffer[2] = char(date);
        buffer[3] = char(date >> 8);
    }
    else // !dt.isValid(), assume 1980-01-01 midnight
    {
        buffer[0] = 0;
        buffer[1] = 0;
        buffer[2] = 33;
        buffer[3] = 0;
    }
}

static int getActualTime( void )
{
    timeval value;
    gettimeofday( &value, NULL );
    return value.tv_sec;
}

////////////////////////////////////////////////////////////////////////
/////////////////////////// KZip ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////

class KZip::KZipPrivate
{
public:
    KZipPrivate()
        : m_crc( 0 ),
          m_currentFile( 0L ),
          m_currentDev( 0L ),
          m_compression( 8 ),
	  m_offset( 0L ) { }

    unsigned long           m_crc;         // checksum
    KZipFileEntry*          m_currentFile; // file currently being written
    QIODevice*              m_currentDev;  // filterdev used to write to the above file
    QPtrList<KZipFileEntry> m_fileList;    // flat list of all files, for the index (saves a recursive method ;)
    int                     m_compression;
    unsigned int	        m_offset; // holds the offset of the place in the zip,
    // where new data can be appended. after openarchive it points to 0, when in
    // writeonly mode, or it points to the beginning of the central directory.
    // each call to writefile updates this value.
};

KZip::KZip( const QString& filename )
    : KArchive( 0L )
{
    //kdDebug(7040) << "KZip(filename) reached." << endl;
    m_filename = filename;
    d = new KZipPrivate;
    setDevice( new QFile( filename ) );
}

KZip::KZip( QIODevice * dev )
    : KArchive( dev )
{
    //kdDebug(7040) << "KZip::KZip( QIODevice * dev) reached." << endl;
    d = new KZipPrivate;
}

KZip::~KZip()
{
    // mjarrett: Closes to prevent ~KArchive from aborting w/o device
    //kdDebug(7040) << "~KZip reached." << endl;
    if( isOpened() )
        close();
    if ( !m_filename.isEmpty() )
        delete device(); // we created it ourselves
    delete d;
}

bool KZip::openArchive( int mode )
{
    //kdDebug(7040) << "openarchive reached." << endl;
    d->m_fileList.clear();

    if ( mode == IO_WriteOnly )
        return true;
    if ( mode != IO_ReadOnly && mode != IO_ReadWrite )
    {
        kdWarning(7040) << "Unsupported mode " << mode << endl;
        return false;
    }

    char buffer[47];

    // Check that it's a valid ZIP file
    // KArchive::open() opened the underlying device already.
    QIODevice* dev = device();

    uint offset = 0; // holds offset, where we read
    int n;

    for (;;) // repeat until 'end of entries' signature is reached
    {
        n = dev->readBlock( buffer, 4 );

        if (n < 4)
        {
            kdWarning(7040) << "Invalid ZIP file. Unexpected end of file. (#1)" << endl;

            return false;
        }

        if ( !memcmp( buffer, "PK\5\6", 4 ) ) // 'end of entries'
            break;

        if ( !memcmp( buffer, "PK\3\4", 4 ) ) // local file header
        {
            dev->at( dev->at() + 2 ); // skip 'version needed to extract'

            // we have to take care of the 'general purpose bit flag'.
            // if bit 3 is set, the header doesn't contain the length of
            // the file and we look for the signature 'PK\7\8'.

            dev->readBlock( buffer, 2 );
            if ( buffer[0] & 8 )
            {
                bool foundSignature = false;

                while (!foundSignature)
                {
                    n = dev->readBlock( buffer, 1 );
                    if (n < 1)
                    {
                        kdWarning(7040) << "Invalid ZIP file. Unexpected end of file. (#2)" << endl;
                        return false;
                    }

                    if ( buffer[0] != 'P' )
                        continue;

                    n = dev->readBlock( buffer, 3 );
                    if (n < 3)
                    {
                        kdWarning(7040) << "Invalid ZIP file. Unexpected end of file. (#3)" << endl;
                        return false;
                    }

                    if ( buffer[0] == 'K' && buffer[1] == 7 && buffer[2] == 8 )
                    {
                        foundSignature = true;
                        dev->at( dev->at() + 12 ); // skip the 'data_descriptor'
                    }
                }
            }
            else
            {
                // here we calculate the length of the file in the zip
                // with headers and jump to the next header.
                dev->at( dev->at() + 10 );

                uint skip;

                n = dev->readBlock( buffer, 4 ); // compressed file size
                skip = (uchar)buffer[3] << 24 | (uchar)buffer[2] << 16 |
                       (uchar)buffer[1] << 8 | (uchar)buffer[0];

                dev->at( dev->at() + 4 );
                n = dev->readBlock( buffer, 2 ); // file name length
                skip += (uchar)buffer[1] << 8 | (uchar)buffer[0];

                n = dev->readBlock( buffer, 2 ); // extra field length
                skip += (uchar)buffer[1] << 8 | (uchar)buffer[0];

                dev->at( dev->at() + skip );
                offset += 30 + skip;
            }
        }
        else if ( !memcmp( buffer, "PK\1\2", 4 ) ) // central block
        {

            // so we reached the central header at the end of the zip file
		    // here we get all interesting data out of the central header
            // of a file
            offset = dev->at() - 4;

            //set offset for appending new files
            if ( d->m_offset == 0L ) d->m_offset = offset;

		    n = dev->readBlock( buffer + 4, 42 );
            if (n < 42) {
                kdWarning(7040) << "Invalid ZIP file, central entry too short" << endl; // not long enough for valid entry
                return false;
            }
			// length of the filename (well, pathname indeed)
            int namelen = (uchar)buffer[29] << 8 | (uchar)buffer[28];
            char* bufferName = new char[ namelen + 1 ];
            n = dev->readBlock( bufferName, namelen );
            if ( n < namelen )
                kdWarning(7040) << "Invalid ZIP file. Name not completely read" << endl;
            QString name( QString::fromLocal8Bit(bufferName, namelen) );
            delete[] bufferName;

            //kdDebug(7040) << "name: " << name << endl;
            // only in central header ! see below.
            // length of extra attributes
            int extralen = (uchar)buffer[31] << 8 | (uchar)buffer[30];
            // length of comment for this file
            int commlen =  (uchar)buffer[33] << 8 | (uchar)buffer[32];
            // compression method of this file
            int cmethod =  (uchar)buffer[11] << 8 | (uchar)buffer[10];

            //kdDebug(7040) << "cmethod: " << cmethod << endl;
            //kdDebug(7040) << "extralen: " << extralen << endl;

            // uncompressed file size
            uint ucsize = (uchar)buffer[27] << 24 | (uchar)buffer[26] << 16 |
	    		(uchar)buffer[25] << 8 | (uchar)buffer[24];
            // compressed file size
            uint csize = (uchar)buffer[23] << 24 | (uchar)buffer[22] << 16 |
	    		(uchar)buffer[21] << 8 | (uchar)buffer[20];

            // offset of local header
            uint localheaderoffset = (uchar)buffer[45] << 24 | (uchar)buffer[44] << 16 |
				(uchar)buffer[43] << 8 | (uchar)buffer[42];

            // some clever people use different extra field lengths
            // in the central header and in the local header... funny.
            // so we need to get the localextralen to calculate the offset
            // from localheaderstart to dataoffset
            char localbuf[5];
            int save_at = dev->at();
		    dev->at( localheaderoffset + 28 );
            dev->readBlock( localbuf, 4);
            int localextralen = (uchar)localbuf[1] << 8 | (uchar)localbuf[0];
		    dev->at(save_at);

            //kdDebug(7040) << "localextralen: " << localextralen << endl;

            // offset, where the real data for uncompression starts
            uint dataoffset = localheaderoffset + 30 + localextralen + namelen; //comment only in central header

            //kdDebug(7040) << "esize: " << esize << endl;
            //kdDebug(7040) << "eoffset: " << eoffset << endl;
            //kdDebug(7040) << "csize: " << csize << endl;

            bool isdir = false;
            int access = 0777; // TODO available in zip file?
            int time = getActualTime();

            QString entryName;

		    if ( name.endsWith( "/" ) ) // Entries with a trailing slash are directories
            {
                isdir = true;
                name = name.left( name.length() - 1 );
                access |= S_IFDIR;
            }

            int pos = name.findRev( '/' );
            if ( pos == -1 )
                entryName = name;
            else
                entryName = name.mid( pos + 1 );
            Q_ASSERT( !entryName.isEmpty() );

            KArchiveEntry* entry;
            if ( isdir )
                entry = new KArchiveDirectory( this, entryName, access, time, rootDir()->user(), rootDir()->group(), QString::null );
            else
            {
                entry = new KZipFileEntry( this, entryName, access, time, rootDir()->user(), rootDir()->group(), QString::null,
                                          name, dataoffset, ucsize, cmethod, csize );
                static_cast<KZipFileEntry *>(entry)->setHeaderStart( localheaderoffset );
                //kdDebug(7040) << "KZipFileEntry created" << endl;
                d->m_fileList.append( static_cast<KZipFileEntry *>( entry ) );
            }

            if ( pos == -1 )
            {
                rootDir()->addEntry(entry);
            }
            else
            {
                // In some tar files we can find dir/./file => call cleanDirPath
                QString path = QDir::cleanDirPath( name.left( pos ) );
                // Ensure container directory exists, create otherwise
                KArchiveDirectory * tdir = findOrCreate( path );
                tdir->addEntry(entry);
            }

            //calculate offset to next entry
            offset += 46 + commlen + extralen + namelen;
            bool b = dev->at(offset);
            Q_ASSERT( b );
            if ( !b )
              return false;
        }
        else
        {
            kdWarning(7040) << "Invalid ZIP file. Unrecognized header at offset " << offset << endl;

            return false;
        }
    }
    //kdDebug(7040) << "*** done *** " << endl;
    return true;
}

bool KZip::closeArchive()
{
    if ( ! ( mode() & IO_WriteOnly ) )
    {
        //kdDebug(7040) << "closearchive readonly reached." << endl;
        return true;
    }
    //ReadWrite or WriteOnly
    //write all central dir file entries

    // to be written at the end of the file...
    char buffer[ 22 ]; // first used for 12, then for 22 at the end
    uLong crc = crc32(0L, Z_NULL, 0);

    Q_LONG centraldiroffset = device()->at();
    //kdDebug(7040) << "closearchive: centraldiroffset: " << centraldiroffset << endl;
    Q_LONG atbackup = device()->at();
    QPtrListIterator<KZipFileEntry> it( d->m_fileList );

    for ( ; it.current() ; ++it )
    {	//set crc and compressed size in each local file header
        device()->at( it.current()->headerStart() + 14 );
	//kdDebug(7040) << "closearchive setcrcandcsize: filename: "
	//    << it.current()->path()
	//    << " encoding: "<< it.current()->encoding() << endl;

        uLong mycrc = it.current()->crc32();
        buffer[0] = char(mycrc); // crc checksum, at headerStart+14
        buffer[1] = char(mycrc >> 8);
        buffer[2] = char(mycrc >> 16);
        buffer[3] = char(mycrc >> 24);

        int mysize1 = it.current()->compressedSize();
        buffer[4] = char(mysize1); // compressed file size, at headerStart+18
        buffer[5] = char(mysize1 >> 8);
        buffer[6] = char(mysize1 >> 16);
        buffer[7] = char(mysize1 >> 24);

        int myusize = it.current()->size();
        buffer[8] = char(myusize); // uncompressed file size, at headerStart+22
        buffer[9] = char(myusize >> 8);
        buffer[10] = char(myusize >> 16);
        buffer[11] = char(myusize >> 24);

        device()->writeBlock( buffer, 12 );
    }
    device()->at( atbackup );

    for ( it.toFirst(); it.current() ; ++it )
    {
        //kdDebug(7040) << "closearchive: filename: " << it.current()->path()
        //              << " encoding: "<< it.current()->encoding() << endl;

        QCString path = QFile::encodeName(it.current()->path());

        int bufferSize = path.length() + 46;
        char* buffer = new char[ bufferSize ];

        memset(buffer, 0, 46); // zero is a nice default for most header fields

        const char head[] =
        {
            'P', 'K', 1, 2, // central file header signature
            0x14, 0,        // version made by
            0x14, 0         // version needed to extract
        };

	// I do not know why memcpy is not working here
        //memcpy(buffer, head, sizeof(head));
        qmemmove(buffer, head, sizeof(head));

        if ( it.current()->encoding() == 8 )
        {
            buffer[ 8 ] = 8, // general purpose bit flag, deflated
            buffer[ 10 ] = 8; // compression method, deflated
        }

        transformToMsDos( it.current()->datetime(), &buffer[ 12 ] );

        uLong mycrc = it.current()->crc32();
        buffer[ 16 ] = char(mycrc); // crc checksum
        buffer[ 17 ] = char(mycrc >> 8);
        buffer[ 18 ] = char(mycrc >> 16);
        buffer[ 19 ] = char(mycrc >> 24);

        int mysize1 = it.current()->compressedSize();
        buffer[ 20 ] = char(mysize1); // compressed file size
        buffer[ 21 ] = char(mysize1 >> 8);
        buffer[ 22 ] = char(mysize1 >> 16);
        buffer[ 23 ] = char(mysize1 >> 24);

        int mysize = it.current()->size();
        buffer[ 24 ] = char(mysize); // uncompressed file size
        buffer[ 25 ] = char(mysize >> 8);
        buffer[ 26 ] = char(mysize >> 16);
        buffer[ 27 ] = char(mysize >> 24);

        buffer[ 28 ] = char(it.current()->path().length()); // filename length
        buffer[ 29 ] = char(it.current()->path().length() >> 8);

        int myhst = it.current()->headerStart();
        buffer[ 42 ] = char(myhst); //relative offset of local header
        buffer[ 43 ] = char(myhst >> 8);
        buffer[ 44 ] = char(myhst >> 16);
        buffer[ 45 ] = char(myhst >> 24);

        // file name
        strncpy( buffer + 46, path, path.length() );
	//kdDebug(7040) << "closearchive length to write: " << bufferSize << endl;
        crc = crc32(crc, (Bytef *)buffer, bufferSize );
        device()->writeBlock( buffer, bufferSize );
        delete[] buffer;
    }
    Q_LONG centraldirendoffset = device()->at();
    //kdDebug(7040) << "closearchive: centraldirendoffset: " << centraldirendoffset << endl;
    //kdDebug(7040) << "closearchive: device()->at(): " << device()->at() << endl;

    //write end of central dir record.
    buffer[ 0 ] = 'P'; //end of central dir signature
    buffer[ 1 ] = 'K';
    buffer[ 2 ] = 5;
    buffer[ 3 ] = 6;

    buffer[ 4 ] = 0; // number of this disk
    buffer[ 5 ] = 0;

    buffer[ 6 ] = 0; // number of disk with start of central dir
    buffer[ 7 ] = 0;

    int count = d->m_fileList.count();
    //kdDebug(7040) << "number of files (count): " << count << endl;


    buffer[ 8 ] = char(count); // total number of entries in central dir of
    buffer[ 9 ] = char(count >> 8); // this disk

    buffer[ 10 ] = buffer[ 8 ]; // total number of entries in the central dir
    buffer[ 11 ] = buffer[ 9 ];

    int cdsize = centraldirendoffset - centraldiroffset;
    buffer[ 12 ] = char(cdsize); // size of the central dir
    buffer[ 13 ] = char(cdsize >> 8);
    buffer[ 14 ] = char(cdsize >> 16);
    buffer[ 15 ] = char(cdsize >> 24);

    //kdDebug(7040) << "end : centraldiroffset: " << centraldiroffset << endl;
    //kdDebug(7040) << "end : centraldirsize: " << cdsize << endl;

    buffer[ 16 ] = char(centraldiroffset); // central dir offset
    buffer[ 17 ] = char(centraldiroffset >> 8);
    buffer[ 18 ] = char(centraldiroffset >> 16);
    buffer[ 19 ] = char(centraldiroffset >> 24);

    buffer[ 20 ] = 0; //zipfile comment length
    buffer[ 21 ] = 0;

    device()->writeBlock( buffer, 22);

    //kdDebug(7040) << "kzip.cpp reached." << endl;
    return true;
}

// Reimplemented to replace device()->writeBlock with writeData
bool KZip::writeFile( const QString& name, const QString& user, const QString& group, uint size, const char* data )
{
    // set right offset in zip.
    device()->at( d->m_offset );
    if ( !prepareWriting( name, user, group, size ) )
    {
        kdWarning() << "KZip::writeFile prepareWriting failed" << endl;
        return false;
    }

    // Write data
    if ( data && size && !writeData( data, size ) )
    {
        kdWarning() << "KZip::writeFile writeData failed" << endl;
        return false;
    }

    if ( ! doneWriting( size ) )
    {
        kdWarning() << "KZip::writeFile doneWriting failed" << endl;
        return false;
    }
    // update saved offset for appending new files
    d->m_offset = device()->at();
    return true;
}

bool KZip::prepareWriting( const QString& name, const QString& user, const QString& group, uint /*size*/ )
{
    //kdDebug(7040) << "prepareWriting reached." << endl;
    if ( !isOpened() )
    {
        qWarning( "KZip::writeFile: You must open the zip file before writing to it\n");
        return false;
    }

    if ( ! ( mode() & IO_WriteOnly ) ) // accept WriteOnly and ReadWrite
    {
        qWarning( "KZip::writeFile: You must open the zip file for writing\n");
        return false;
    }

    // delete entries in the filelist with the same filename as the one we want
    // to save, so that we don´t have duplicate file entries when viewing the zip
    // with konqi...
    // CAUTION: the old file itself is still in the zip and won´t be removed !!!
    QPtrListIterator<KZipFileEntry> it( d->m_fileList );

	//kdDebug(7040) << "filename to write: " << name <<endl;
    for ( ; it.current() ; ++it )
    {
    	//kdDebug(7040) << "prepfilename: " << it.current()->path() <<endl;
		if (name == it.current()->path() )
        {
	    	//kdDebug(7040) << "removing following entry: " << it.current()->path() <<endl;
	        d->m_fileList.remove();
        }

    }
    // Find or create parent dir
    KArchiveDirectory* parentDir = rootDir();
    QString fileName( name );
    int i = name.findRev( '/' );
    if ( i != -1 )
    {
        QString dir = name.left( i );
        fileName = name.mid( i + 1 );
        //kdDebug(7040) << "KZip::prepareWriting ensuring " << dir << " exists. fileName=" << fileName << endl;
        parentDir = findOrCreate( dir );
    }

    int time = getActualTime();

    // construct a KZipFileEntry and add it to list
    KZipFileEntry * e = new KZipFileEntry( this, fileName, 0777, time, user, group, QString::null,
                                           name, device()->at() + 30 + name.length(), // start
                                           0 /*size unknown yet*/, d->m_compression, 0 /*csize unknown yet*/ );
    e->setHeaderStart( device()->at() );
    //kdDebug(7040) << "wrote file start: " << e->position() << " name: " << name << endl;
    parentDir->addEntry( e );

    d->m_currentFile = e;
    d->m_fileList.append( e );

    // write out zip header
    QCString encodedName = QFile::encodeName(name);
    int bufferSize = encodedName.length() + 30;
    //kdDebug(7040) << "KZip::prepareWriting bufferSize=" << bufferSize << endl;
    char* buffer = new char[ bufferSize ];

    buffer[ 0 ] = 'P'; //local file header signature
    buffer[ 1 ] = 'K';
    buffer[ 2 ] = 3;
    buffer[ 3 ] = 4;

    buffer[ 4 ] = 0x14; // version needed to extract
    buffer[ 5 ] = 0;

    buffer[ 6 ] = 0; // general purpose bit flag
    buffer[ 7 ] = 0;

    buffer[ 8 ] = char(e->encoding()); // compression method
    buffer[ 9 ] = char(e->encoding() >> 8);

    transformToMsDos( e->datetime(), &buffer[ 10 ] );

    buffer[ 14 ] = 'C'; //dummy crc
    buffer[ 15 ] = 'R';
    buffer[ 16 ] = 'C';
    buffer[ 17 ] = 'q';

    buffer[ 18 ] = 'C'; //compressed file size
    buffer[ 19 ] = 'S';
    buffer[ 20 ] = 'I';
    buffer[ 21 ] = 'Z';

    buffer[ 22 ] = 'U'; //uncompressed file size
    buffer[ 23 ] = 'S';
    buffer[ 24 ] = 'I';
    buffer[ 25 ] = 'Z';

    buffer[ 26 ] = (uchar)(encodedName.length()); //filename length
    buffer[ 27 ] = (uchar)(encodedName.length() >> 8);

    buffer[ 28 ] = 0; // extra field length
    buffer[ 29 ] = 0;

    // file name
    strncpy( buffer + 30, encodedName, encodedName.length() );

    // Write header
    bool b = (device()->writeBlock( buffer, bufferSize ) == bufferSize );
    d->m_crc = 0L;
    delete[] buffer;

    Q_ASSERT( b );
    if (!b)
        return false;

    // Prepare device for writing the data
    // Either device() if no compression, or a KFilterDev to compress
    if ( d->m_compression == 0 ) {
        d->m_currentDev = device();
        return true;
    }

    d->m_currentDev = KFilterDev::device( device(), "application/x-gzip", false );
    Q_ASSERT( d->m_currentDev );
    if ( !d->m_currentDev )
        return false; // ouch
    static_cast<KFilterDev *>(d->m_currentDev)->setSkipHeaders(); // Just zlib, not gzip

    b = d->m_currentDev->open( IO_WriteOnly );
    Q_ASSERT( b );
    return b;
}

bool KZip::doneWriting( uint size )
{
    if ( d->m_currentFile->encoding() == 8 ) {
        // Finish
        (void)d->m_currentDev->writeBlock( 0, 0 );
        delete d->m_currentDev;
    }
    // If 0, d->m_currentDev was device() - don't delete ;)
    d->m_currentDev = 0L;

    Q_ASSERT( d->m_currentFile );
    //kdDebug(7040) << "donewriting reached." << endl;
    //kdDebug(7040) << "filename: " << d->m_currentFile->path() << endl;
    //kdDebug(7040) << "getpos (at): " << device()->at() << endl;
    d->m_currentFile->setSize(size);
    int csize = device()->at() -
        d->m_currentFile->headerStart() - 30 -
		d->m_currentFile->path().length();
    d->m_currentFile->setCompressedSize(csize);
    //kdDebug(7040) << "usize: " << d->m_currentFile->size() << endl;
    //kdDebug(7040) << "csize: " << d->m_currentFile->compressedSize() << endl;
    //kdDebug(7040) << "headerstart: " << d->m_currentFile->headerStart() << endl;

    //kdDebug(7040) << "crc: " << d->m_crc << endl;
    d->m_currentFile->setCRC32( d->m_crc );

    d->m_currentFile = 0L;
    return true;
}

void KZip::virtual_hook( int id, void* data )
{
  KArchive::virtual_hook( id, data );
}

bool KZip::writeData(const char * c, uint i)
{
    Q_ASSERT( d->m_currentFile );
    Q_ASSERT( d->m_currentDev );
    if (!d->m_currentFile || !d->m_currentDev)
        return false;

    // crc to be calculated over uncompressed stuff...
    // and they didn't mention it in their docs...
    d->m_crc = crc32(d->m_crc, (const Bytef *) c , i);

    Q_LONG written = d->m_currentDev->writeBlock( c, i );
    //kdDebug(7040) << "KZip::writeData wrote " << i << " bytes." << endl;
    Q_ASSERT( written == (Q_LONG)i );
    return written == (Q_LONG)i;
}

void KZip::setCompression( Compression c )
{
    d->m_compression = ( c == NoCompression ) ? 0 : 8;
}

KZip::Compression KZip::compression() const
{
   return ( d->m_compression == 8 ) ? DeflateCompression : NoCompression;
}

///////////////

QByteArray KZipFileEntry::data() const
{
    QIODevice* dev = device();
    QByteArray arr = dev->readAll();
    delete dev;
    return arr;
}

QIODevice* KZipFileEntry::device() const
{
    //kdDebug(7040) << "KZipFileEntry::device creating iodevice limited to pos=" << position() << ", csize=" << compressedSize() << endl;
    // Limit the reading to the appropriate part of the underlying device (e.g. file)
    KLimitedIODevice* limitedDev = new KLimitedIODevice( archive()->device(), position(), compressedSize() );
    if ( encoding() == 0 || compressedSize() == 0 ) // no compression (or even no data)
        return limitedDev;

    if ( encoding() == 8 )
    {
        // On top of that, create a device that uncompresses the zlib data
        QIODevice* filterDev = KFilterDev::device( limitedDev, "application/x-gzip" );
        if ( !filterDev )
            return 0L; // ouch
        static_cast<KFilterDev *>(filterDev)->setSkipHeaders(); // Just zlib, not gzip
        bool b = filterDev->open( IO_ReadOnly );
        Q_ASSERT( b );
        return filterDev;
    }

    kdError() << "This zip file contains files compressed with method "
              << encoding() <<", this method is currently not supported by KZip,"
              <<" please use a command-line tool to handle this file." << endl;
    return 0L;
}

