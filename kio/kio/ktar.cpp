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

//#include <stdio.h>
#include <stdlib.h> // strtol
#include <time.h> // time()
/*#include <unistd.h>
#include <grp.h>
#include <pwd.h>*/
#include <assert.h>

#include <q3cstring.h>
#include <qdir.h>
#include <qfile.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <ktempfile.h>

#include <kfilterdev.h>
#include <kfilterbase.h>

#include "ktar.h"
#include <kstandarddirs.h>

////////////////////////////////////////////////////////////////////////
/////////////////////////// KTar ///////////////////////////////////
////////////////////////////////////////////////////////////////////////

class KTar::KTarPrivate
{
public:
    KTarPrivate() : tarEnd( 0 ), tmpFile( 0 ) {}
    QStringList dirList;
    int tarEnd;
    KTempFile* tmpFile;
    QString mimetype;
    QByteArray origFileName;

    bool fillTempFile(const QString & filename);
    bool writeBackTempFile( const QString & filename );
};

KTar::KTar( const QString& filename, const QString & _mimetype )
    : KArchive( 0 )
{
    m_filename = filename;
    d = new KTarPrivate;
    QString mimetype( _mimetype );
    bool forced = true;
    if ( mimetype.isEmpty() ) // Find out mimetype manually
    {
        if ( QFile::exists( filename ) )
            mimetype = KMimeType::findByFileContent( filename )->name();
        else
            mimetype = KMimeType::findByPath( filename, 0, true )->name();
        kdDebug(7041) << "KTar::KTar mimetype = " << mimetype << endl;

        // Don't move to prepareDevice - the other constructor theoretically allows ANY filter
        if ( mimetype == "application/x-tgz" || mimetype == "application/x-targz" || // the latter is deprecated but might still be around
             mimetype == "application/x-webarchive" )
        {
            // that's a gzipped tar file, so ask for gzip filter
            mimetype = "application/x-gzip";
        }
        else if ( mimetype == "application/x-tbz" ) // that's a bzipped2 tar file, so ask for bz2 filter
        {
            mimetype = "application/x-bzip2";
        }
        else
        {
            // Something else. Check if it's not really gzip though (e.g. for KOffice docs)
            QFile file( filename );
            if ( file.open( QIODevice::ReadOnly ) )
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
            file.close();
        }
        forced = false;
    }
    d->mimetype = mimetype;

    prepareDevice( filename, mimetype, forced );
}

void KTar::prepareDevice( const QString & filename,
                            const QString & mimetype, bool /*forced*/ )
{
  if( "application/x-tar" == mimetype )
      setDevice( new QFile( filename ) );
  else
  {
    // The compression filters are very slow with random access.
    // So instead of applying the filter to the device,
    // the file is completly extracted instead,
    // and we work on the extracted tar file.
    // This improves the extraction speed by the tar ioslave dramatically,
    // if the archive file contains many files.
    // This is because the tar ioslave extracts one file after the other and normally
    // has to walk through the decompression filter each time.
    // Which is in fact nearly as slow as a complete decompression for each file.
    d->tmpFile = new KTempFile(locateLocal("tmp", "ktar-"),".tar");
    kdDebug( 7041 ) << "KTar::prepareDevice creating TempFile: " << d->tmpFile->name() << endl;
    d->tmpFile->setAutoDelete(true);

    // KTempFile opens the file automatically,
    // the device must be closed, however, for KArchive.setDevice()
    QFile* file = d->tmpFile->file();
    file->close();
    setDevice(file);
  }
}

KTar::KTar( QIODevice * dev )
    : KArchive( dev )
{
    Q_ASSERT( dev );
    d = new KTarPrivate;
}

KTar::~KTar()
{
    // mjarrett: Closes to prevent ~KArchive from aborting w/o device
    if( isOpened() )
        close();

    if (d->tmpFile)
        delete d->tmpFile; // will delete the device
    else if ( !m_filename.isEmpty() )
        delete device(); // we created it ourselves


    delete d;
}

