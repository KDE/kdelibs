/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

//#include <stdio.h>
#include <stdlib.h> // strtol
#include <time.h> // time()
/*#include <unistd.h>
#include <grp.h>
#include <pwd.h>*/
#include <assert.h>

#include <qcstring.h>
#include <qdir.h>
#include <qfile.h>
#include <kdebug.h>
#include <kurl.h>
#include <kmimetype.h>

#include <kfilterdev.h>
#include <kfilterbase.h>

#include "ktar.h"

////////////////////////////////////////////////////////////////////////
/////////////////////////// KTar ///////////////////////////////////
////////////////////////////////////////////////////////////////////////

class KTar::KTarPrivate
{
public:
    KTarPrivate() {}
    QStringList dirList;
};

KTar::KTar( const QString& filename, const QString & _mimetype )
    : KArchive( 0L )
{
    m_filename = filename;
    d = new KTarPrivate;
    QString mimetype( _mimetype );
    bool forced = true;
    if ( mimetype.isEmpty() )
    {
	if ( QFile::exists( filename ) )
            mimetype = KMimeType::findByFileContent( filename )->name();
	else
	    mimetype = KMimeType::findByPath( filename, 0, true )->name();
        kdDebug() << "KTar::KTar mimetype=" << mimetype << endl;

        // Don't move to prepareDevice - the other constructor theoretically allows ANY filter
        if ( mimetype == "application/x-tgz" || mimetype == "application/x-targz" || // the latter is deprecated but might still be around
             mimetype == "application/x-webarchive" )
            // that's a gzipped tar file, so ask for gzip filter
            mimetype = "application/x-gzip";
        else if ( mimetype == "application/x-tbz" ) // that's a bzipped2 tar file, so ask for bz2 filter
            mimetype = "application/x-bzip2";
        else
        {
            // Something else. Check if it's not really gzip though (e.g. for KOffice docs)
            QFile file( filename );
            if ( file.open( IO_ReadOnly ) )
            {
                unsigned char firstByte = file.getch();
                unsigned char secondByte = file.getch();
                unsigned char thirdByte = file.getch();
                if ( firstByte == 0037 && secondByte == 0213 )
                    mimetype = "application/x-gzip";
                else if ( firstByte == 'B' && secondByte == 'Z' && thirdByte == 'h' )
                    mimetype = "application/x-bzip2";
                else if ( firstByte == 'P' && secondByte == 'K' && thirdByte == 3 )
                {
                    unsigned char fourthByte = file.getch();
                    if ( fourthByte == 4 )
                        mimetype = "application/x-zip";
                }
            }
        }
        forced = false;
    }

    prepareDevice( filename, mimetype, forced );
}

void KTar::prepareDevice( const QString & filename,
                            const QString & mimetype, bool forced )
{
  if( "application/x-tar" == mimetype )
      setDevice( new QFile( filename ) );
  else
  {
    if( "application/x-gzip" == mimetype
       || "application/x-bzip2" == mimetype)
        forced = true;

    QIODevice *dev = KFilterDev::deviceForFile( filename, mimetype, forced );
    if( dev )
      setDevice( dev );
  }
}

KTar::KTar( QIODevice * dev )
    : KArchive( dev )
{
    d = new KTarPrivate;
}

KTar::~KTar()
{
    // mjarrett: Closes to prevent ~KArchive from aborting w/o device
    if( isOpened() )
        close();
    if ( !m_filename.isEmpty() )
        delete device(); // we created it ourselves
    delete d;
}

void KTar::setOrigFileName( const QCString & fileName )
{
    if ( !isOpened() || !(mode() & IO_WriteOnly) )
    {
        kdWarning() << "KTar::setOrigFileName: File must be opened for writing first.\n";
        return;
    }
    static_cast<KFilterDev *>(device())->setOrigFileName( fileName );
}


