/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
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

template class QDict<KArchiveEntry>;

class KArchive::KArchivePrivate
{
public:
    KArchiveDirectory* rootDir;
};

////////////////////////////////////////////////////////////////////////
/////////////////////////// KArchive ///////////////////////////////////
////////////////////////////////////////////////////////////////////////

KArchive::KArchive( QIODevice * dev )
{
    d = new KArchivePrivate;
    d->rootDir = 0;
    m_dev = dev;
    m_open = false;
}

KArchive::~KArchive()
{
    if ( m_open )
        close();
    delete d->rootDir;
    delete d;
}

bool KArchive::open( int mode )
{
    if(0 == m_dev)
        return false; // Fail w/o segfaulting if the device is no good

    if ( !m_dev->open( mode ) )
        return false;

    if ( m_open )
        close();

    m_mode = mode;
    m_open = true;

    Q_ASSERT( d->rootDir == 0L );
    d->rootDir = 0L;

    return openArchive( mode );
}

void KArchive::close()
{
    if ( !m_open )
        return;

    m_dev->close();
    //m_dirList.clear();

    delete d->rootDir;
    d->rootDir = 0;
    m_open = false;
    closeArchive();
}

const KArchiveDirectory* KArchive::directory() const
{
    // rootDir isn't const so that parsing-on-demand is possible
    return const_cast<KArchive *>(this)->rootDir();
}

bool KArchive::writeFile( const QString& name, const QString& user, const QString& group, uint size, const char* data )
{
    if ( !prepareWriting( name, user, group, size ) )
        return false;

    // Write data
    if ( device()->writeBlock( data, size ) != (int)size )
        return false;

    return doneWriting( size );
}

KArchiveDirectory * KArchive::rootDir()
{
    if ( !d->rootDir )
    {
        //kdDebug() << "Making root dir " << endl;
        struct passwd* pw =  getpwuid( getuid() );
        struct group* grp = getgrgid( getgid() );
        QString username = pw ? QFile::decodeName(pw->pw_name) : QString::number( getuid() );
        QString groupname = grp ? QFile::decodeName(grp->gr_name) : QString::number( getgid() );

        d->rootDir = new KArchiveDirectory( this, QString::fromLatin1("/"), (int)(0777 + S_IFDIR), 0, username, groupname, QString::null );
    }
    return d->rootDir;
}

KArchiveDirectory * KArchive::findOrCreate( const QString & path )
{
    //kdDebug() << "KArchive::findOrCreate " << path << endl;
    if ( path == "" || path == "/" || path == "." ) // root dir => found
    {
        //kdDebug() << "KArchive::findOrCreate returning rootdir" << endl;
        return rootDir();
    }
    // Important note : for tar files containing absolute paths
    // (i.e. beginning with "/"), this means the leading "/" will
    // be removed (no KDirectory for it), which is exactly the way
    // the "tar" program works (though it displays a warning about it)
    // See also KArchiveDirectory::entry().

    // Already created ? => found
    KArchiveEntry* ent = rootDir()->entry( path );
    if ( ent && ent->isDirectory() )
    {
        //kdDebug() << "KArchive::findOrCreate found it" << endl;
        return (KArchiveDirectory *) ent;
    }

    // Otherwise go up and try again
    int pos = path.findRev( '/' );
    KArchiveDirectory * parent;
    QString dirname;
    if ( pos == -1 ) // no more slash => create in root dir
    {
        parent =  rootDir();
        dirname = path;
    }
    else
    {
        QString left = path.left( pos );
        dirname = path.mid( pos + 1 );
        parent = findOrCreate( left ); // recursive call... until we find an existing dir.
    }

    //kdDebug() << "KTar : found parent " << parent->name() << " adding " << dirname << " to ensure " << path << endl;
    // Found -> add the missing piece
    KArchiveDirectory * e = new KArchiveDirectory( this, dirname, d->rootDir->permissions(),
                                                   d->rootDir->date(), d->rootDir->user(),
                                                   d->rootDir->group(), QString::null );
    parent->addEntry( e );
    return e; // now a directory to <path> exists
}

void KArchive::setDevice( QIODevice * dev )
{
    m_dev = dev;
}