void KTar::setOrigFileName( const QByteArray & fileName )
{
    if ( !isOpened() || !(mode() & QIODevice::WriteOnly) )
    {
        kdWarning(7041) << "KTar::setOrigFileName: File must be opened for writing first.\n";
        return;
    }
    d->origFileName = fileName;
}

Q_LONG KTar::readRawHeader(char *buffer) {
  // Read header
  Q_LONG n = device()->readBlock( buffer, 0x200 );
  if ( n == 0x200 && buffer[0] != 0 ) {
    // Make sure this is actually a tar header
    if (strncmp(buffer + 257, "ustar", 5)) {
      // The magic isn't there (broken/old tars), but maybe a correct checksum?

      int check = 0;
      for( uint j = 0; j < 0x200; ++j )
        check += buffer[j];

      // adjust checksum to count the checksum fields as blanks
      for( uint j = 0; j < 8 /*size of the checksum field including the \0 and the space*/; j++ )
        check -= buffer[148 + j];
      check += 8 * ' ';

      QByteArray s = QByteArray::number( check, 8 ); // octal

      // only compare those of the 6 checksum digits that mean something,
      // because the other digits are filled with all sorts of different chars by different tars ...
      if( strncmp( buffer + 148 + 6 - s.length(), s.data(), s.length() ) ) {
        kdWarning(7041) << "KTar: invalid TAR file. Header is: " << QByteArray( buffer+257, 5 ) << endl;
        return -1;
      }
    }/*end if*/
  } else {
    // reset to 0 if 0x200 because logical end of archive has been reached
    if (n == 0x200) n = 0;
  }/*end if*/
  return n;
}

bool KTar::readLonglink(char *buffer,QByteArray &longlink) {
  Q_LONG n = 0;
  QIODevice *dev = device();
  // read size of longlink from size field in header
  // size is in bytes including the trailing null (which we ignore)
  buffer[ 0x88 ] = 0; // was 0x87, but 0x88 fixes BR #26437
  char *dummy;
  const char* p = buffer + 0x7c;
  while( *p == ' ' ) ++p;
  int size = (int)strtol( p, &dummy, 8 );

  longlink.resize(size);
  size--;    // ignore trailing null
  dummy = longlink.data();
  int offset = 0;
  while (size > 0) {
    int chunksize = QMIN(size, 0x200);
    n = dev->readBlock( dummy + offset, chunksize );
    if (n == -1) return false;
    size -= chunksize;
    offset += 0x200;
  }/*wend*/
  // jump over the rest
  int skip = 0x200 - (n % 0x200);
  if (skip < 0x200) {
    if (dev->readBlock(buffer,skip) != skip) return false;
  }
  return true;
}

Q_LONG KTar::readHeader(char *buffer,QString &name,QString &symlink) {
  name.truncate(0);
  symlink.truncate(0);
  while (true) {
    Q_LONG n = readRawHeader(buffer);
    if (n != 0x200) return n;

    // is it a longlink?
    if (strcmp(buffer,"././@LongLink") == 0) {
      char typeflag = buffer[0x9c];
      QByteArray longlink;
      readLonglink(buffer,longlink);
      switch (typeflag) {
        case 'L': name = QFile::decodeName(longlink); break;
        case 'K': symlink = QFile::decodeName(longlink); break;
      }/*end switch*/
    } else {
      break;
    }/*end if*/
  }/*wend*/

  // if not result of longlink, read names directly from the header
  if (name.isEmpty())
    // there are names that are exactly 100 bytes long
    // and neither longlink nor \0 terminated (bug:101472)
    name = QFile::decodeName(QByteArray(buffer, 101));
  if (symlink.isEmpty())
    symlink = QFile::decodeName(QByteArray(buffer + 0x9d, 101));

  return 0x200;
}

/*
 * If we have created a temporary file, we have
 * to decompress the original file now and write
 * the contents to the temporary file.
 */