bool KTar::openArchive( int mode )
{
    if ( !(mode & IO_ReadOnly) )
        return true;

    // We'll use the permission and user/group of d->rootDir
    // for any directory we emulate (see findOrCreate)
    //struct stat buf;
    //stat( m_filename, &buf );

    d->dirList.clear();
    QIODevice* dev = device();

    // read dir infos
    char buffer[ 0x200 ];
    bool ende = false;
    do
    {
        // Read header
        int n = dev->readBlock( buffer, 0x200 );
        if ( n == 0x200 && buffer[0] != 0 )
        {
            // Make sure this is actually a tar header
            if (strncmp(buffer + 257, "ustar", 5)) 
            {
                // The magic isn't there (broken/old tars), but maybe a correct checksum?
                QCString s;

                int check = 0;
                for( uint j = 0; j < 0x200; ++j )
                    check += buffer[j];
                
                // adjust checksum to count the checksum fields as blanks
                for( uint j = 0; j < 8 /*size of the checksum field including the \0 and the space*/; j++ )
                    check -= buffer[148 + j];
                check += 8 * ' ';

                s.sprintf("%o", check ); 

                // only compare those of the 6 checksum digits that mean something,
                // because the other digits are filled with all sorts of different chars by different tars ...
                if( strncmp( buffer + 148 + 6 - s.length(), s.data(), s.length() ) )
                {
                    kdWarning() << "KTar: invalid TAR file. Header is: " << QCString( buffer+257, 5 ) << endl;
                    return false;
                }
	    }

            QString name( QString::fromLocal8Bit(buffer) );

            // If filename is longer than 100 (0x64) chars, then tar uses ././@LongLink (David)
            if ( name == "././@LongLink" )
            {
                // in this case, here's what happens (according to od -cx !)
                // 1) the filename is stored in the next 512b buffer
                n = dev->readBlock( buffer, 0x200 );
                if ( n == 0x200 && buffer[0] != 0 )
                {
                    name = QString::fromLocal8Bit(buffer);
                    // 2) read yet another 512b buffer, for permissions, time, size ...
                    n = dev->readBlock( buffer, 0x200 );
                    if (!( n == 0x200 && buffer[0] != 0 ))
                        break;
                }
                else
                    break;
            }

            bool isdir = false;
            QString nm;

            if ( name.right(1) == "/" )
            {
                isdir = true;
                name = name.left( name.length() - 1 );
            }

            int pos = name.findRev( '/' );
            if ( pos == -1 )
                nm = name;
            else
                nm = name.mid( pos + 1 );

            // read access
            buffer[ 0x6b ] = 0;
            char *dummy;
            const char* p = buffer + 0x64;
            while( *p == ' ' ) ++p;
            int access = (int)strtol( p, &dummy, 8 );

            // read user and group
            QString user( buffer + 0x109 );
            QString group( buffer + 0x129 );
            // read symlink dest (if any)
            QString symlink(buffer + 0x9d );

            // read time
            buffer[ 0x93 ] = 0;
            p = buffer + 0x88;
            while( *p == ' ' ) ++p;
            int time = (int)strtol( p, &dummy, 8 );

            // read type flag
            char typeflag = buffer[ 0x9c ];
            // '0' for files, '1' hard link, '2' symlink, '5' for directory (and 'L' for longlink)
            // and 'D' for GNU tar extension DUMPDIR
            if ( typeflag == '1' )
                isdir = true;

            bool isDumpDir = false;
            if ( typeflag == 'D' )
            {
                isdir = false;
                isDumpDir = true;
            }
            //bool islink = ( typeflag == '1' || typeflag == '2' );
            //kdDebug() << "typeflag=" << typeflag << " islink=" << islink << endl;

            if (isdir)
                access |= S_IFDIR; // f*cking broken tar files

            KArchiveEntry* e;
            if ( isdir )
            {
                //kdDebug() << "KArchive::open directory " << nm << endl;
                e = new KArchiveDirectory( this, nm, access, time, user, group, symlink );
            }
            else
            {
                // read size
                buffer[ 0x88 ] = 0; // was 0x87, but 0x88 fixes BR #26437
                char *dummy;
                const char* p = buffer + 0x7c;
                while( *p == ' ' ) ++p;
                int size = (int)strtol( p, &dummy, 8 );

                // for isDumpDir we will skip the additional info about that dirs contents 
                if ( isDumpDir )
                {
		    e = new KArchiveDirectory( this, nm, access, time, user, group, symlink );
                }
		else
		{
            
                    // Let's hack around hard links. Our classes don't support that, so make them symlinks
                    if ( typeflag == '1' )
                    {
                        size = nm.length(); // in any case, we don't want to skip the real size, hence this resetting of size
                        kdDebug() << "HARD LINK, setting size to " << size << endl;
                    }

                    //kdDebug() << "KArchive::open file " << nm << " size=" << size << endl;

                    e = new KArchiveFile( this, nm, access, time, user, group, symlink,
                                          dev->at(), size );
		}			  

                // Skip contents + align bytes
                int rest = size % 0x200;
                int skip = size + (rest ? 0x200 - rest : 0);
                //kdDebug() << "KArchive::open, at()=" << dev->at() << " rest=" << rest << " skipping " << skip << endl;
                if (! dev->at( dev->at() + skip ) )
                    kdWarning() << "KArchive::open skipping " << skip << " failed" << endl;
            }

            if ( pos == -1 )
            {
                if ( nm == "." ) // special case
                {
                    Q_ASSERT( isdir );
                    if ( isdir )
                        setRootDir( static_cast<KArchiveDirectory *>( e ) );
                }
                else
                    rootDir()->addEntry( e );
            }
            else
            {
                // In some tar files we can find dir/./file => call cleanDirPath
                QString path = QDir::cleanDirPath( name.left( pos ) );
                // Ensure container directory exists, create otherwise
                KArchiveDirectory * d = findOrCreate( path );
                d->addEntry( e );
            }
        }
        else
        {
            //qDebug("Terminating. Read %d bytes, first one is %d", n, buffer[0]);
            ende = true;
        }
    } while( !ende );
    return true;
}