void KArchive::setRootDir( KArchiveDirectory *rootDir )
{
    Q_ASSERT( !d->rootDir ); // Call setRootDir only once during parsing please ;)
    d->rootDir = rootDir;
}

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
        KURL url;
        url.setPath( filename );
        mimetype = KMimeType::findByURL( url )->name();
        //kdDebug() << "KTar::KTar mimetype=" << mimetype << endl;

        // Don't move to prepareDevice - the other constructor theoretically allows ANY filter
        if (mimetype == "application/x-tgz" || mimetype == "application/x-targz") // the latter is deprecated but might still be around
            // that's a gzipped tar file, so ask for gzip filter
            mimetype = "application/x-gzip";
        else if ( mimetype == "application/x-tbz" ) // that's a bzipped2 tar file, so ask for bz2 filter
            mimetype = "application/x-bzip2";
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
    if ( !isOpened() || mode() != IO_WriteOnly )
    {
        qWarning( "KArchive::setOrigFileName: File must be opened for writing first.\n");
        return;
    }
    static_cast<KFilterDev *>(device())->setOrigFileName( fileName );
}


bool KTar::openArchive( int mode )
{
    if ( mode == IO_WriteOnly )
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
            if ( typeflag == '1' )
                isdir = true;
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
                buffer[ 0x87 ] = 0;
                char *dummy;
                const char* p = buffer + 0x7c;
                while( *p == ' ' ) ++p;
                int size = (int)strtol( p, &dummy, 8 );

                // Let's hack around hard links. Our classes don't support that, so make them symlinks
                if ( typeflag == '1' )
                {
                    size = nm.length(); // in any case, we don't want to skip the real size, hence this resetting of size
                    kdDebug() << "HARD LINK, setting size to " << size << endl;
                }

                int rest = size % 0x200;
                //kdDebug() << "KArchive::open file " << nm << " size=" << size << endl;

                e = new KArchiveFile( this, nm, access, time, user, group, symlink,
                                      dev->at(), size );

                // Skip contents + align bytes
                int skip = size + (rest ? 0x200 - rest : 0);
                //kdDebug() << "KArchive::open skipping " << skip << endl;
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
        qWarning( "KArchive::writeDir: You must open the tar file before writing to it\n");
        return false;
    }

    if ( mode() != IO_WriteOnly )
    {
        qWarning( "KArchive::writeDir: You must open the tar file for writing\n");
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
        qWarning( "KArchive::writeFile: You must open the tar file before writing to it\n");
        return false;
    }

    if ( mode() != IO_WriteOnly )
    {
        qWarning( "KArchive::writeFile: You must open the tar file for writing\n");
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
        return nwritten == (Q_LONG)size;
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

////////////////////////////////////////////////////////////////////////
/////////////////////// KArchiveEntry //////////////////////////////////
////////////////////////////////////////////////////////////////////////
KArchiveEntry::KArchiveEntry( KArchive* t, const QString& name, int access, int date,
                      const QString& user, const QString& group, const
                      QString& symlink)
{
  m_name = name;
  m_access = access;
  m_date = date;
  m_user = user;
  m_group = group;
  m_symlink = symlink;
  m_archive = t;

}

QDateTime KArchiveEntry::datetime() const
{
  QDateTime d;
  d.setTime_t( m_date );
  return d;
}

////////////////////////////////////////////////////////////////////////
/////////////////////// KArchiveFile ///////////////////////////////////
////////////////////////////////////////////////////////////////////////

KArchiveFile::KArchiveFile( KArchive* t, const QString& name, int access, int date,
                    const QString& user, const QString& group,
                    const QString & symlink,
                    int pos, int size )
  : KArchiveEntry( t, name, access, date, user, group, symlink )
{
  m_pos = pos;
  m_size = size;
}

int KArchiveFile::position() const
{
  return m_pos;
}

int KArchiveFile::size() const
{
  return m_size;
}

QByteArray KArchiveFile::data() const
{
  archive()->device()->at( m_pos );

  // Read content
  QByteArray arr( m_size );
  if ( m_size )
  {
    assert( arr.data() );
    int n = archive()->device()->readBlock( arr.data(), m_size );
    if ( n != m_size )
      arr.resize( n );
  }
  return arr;
}

/**
 * A readonly device that reads from an underlying device
 * from a given point to another (e.g. to give access to a single
 * file inside an archive)
 * @author David Faure <david@mandrakesoft.com>
 */
class KLimitedIODevice : public QIODevice
{
public:
    /**
     * @param dev the underlying device, opened or not
     * This device itself auto-opens (in readonly mode), no need to open it.
     */
    KLimitedIODevice( QIODevice *dev, int start, int length )
        : m_dev( dev ), m_start( start ), m_length( length )
    {
        //kdDebug(7005) << "KLimitedIODevice::KLimitedIODevice start=" << start << " length=" << length << endl;
        setType( IO_Direct ); // we support sequential too, but then atEnd() tries getch/ungetch !
        open( IO_ReadOnly );
    }
    virtual ~KLimitedIODevice() {}

    virtual bool open( int m ) {
        //kdDebug(7005) << "KLimitedIODevice::open m=" << m << endl;
        if ( m & IO_ReadOnly ) {
            /*bool ok = false;
            if ( m_dev->isOpen() )
                ok = ( m_dev->mode() == IO_ReadOnly );
            else
                ok = m_dev->open( m );
            if ( ok )*/
                m_dev->at( m_start ); // No concurrent access for the moment !
        }
        else
            kdWarning(7005) << "KLimitedIODevice::open only supports IO_ReadOnly!" << endl;
        setState( IO_Open );
        setMode( m );
        return true;
    }
    virtual void close() {}
    virtual void flush() {}

    virtual Offset size() const { return m_length; }
    // at(), at(offset) and atEnd() are taken care of by QIODevice

    virtual Q_LONG readBlock ( char * data, Q_ULONG maxlen )
    {
        //kdDebug(7005) << "KLimitedIODevice::readBlock maxlen=" << maxlen << endl;
        Q_ULONG max = QMIN( at() + maxlen, m_start + m_length );
        return m_dev->readBlock( data, max );
    }
    virtual Q_LONG writeBlock ( const char *, Q_ULONG ) { return -1; }

    virtual int getch() {
        char c[2];
        if ( readBlock(c, 1) == -1)
            return -1;
        else
            return c[0];
    }
    virtual int putch( int ) { return -1; } // unsupported
    virtual int ungetch( int ) { return -1; } // unsupported
private:
    QIODevice* m_dev;
    Q_ULONG m_start;
    Q_ULONG m_length;
};


QIODevice *KArchiveFile::device() const
{
    return new KLimitedIODevice( archive()->device(), m_pos, m_size );
}

////////////////////////////////////////////////////////////////////////
//////////////////////// KArchiveDirectory /////////////////////////////////
////////////////////////////////////////////////////////////////////////


KArchiveDirectory::KArchiveDirectory( KArchive* t, const QString& name, int access,
                              int date,
                              const QString& user, const QString& group,
                              const QString &symlink)
  : KArchiveEntry( t, name, access, date, user, group, symlink )
{
  m_entries.setAutoDelete( true );
}

QStringList KArchiveDirectory::entries() const
{
  QStringList l;

  QDictIterator<KArchiveEntry> it( m_entries );
  for( ; it.current(); ++it )
    l.append( it.currentKey() );

  return l;
}

KArchiveEntry* KArchiveDirectory::entry( QString name )
  // not "const QString & name" since we want a local copy
  // (to remove leading slash if any)
{
  int pos = name.find( '/' );
  if ( pos == 0 ) // ouch absolute path (see also KTar::findOrCreate)
  {
    if (name.length()>1)
    {
      name = name.mid( 1 ); // remove leading slash
      pos = name.find( '/' ); // look again
    }
    else // "/"
      return this;
  }
  // trailing slash ? -> remove
  if ( pos != -1 && pos == (int)name.length()-1 )
  {
    name = name.left( pos );
    pos = name.find( '/' ); // look again
  }
  if ( pos != -1 )
  {
    QString left = name.left( pos );
    QString right = name.mid( pos + 1 );

    //kdDebug() << "KArchiveDirectory::entry left=" << left << " right=" << right << endl;

    KArchiveEntry* e = m_entries[ left ];
    if ( !e || !e->isDirectory() )
      return 0;
    return ((KArchiveDirectory*)e)->entry( right );
  }

  return m_entries[ name ];
}

const KArchiveEntry* KArchiveDirectory::entry( QString name ) const
{
  return ((KArchiveDirectory*)this)->entry( name );
}

void KArchiveDirectory::addEntry( KArchiveEntry* entry )
{
  m_entries.insert( entry->name(), entry );
}