bool KTar::KTarPrivate::fillTempFile( const QString & filename) {
    if ( ! tmpFile )
        return true;

    kdDebug( 7041 ) <<
        "KTar::openArchive: filling tmpFile of mimetype '" << mimetype <<
        "' ... " << endl;

    bool forced = false;
    if( "application/x-gzip" == mimetype
    || "application/x-bzip2" == mimetype)
        forced = true;

    QIODevice *filterDev = KFilterDev::deviceForFile( filename, mimetype, forced );

    if( filterDev ) {
        QFile* file = tmpFile->file();
        file->close();
        if ( ! file->open( QIODevice::WriteOnly ) )
        {
            delete filterDev;
            return false;
        }
        QByteArray buffer(8*1024);
        if ( ! filterDev->open( QIODevice::ReadOnly ) )
        {
            delete filterDev;
            return false;
        }
        Q_LONG len;
        while ( !filterDev->atEnd() ) {
            len = filterDev->readBlock(buffer.data(),buffer.size());
            if ( len <= 0 ) { // corrupted archive
                delete filterDev;
                return false;
            }
            file->writeBlock(buffer.data(),len);
        }
        filterDev->close();
        delete filterDev;

        file->close();
        if ( ! file->open( QIODevice::ReadOnly ) )
            return false;
    }
    else
        kdDebug( 7041 ) << "KTar::openArchive: no filterdevice found!" << endl;

    kdDebug( 7041 ) << "KTar::openArchive: filling tmpFile finished." << endl;
    return true;
}

bool KTar::openArchive( QIODevice::OpenMode mode )
{
    kdDebug( 7041 ) << "KTar::openArchive" << endl;
    if ( !(mode & QIODevice::ReadOnly) )
        return true;

    if ( !d->fillTempFile( m_filename ) )
        return false;

    // We'll use the permission and user/group of d->rootDir
    // for any directory we emulate (see findOrCreate)
    //struct stat buf;
    //stat( m_filename, &buf );

    d->dirList.clear();
    QIODevice* dev = device();

    if ( !dev )
        return false;

    // read dir infos
    char buffer[ 0x200 ];
    bool ende = false;
    do
    {
        QString name;
        QString symlink;

        // Read header
        Q_LONG n = readHeader(buffer,name,symlink);
        if (n < 0) return false;
        if (n == 0x200)
        {
            bool isdir = false;
            QString nm;

            if ( name.endsWith( QLatin1Char( '/' ) ) )
            {
                isdir = true;
                name = name.left( name.length() - 1 );
            }

            int pos = name.lastIndexOf( '/' );
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

            // read time
            buffer[ 0x93 ] = 0;
            p = buffer + 0x88;
            while( *p == ' ' ) ++p;
            int time = (int)strtol( p, &dummy, 8 );

            // read type flag
            char typeflag = buffer[ 0x9c ];
            // '0' for files, '1' hard link, '2' symlink, '5' for directory
            // (and 'L' for longlink filenames, 'K' for longlink symlink targets)
            // and 'D' for GNU tar extension DUMPDIR
            if ( typeflag == '5' )
                isdir = true;

            bool isDumpDir = false;
            if ( typeflag == 'D' )
            {
                isdir = false;
                isDumpDir = true;
            }
            //bool islink = ( typeflag == '1' || typeflag == '2' );
            //kdDebug(7041) << "typeflag=" << typeflag << " islink=" << islink << endl;

            if (isdir)
                access |= S_IFDIR; // f*cking broken tar files

            KArchiveEntry* e;
            if ( isdir )
            {
                //kdDebug(7041) << "KTar::openArchive directory " << nm << endl;
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
                    //kdDebug(7041) << "KTar::openArchive " << nm << " isDumpDir" << endl;
                    e = new KArchiveDirectory( this, nm, access, time, user, group, symlink );
                }
                else
                {

                    // Let's hack around hard links. Our classes don't support that, so make them symlinks
                    if ( typeflag == '1' )
                    {
                        size = nm.length(); // in any case, we don't want to skip the real size, hence this resetting of size
                        kdDebug(7041) << "HARD LINK, setting size to " << size << endl;
                    }

                    //kdDebug(7041) << "KTar::openArchive file " << nm << " size=" << size << endl;
                    e = new KArchiveFile( this, nm, access, time, user, group, symlink,
                                          dev->at(), size );
                }

                // Skip contents + align bytes
                int rest = size % 0x200;
                int skip = size + (rest ? 0x200 - rest : 0);
                //kdDebug(7041) << "KTar::openArchive, at()=" << dev->at() << " rest=" << rest << " skipping " << skip << endl;
                if (! dev->at( dev->at() + skip ) )
                    kdWarning(7041) << "KTar::openArchive skipping " << skip << " failed" << endl;
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
                // In some tar files we can find dir/./file => call cleanPath
                QString path = QDir::cleanPath( name.left( pos ) );
                // Ensure container directory exists, create otherwise
                KArchiveDirectory * d = findOrCreate( path );
                d->addEntry( e );
            }
        }
        else
        {
            //qDebug("Terminating. Read %d bytes, first one is %d", n, buffer[0]);
            d->tarEnd = dev->at() - n; // Remember end of archive
            ende = true;
        }
    } while( !ende );
    return true;
}