bool KTar::closeArchive()
{
    d->dirList.clear();
    return true;
}

bool KTar::writeDir( const QString& name, const QString& user, const QString& group )
{
    if ( !isOpened() )
    {
        kdWarning() << "KArchive::writeDir: You must open the tar file before writing to it\n";
        return false;
    }

    if ( !(mode() & IO_WriteOnly) )
    {
        kdWarning() << "KArchive::writeDir: You must open the tar file for writing\n";
        return false;
    }

    // In some tar files we can find dir/./ => call cleanDirPath
    QString dirName ( QDir::cleanDirPath( name ) );

    // Need trailing '/'
    if ( dirName.right(1) != "/" )
        dirName += "/";

    if ( d->dirList.contains( dirName ) )
        return true; // already there

    char buffer[ 0x201 ];
    memset( buffer, 0, 0x200 );

    // If more than 100 chars, we need to use the LongLink trick
    if ( dirName.length() > 99 )
    {
        strcpy( buffer, "././@LongLink" );
        fillBuffer( buffer, "     0", dirName.length()+1, 'L', user.local8Bit(), group.local8Bit() );
        device()->writeBlock( buffer, 0x200 );
        memset( buffer, 0, 0x200 );
        strcpy( buffer, QFile::encodeName(dirName) );
        // write long name
        device()->writeBlock( buffer, 0x200 );
        // not even needed to reclear the buffer, tar doesn't do it
    }
    else
        // Write name
        strcpy( buffer, QFile::encodeName(dirName) );

    fillBuffer( buffer, " 40755", 0, 0x35, user.local8Bit(), group.local8Bit());

    // Write header
    device()->writeBlock( buffer, 0x200 );

    d->dirList.append( dirName ); // contains trailing slash
    return true; // TODO if wanted, better error control
}

bool KTar::prepareWriting( const QString& name, const QString& user, const QString& group, uint size )
{
    if ( !isOpened() )
    {
        kdWarning() << "KArchive::writeFile: You must open the tar file before writing to it\n";
        return false;
    }

    if ( !(mode() & IO_WriteOnly) )
    {
        kdWarning() << "KArchive::writeFile: You must open the tar file for writing\n";
        return false;
    }

    // In some tar files we can find dir/./file => call cleanDirPath
    QString fileName ( QDir::cleanDirPath( name ) );

    /*
      // Create toplevel dirs
      // Commented out by David since it's not necessary, and if anybody thinks it is,
      // he needs to implement a findOrCreate equivalent in writeDir.
      // But as KTar and the "tar" program both handle tar files without
      // dir entries, there's really no need for that
      QString tmp ( fileName );
      int i = tmp.findRev( '/' );
      if ( i != -1 )
      {
      QString d = tmp.left( i + 1 ); // contains trailing slash
      if ( !m_dirList.contains( d ) )
      {
      tmp = tmp.mid( i + 1 );
      writeDir( d, user, group ); // WARNING : this one doesn't create its toplevel dirs
      }
      }
    */

    char buffer[ 0x201 ];
    memset( buffer, 0, 0x200 );

    // If more than 100 chars, we need to use the LongLink trick
    if ( fileName.length() > 99 )
    {
        strcpy( buffer, "././@LongLink" );
        fillBuffer( buffer, "     0", fileName.length()+1, 'L', user.local8Bit(), group.local8Bit() );
        device()->writeBlock( buffer, 0x200 );

        memset( buffer, 0, 0x200 );
        strcpy( buffer, QFile::encodeName(fileName) );
        // write long name
        device()->writeBlock( buffer, 0x200 );
        // not even needed to reclear the buffer, tar doesn't do it
    }
    else
        // Write name
        strcpy( buffer, QFile::encodeName(fileName) );

    fillBuffer( buffer, "100644", size, 0x30, user.local8Bit(), group.local8Bit() );

    // Write header
    return device()->writeBlock( buffer, 0x200 ) == 0x200;
}