/*
 * Writes back the changes of the temporary file
 * to the original file.
 * Must only be called if in QIODevice::WriteOnly mode
 */
bool KTar::KTarPrivate::writeBackTempFile( const QString & filename ) {
    if ( ! tmpFile )
        return true;

    kdDebug(7041) << "Write temporary file to compressed file" << endl;
    kdDebug(7041) << filename << " " << mimetype << endl;

    bool forced = false;
    if( "application/x-gzip" == mimetype
        || "application/x-bzip2" == mimetype)
        forced = true;


    QIODevice *dev = KFilterDev::deviceForFile( filename, mimetype, forced );
    if( dev ) {
        QFile* file = tmpFile->file();
        file->close();
        if ( ! file->open(QIODevice::ReadOnly) || ! dev->open(QIODevice::WriteOnly) )
        {
            file->close();
            delete dev;
            return false;
        }
        if ( forced )
            static_cast<KFilterDev *>(dev)->setOrigFileName( origFileName );
        QByteArray buffer(8*1024);
        Q_LONG len;
        while ( ! file->atEnd()) {
            len = file->readBlock(buffer.data(),buffer.size());
            dev->writeBlock(buffer.data(),len);
        }
        file->close();
        dev->close();
        delete dev;
    }

    kdDebug(7041) << "Write temporary file to compressed file done." << endl;
    return true;
}

bool KTar::closeArchive()
{
    d->dirList.clear();

    // If we are in write mode and had created
    // a temporary tar file, we have to write
    // back the changes to the original file
    if( mode() == QIODevice::WriteOnly)
        return d->writeBackTempFile( m_filename );

    return true;
}

bool KTar::writeDir( const QString& name, const QString& user, const QString& group )
{
    mode_t perm = 040755;
    time_t the_time = time(0);
    return writeDir(name,user,group,perm,the_time,the_time,the_time);
#if 0
    if ( !isOpened() )
    {
        kdWarning(7041) << "KTar::writeDir: You must open the tar file before writing to it\n";
        return false;
    }

    if ( !(mode() & QIODevice::WriteOnly) )
    {
        kdWarning(7041) << "KTar::writeDir: You must open the tar file for writing\n";
        return false;
    }

    // In some tar files we can find dir/./ => call cleanPath
    QString dirName ( QDir::cleanPath( name ) );

    // Need trailing '/'
    if ( !dirName.endsWith( QLatin1Char( '/' ) ) )
        dirName += QLatin1Char( '/' );

    if ( d->dirList.contains( dirName ) )
        return true; // already there

    char buffer[ 0x201 ];
    memset( buffer, 0, 0x200 );
    if ( mode() & QIODevice::ReadWrite ) device()->at(d->tarEnd); // Go to end of archive as might have moved with a read

    // If more than 100 chars, we need to use the LongLink trick
    if ( dirName.length() > 99 )
    {
        strcpy( buffer, "././@LongLink" );
        fillBuffer( buffer, "     0", dirName.length()+1, 'L', user.toLocal8Bit(), group.toLocal8Bit() );
        device()->writeBlock( buffer, 0x200 );
        strncpy( buffer, QFile::encodeName(dirName), 0x200 );
        buffer[0x200] = 0;
        // write long name
        device()->writeBlock( buffer, 0x200 );
        // not even needed to reclear the buffer, tar doesn't do it
    }
    else
    {
        // Write name
        strncpy( buffer, QFile::encodeName(dirName), 0x200 );
        buffer[0x200] = 0;
    }

    fillBuffer( buffer, " 40755", 0, 0x35, user.toLocal8Bit(), group.toLocal8Bit());

    // Write header
    device()->writeBlock( buffer, 0x200 );
    if ( mode() & QIODevice::ReadWrite )  d->tarEnd = device()->at();

    d->dirList.append( dirName ); // contains trailing slash
    return true; // TODO if wanted, better error control
#endif
}

bool KTar::prepareWriting( const QString& name, const QString& user, const QString& group, uint size )
{
    mode_t dflt_perm = 0100644;
    time_t the_time = time(0);
    return prepareWriting(name,user,group,size,dflt_perm,
                          the_time,the_time,the_time);
}

bool KTar::doneWriting( uint size )
{
    // Write alignment
    int rest = size % 0x200;
    if ( mode() & QIODevice::ReadWrite )
        d->tarEnd = device()->at() + (rest ? 0x200 - rest : 0); // Record our new end of archive
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
    const char * mode, int size, time_t mtime, char typeflag,
    const char * uname, const char * gname )
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
  QByteArray s = QByteArray::number( size, 8 ); // octal
  s = s.rightJustified( 11, ' ' );
  strcpy( buffer + 0x7c, s.data() );
  buffer[ 0x87 ] = ' '; // space-terminate (no null after)

  // modification time
  s = QByteArray::number( static_cast<qulonglong>(mtime), 8 ); // octal
  s = s.rightJustified( 11, ' ' );
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
  s = QByteArray::number( check, 8 ); // octal
  s = s.rightJustified( 7, ' ' );
  strcpy( buffer + 0x94, s.data() );
}

void KTar::writeLonglink(char *buffer, const QByteArray &name, char typeflag,
                         const char *uname, const char *gname) {
  strcpy( buffer, "././@LongLink" );
  int namelen = name.length() + 1;
  fillBuffer( buffer, "     0", namelen, 0, typeflag, uname, gname );
  device()->writeBlock( buffer, 0x200 );
  int offset = 0;
  while (namelen > 0) {
    int chunksize = QMIN(namelen, 0x200);
    memcpy(buffer, name.data()+offset, chunksize);
    // write long name
    device()->writeBlock( buffer, 0x200 );
    // not even needed to reclear the buffer, tar doesn't do it
    namelen -= chunksize;
    offset += 0x200;
  }/*wend*/
}

bool KTar::prepareWriting(const QString& name, const QString& user,
                          const QString& group, uint size, mode_t perm,
                          time_t atime, time_t mtime, time_t ctime) {
  return KArchive::prepareWriting(name,user,group,size,perm,atime,mtime,ctime);
}