bool KTar::doneWriting( uint size )
{
    // Write alignment
    int rest = size % 0x200;
    if ( rest )
    {
        char buffer[ 0x201 ];
        for( uint i = 0; i < 0x200; ++i )
            buffer[i] = 0;
        Q_LONG nwritten = device()->writeBlock( buffer, 0x200 - rest );
        return nwritten == 0x200 - rest;
    }
    return true;
}

/*** Some help from the tar sources
struct posix_header
{                               byte offset
  char name[100];               *   0 *     0x0
  char mode[8];                 * 100 *     0x64
  char uid[8];                  * 108 *     0x6c
  char gid[8];                  * 116 *     0x74
  char size[12];                * 124 *     0x7c
  char mtime[12];               * 136 *     0x88
  char chksum[8];               * 148 *     0x94
  char typeflag;                * 156 *     0x9c
  char linkname[100];           * 157 *     0x9d
  char magic[6];                * 257 *     0x101
  char version[2];              * 263 *     0x107
  char uname[32];               * 265 *     0x109
  char gname[32];               * 297 *     0x129
  char devmajor[8];             * 329 *     0x149
  char devminor[8];             * 337 *     ...
  char prefix[155];             * 345 *
                                * 500 *
};
*/

void KTar::fillBuffer( char * buffer,
    const char * mode, int size, char typeflag, const char * uname, const char * gname )
{
  // mode (as in stat())
  assert( strlen(mode) == 6 );
  strcpy( buffer+0x64, mode );
  buffer[ 0x6a ] = ' ';
  buffer[ 0x6b ] = '\0';

  // dummy uid
  strcpy( buffer + 0x6c, "   765 ");
  // dummy gid
  strcpy( buffer + 0x74, "   144 ");

  // size
  QCString s;
  s.sprintf("%o", size); // OCT
  s = s.rightJustify( 11, ' ' );
  strcpy( buffer + 0x7c, s.data() );
  buffer[ 0x87 ] = ' '; // space-terminate (no null after)

  // Dummy time
  s.sprintf("%lo", time( 0 ) ); // OCT
  s = s.rightJustify( 11, ' ' );
  strcpy( buffer + 0x88, s.data() );
  buffer[ 0x93 ] = ' '; // space-terminate (no null after)

  // spaces, replaced by the check sum later
  buffer[ 0x94 ] = 0x20;
  buffer[ 0x95 ] = 0x20;
  buffer[ 0x96 ] = 0x20;
  buffer[ 0x97 ] = 0x20;
  buffer[ 0x98 ] = 0x20;
  buffer[ 0x99 ] = 0x20;

  /* From the tar sources :
     Fill in the checksum field.  It's formatted differently from the
     other fields: it has [6] digits, a null, then a space -- rather than
     digits, a space, then a null. */

  buffer[ 0x9a ] = '\0';
  buffer[ 0x9b ] = ' ';

  // type flag (dir, file, link)
  buffer[ 0x9c ] = typeflag;

 // magic + version
  strcpy( buffer + 0x101, "ustar");
  strcpy( buffer + 0x107, "00" );

  // user
  strcpy( buffer + 0x109, uname );
  // group
  strcpy( buffer + 0x129, gname );

  // Header check sum
  int check = 32;
  for( uint j = 0; j < 0x200; ++j )
    check += buffer[j];
  s.sprintf("%o", check ); // OCT
  s = s.rightJustify( 7, ' ' );
  strcpy( buffer + 0x94, s.data() );
}

void KTar::virtual_hook( int id, void* data )
{ KArchive::virtual_hook( id, data ); }