bool KTar::prepareWriting_impl(const QString &name, const QString &user,
                               const QString &group, uint size, mode_t perm,
                               time_t /*atime*/, time_t mtime, time_t /*ctime*/) {
    if ( !isOpened() )
    {
        kdWarning(7041) << "KTar::prepareWriting: You must open the tar file before writing to it\n";
        return false;
    }

    if ( !(mode() & QIODevice::WriteOnly) )
    {
        kdWarning(7041) << "KTar::prepareWriting: You must open the tar file for writing\n";
        return false;
    }

    // In some tar files we can find dir/./file => call cleanPath
    QString fileName ( QDir::cleanPath( name ) );

    /*
      // Create toplevel dirs
      // Commented out by David since it's not necessary, and if anybody thinks it is,
      // he needs to implement a findOrCreate equivalent in writeDir.
      // But as KTar and the "tar" program both handle tar files without
      // dir entries, there's really no need for that
      QString tmp ( fileName );
      int i = tmp.lastIndexOf( '/' );
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
    if ( mode() & QIODevice::ReadWrite ) device()->at(d->tarEnd); // Go to end of archive as might have moved with a read

    // provide converted stuff we need lateron
    QByteArray encodedFilename = QFile::encodeName(fileName);
    QByteArray uname = user.toLocal8Bit();
    QByteArray gname = group.toLocal8Bit();

    // If more than 100 chars, we need to use the LongLink trick
    if ( fileName.length() > 99 )
        writeLonglink(buffer,encodedFilename,'L',uname,gname);

    // Write (potentially truncated) name
    strncpy( buffer, encodedFilename, 99 );
    buffer[99] = 0;
    // zero out the rest (except for what gets filled anyways)
    memset(buffer+0x9d, 0, 0x200 - 0x9d);

    QByteArray permstr = QByteArray::number( perm, 8 );
    permstr.rightJustified(6, ' ');
    fillBuffer(buffer, permstr, size, mtime, 0x30, uname, gname);

    // Write header
    return device()->writeBlock( buffer, 0x200 ) == 0x200;
}

bool KTar::writeDir(const QString& name, const QString& user,
                    const QString& group, mode_t perm,
                    time_t atime, time_t mtime, time_t ctime) {
  return KArchive::writeDir(name,user,group,perm,atime,mtime,ctime);
}

bool KTar::writeDir_impl(const QString &name, const QString &user,
                         const QString &group, mode_t perm,
                         time_t /*atime*/, time_t mtime, time_t /*ctime*/) {
    if ( !isOpened() )
    {
        kdWarning(7041) << "KTar::writeDir: You must open the tar file before writing to it\n";
        return false;
    }

    if ( !(mode() & QIODevice::WriteOnly) )
    {
        kdWarning(7041) << "KTar::writeDir: You must open the tar file for writing\n";
        return false;
    }

    // In some tar files we can find dir/./ => call cleanPath
    QString dirName ( QDir::cleanPath( name ) );

    // Need trailing '/'
    if ( !dirName.endsWith( QLatin1Char( '/' ) ) )
        dirName += QLatin1Char( '/' );

    if ( d->dirList.contains( dirName ) )
        return true; // already there

    char buffer[ 0x201 ];
    memset( buffer, 0, 0x200 );
    if ( mode() & QIODevice::ReadWrite ) device()->at(d->tarEnd); // Go to end of archive as might have moved with a read

    // provide converted stuff we need lateron
    QByteArray encodedDirname = QFile::encodeName(dirName);
    QByteArray uname = user.toLocal8Bit();
    QByteArray gname = group.toLocal8Bit();

    // If more than 100 chars, we need to use the LongLink trick
    if ( dirName.length() > 99 )
        writeLonglink(buffer,encodedDirname,'L',uname,gname);

    // Write (potentially truncated) name
    strncpy( buffer, encodedDirname, 99 );
    buffer[99] = 0;
    // zero out the rest (except for what gets filled anyways)
    memset(buffer+0x9d, 0, 0x200 - 0x9d);

    QByteArray permstr = QByteArray::number( perm, 8 );
    permstr.rightJustified(6, ' ');
    fillBuffer( buffer, permstr, 0, mtime, 0x35, uname, gname);

    // Write header
    device()->writeBlock( buffer, 0x200 );
    if ( mode() & QIODevice::ReadWrite )  d->tarEnd = device()->at();

    d->dirList.append( dirName ); // contains trailing slash
    return true; // TODO if wanted, better error control
}

bool KTar::writeSymLink(const QString &name, const QString &target,
                        const QString &user, const QString &group,
                        mode_t perm, time_t atime, time_t mtime, time_t ctime) {
  return KArchive::writeSymLink(name,target,user,group,perm,atime,mtime,ctime);
}

bool KTar::writeSymLink_impl(const QString &name, const QString &target,
                             const QString &user, const QString &group,
                             mode_t perm, time_t /*atime*/, time_t mtime, time_t /*ctime*/) {
    if ( !isOpened() )
    {
        kdWarning(7041) << "KTar::writeSymLink: You must open the tar file before writing to it\n";
        return false;
    }

    if ( !(mode() & QIODevice::WriteOnly) )
    {
        kdWarning(7041) << "KTar::writeSymLink: You must open the tar file for writing\n";
        return false;
    }

    // In some tar files we can find dir/./file => call cleanPath
    QString fileName ( QDir::cleanPath( name ) );

    char buffer[ 0x201 ];
    memset( buffer, 0, 0x200 );
    if ( mode() & QIODevice::ReadWrite ) device()->at(d->tarEnd); // Go to end of archive as might have moved with a read

    // provide converted stuff we need lateron
    QByteArray encodedFilename = QFile::encodeName(fileName);
    QByteArray encodedTarget = QFile::encodeName(target);
    QByteArray uname = user.toLocal8Bit();
    QByteArray gname = group.toLocal8Bit();

    // If more than 100 chars, we need to use the LongLink trick
    if (target.length() > 99)
        writeLonglink(buffer,encodedTarget,'K',uname,gname);
    if ( fileName.length() > 99 )
        writeLonglink(buffer,encodedFilename,'L',uname,gname);

    // Write (potentially truncated) name
    strncpy( buffer, encodedFilename, 99 );
    buffer[99] = 0;
    // Write (potentially truncated) symlink target
    strncpy(buffer+0x9d, encodedTarget, 99);
    buffer[0x9d+99] = 0;
    // zero out the rest
    memset(buffer+0x9d+100, 0, 0x200 - 100 - 0x9d);

    QByteArray permstr = QByteArray::number( perm, 8 );
    permstr.rightJustified(6, ' ');
    fillBuffer(buffer, permstr, 0, mtime, 0x32, uname, gname);

    // Write header
    bool retval = device()->writeBlock( buffer, 0x200 ) == 0x200;
    if ( mode() & QIODevice::ReadWrite )  d->tarEnd = device()->at();
    return retval;
}

void KTar::virtual_hook( int id, void* data ) {
  switch (id) {
    case VIRTUAL_WRITE_SYMLINK: {
      WriteSymlinkParams *params = reinterpret_cast<WriteSymlinkParams *>(data);
      params->retval = writeSymLink_impl(*params->name,*params->target,
                                         *params->user,*params->group,params->perm,
                                         params->atime,params->mtime,params->ctime);
      break;
    }
    case VIRTUAL_WRITE_DIR: {
      WriteDirParams *params = reinterpret_cast<WriteDirParams *>(data);
      params->retval = writeDir_impl(*params->name,*params->user,
                                     *params->group,params->perm,
                                     params->atime,params->mtime,params->ctime);
      break;
    }
    case VIRTUAL_PREPARE_WRITING: {
      PrepareWritingParams *params = reinterpret_cast<PrepareWritingParams *>(data);
      params->retval = prepareWriting_impl(*params->name,*params->user,
                                           *params->group,params->size,params->perm,
                                           params->atime,params->mtime,params->ctime);
      break;
    }
    default:
      KArchive::virtual_hook( id, data );
  }/*end switch*/
}

