/* This file is part of the KDE libraries
   Copyright (C) 2000-2005 David Faure <faure@kde.org>
   Copyright (C) 2003 Leo Savernik <l.savernik@aon.at>

   Moved from ktar.cpp by Roberto Teixeira <maragato@kde.org>

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


#include "karchive.h"
#include "klimitediodevice.h"

#include <kdebug.h>
#include <ksavefile.h>
#include <kfilterdev.h>
#include <kfilterbase.h>
#include <kde_file.h>

#include <q3ptrlist.h>
#include <QStack>
#include <qmap.h>
#include <qdir.h>
#include <qfile.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

class KArchive::KArchivePrivate
{
public:
    KArchivePrivate()
        : rootDir( 0 ),
          saveFile( 0 ),
          fileName(),
          mode( QIODevice::NotOpen ),
          deviceOwned( false )
    {}
    KArchiveDirectory* rootDir;
    KSaveFile* saveFile;
    QString fileName;
    QIODevice::OpenMode mode;
    bool deviceOwned; // if true, we (KArchive) own m_dev and must delete it
};

// ### KDE4 TODO port me and add unit test
class PosSortedPtrList : public Q3PtrList<KArchiveFile> {
protected:
    int compareItems( Q3PtrCollection::Item i1,
                      Q3PtrCollection::Item i2 )
    {
        int pos1 = static_cast<KArchiveFile*>( i1 )->position();
        int pos2 = static_cast<KArchiveFile*>( i2 )->position();
        return ( pos1 - pos2 );
    }
};


////////////////////////////////////////////////////////////////////////
/////////////////////////// KArchive ///////////////////////////////////
////////////////////////////////////////////////////////////////////////

KArchive::KArchive( const QString& fileName )
	: d(new KArchivePrivate)
{
    Q_ASSERT( !fileName.isEmpty() );
    d->fileName = fileName;
    // This constructor leaves the device set to 0.
    // This is for the use of KSaveFile, see open().
    m_dev = 0;
}

KArchive::KArchive( QIODevice * dev )
	: d(new KArchivePrivate)
{
    m_dev = dev;
}

KArchive::~KArchive()
{
    if ( isOpen() )
        close(); // WARNING: won't call the virtual method close in the derived class!!!

    delete d->saveFile;
    delete d->rootDir;
    delete d;
}

bool KArchive::open( QIODevice::OpenMode mode )
{
    Q_ASSERT( mode != QIODevice::NotOpen );

    if ( isOpen() )
        close();

    if ( !d->fileName.isEmpty() )
    {
        Q_ASSERT( !m_dev );
        if ( !createDevice( mode ) )
            return false;
    }

    Q_ASSERT( m_dev );

    if ( !m_dev->isOpen() && !m_dev->open( mode ) )
        return false;

    d->mode = mode;

    Q_ASSERT( !d->rootDir );
    d->rootDir = 0;

    return openArchive( mode );
}

bool KArchive::createDevice( QIODevice::OpenMode mode )
{
    switch( mode ) {
    case QIODevice::WriteOnly:
        if ( !d->fileName.isEmpty() ) {
            // The use of KSaveFile can't be done in the ctor (no mode known yet)
            //kDebug() << "Writing to a file using KSaveFile" << endl;
            d->saveFile = new KSaveFile( d->fileName );
            if ( d->saveFile->status() != 0 ) {
                kWarning() << "KSaveFile creation for " << d->fileName << " failed, " << strerror( d->saveFile->status() ) << endl;
                delete d->saveFile;
                d->saveFile = 0;
                return false;
            }
            Q_ASSERT( d->saveFile->file() );
            m_dev = d->saveFile->file();
        }
        break;
    case QIODevice::ReadOnly:
    case QIODevice::ReadWrite:
        // ReadWrite mode still uses QFile for now; we'd need to copy to the tempfile, in fact.
        if ( !d->fileName.isEmpty() ) {
            m_dev = new QFile( d->fileName );
            d->deviceOwned = true;
        }
        break; // continued below
    default:
        kWarning() << "Unsupported mode " << d->mode << endl;
        return false;
    }
    return true;
}

bool KArchive::close()
{
    if ( !isOpen() )
        return false; // already closed (return false or true? arguable...)

    // moved by holger to allow kzip to write the zip central dir
    // to the file in closeArchive()
    // DF: added m_dev so that we skip closeArchive if saving aborted.
    bool closeSucceeded = true;
    if ( m_dev ) {
        closeSucceeded = closeArchive();
        if ( d->mode == QIODevice::WriteOnly && !closeSucceeded )
            abortWriting();
    }

    if ( m_dev )
        m_dev->close();

    if ( d->deviceOwned ) {
        delete m_dev; // we created it ourselves in open()
    }
    if ( d->saveFile ) {
        d->saveFile->close();
        delete d->saveFile;
        d->saveFile = 0;
    }

    delete d->rootDir;
    d->rootDir = 0;
    d->mode = QIODevice::NotOpen;
    m_dev = 0;
    return closeSucceeded;
}

const KArchiveDirectory* KArchive::directory() const
{
    // rootDir isn't const so that parsing-on-demand is possible
    return const_cast<KArchive *>(this)->rootDir();
}


bool KArchive::addLocalFile( const QString& fileName, const QString& destName )
{
    QFileInfo fileInfo( fileName );
    if ( !fileInfo.isFile() && !fileInfo.isSymLink() )
    {
        kWarning() << "KArchive::addLocalFile " << fileName << " doesn't exist or is not a regular file." << endl;
        return false;
    }

    KDE_struct_stat fi;
    if (KDE_lstat(QFile::encodeName(fileName),&fi) == -1) {
        kWarning() << "KArchive::addLocalFile stating " << fileName
        	<< " failed: " << strerror(errno) << endl;
        return false;
    }

    if (fileInfo.isSymLink()) {
        return writeSymLink(destName, fileInfo.readLink(), fileInfo.owner(),
                            fileInfo.group(), fi.st_mode, fi.st_atime, fi.st_mtime,
                            fi.st_ctime);
    }/*end if*/

    qint64 size = fileInfo.size();

    // the file must be opened before prepareWriting is called, otherwise
    // if the opening fails, no content will follow the already written
    // header and the tar file is effectively f*cked up
    QFile file( fileName );
    if ( !file.open( QIODevice::ReadOnly ) )
    {
        kWarning() << "KArchive::addLocalFile couldn't open file " << fileName << endl;
        return false;
    }

    if ( !prepareWriting( destName, fileInfo.owner(), fileInfo.group(), size,
    		fi.st_mode, fi.st_atime, fi.st_mtime, fi.st_ctime ) )
    {
        kWarning() << "KArchive::addLocalFile prepareWriting " << destName << " failed" << endl;
        return false;
    }

    // Read and write data in chunks to minimize memory usage
    QByteArray array;
    array.resize(8*1024);
    qint64 n;
    qint64 total = 0;
    while ( ( n = file.read( array.data(), array.size() ) ) > 0 )
    {
        if ( !writeData( array.data(), n ) )
        {
            kWarning() << "KArchive::addLocalFile writeData failed" << endl;
            return false;
        }
        total += n;
    }
    Q_ASSERT( total == size );

    if ( !finishWriting( size ) )
    {
        kWarning() << "KArchive::addLocalFile finishWriting failed" << endl;
        return false;
    }
    return true;
}

bool KArchive::addLocalDirectory( const QString& path, const QString& destName )
{
    QString dot = ".";
    QString dotdot = "..";
    QDir dir( path );
    if ( !dir.exists() )
        return false;
    QStringList files = dir.entryList();
    for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it )
    {
        if ( *it != dot && *it != dotdot )
        {
            QString fileName = path + "/" + *it;
//            kDebug() << "storing " << fileName << endl;
            QString dest = destName.isEmpty() ? *it : (destName + "/" + *it);
            QFileInfo fileInfo( fileName );

            if ( fileInfo.isFile() || fileInfo.isSymLink() )
                addLocalFile( fileName, dest );
            else if ( fileInfo.isDir() )
                addLocalDirectory( fileName, dest );
            // We omit sockets
        }
    }
    return true;
}

bool KArchive::writeFile( const QString& name, const QString& user,
                          const QString& group, const char* data, qint64 size,
                          mode_t perm, time_t atime, time_t mtime, time_t ctime )
{
    if ( !prepareWriting( name, user, group, size, perm, atime, mtime, ctime ) )
    {
        kWarning() << "KArchive::writeFile prepareWriting failed" << endl;
        return false;
    }

    // Write data
    // Note: if data is 0L, don't call writeBlock, it would terminate the KFilterDev
    if ( data && size && !writeData( data, size ) )
    {
        kWarning() << "KArchive::writeFile writeData failed" << endl;
        return false;
    }

    if ( !finishWriting( size ) )
    {
        kWarning() << "KArchive::writeFile finishWriting failed" << endl;
        return false;
    }
    return true;
}

bool KArchive::writeData( const char* data, qint64 size )
{
    bool ok = device()->write( data, size ) == size;
    if ( !ok )
        abortWriting();
    return ok;
}

// The writeDir -> doWriteDir pattern allows to avoid propagating the default
// values into all virtual methods of subclasses, and it allows more extensibility:
// if a new argument is needed, we can add a writeDir overload which stores the
// additional argument in the d pointer, and doWriteDir reimplementations can fetch
// it from there.

bool KArchive::writeDir( const QString& name, const QString& user, const QString& group,
                         mode_t perm, time_t atime,
                         time_t mtime, time_t ctime )
{
    return doWriteDir( name, user, group, perm, atime, mtime, ctime );
}

bool KArchive::writeSymLink(const QString &name, const QString &target,
                            const QString &user, const QString &group,
                            mode_t perm, time_t atime,
                            time_t mtime, time_t ctime )
{
    return doWriteSymLink( name, target, user, group, perm, atime, mtime, ctime );
}


bool KArchive::prepareWriting( const QString& name, const QString& user,
                               const QString& group, qint64 size,
                               mode_t perm, time_t atime,
                               time_t mtime, time_t ctime )
{
    bool ok = doPrepareWriting( name, user, group, size, perm, atime, mtime, ctime );
    if ( !ok )
        abortWriting();
    return ok;
}

bool KArchive::finishWriting( qint64 size )
{
    return doFinishWriting( size );
}

KArchiveDirectory * KArchive::rootDir()
{
    if ( !d->rootDir )
    {
        //kDebug() << "Making root dir " << endl;
        struct passwd* pw =  getpwuid( getuid() );
        struct group* grp = getgrgid( getgid() );
        QString username = pw ? QFile::decodeName(pw->pw_name) : QString::number( getuid() );
        QString groupname = grp ? QFile::decodeName(grp->gr_name) : QString::number( getgid() );

        d->rootDir = new KArchiveDirectory( this, QLatin1String("/"), (int)(0777 + S_IFDIR), 0, username, groupname, QString() );
    }
    return d->rootDir;
}

KArchiveDirectory * KArchive::findOrCreate( const QString & path )
{
    //kDebug() << "KArchive::findOrCreate " << path << endl;
    if ( path.isEmpty() || path == "/" || path == "." ) // root dir => found
    {
        //kDebug() << "KArchive::findOrCreate returning rootdir" << endl;
        return rootDir();
    }
    // Important note : for tar files containing absolute paths
    // (i.e. beginning with "/"), this means the leading "/" will
    // be removed (no KDirectory for it), which is exactly the way
    // the "tar" program works (though it displays a warning about it)
    // See also KArchiveDirectory::entry().

    // Already created ? => found
    const KArchiveEntry* ent = rootDir()->entry( path );
    if ( ent )
    {
        if ( ent->isDirectory() )
            //kDebug() << "KArchive::findOrCreate found it" << endl;
            return (KArchiveDirectory *) ent;
        else
            kWarning() << "Found " << path << " but it's not a directory" << endl;
    }

    // Otherwise go up and try again
    int pos = path.lastIndexOf( '/' );
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

    //kDebug() << "KTar : found parent " << parent->name() << " adding " << dirname << " to ensure " << path << endl;
    // Found -> add the missing piece
    KArchiveDirectory * e = new KArchiveDirectory( this, dirname, d->rootDir->permissions(),
                                                   d->rootDir->date(), d->rootDir->user(),
                                                   d->rootDir->group(), QString() );
    parent->addEntry( e );
    return e; // now a directory to <path> exists
}

void KArchive::setDevice( QIODevice * dev )
{
    if ( d->deviceOwned )
        delete m_dev;
    m_dev = dev;
    d->deviceOwned = false;
}

void KArchive::setRootDir( KArchiveDirectory *rootDir )
{
    Q_ASSERT( !d->rootDir ); // Call setRootDir only once during parsing please ;)
    d->rootDir = rootDir;
}

QIODevice::OpenMode KArchive::mode() const
{
    return d->mode;
}

bool KArchive::isOpen() const
{
    return d->mode != QIODevice::NotOpen;
}

QString KArchive::fileName() const
{
    return d->fileName;
}

void KArchive::abortWriting()
{
    if ( d->saveFile ) {
        d->saveFile->abort();
        delete d->saveFile;
        d->saveFile = 0;
        m_dev = 0;
    }
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
                            qint64 pos, qint64 size )
  : KArchiveEntry( t, name, access, date, user, group, symlink )
{
  m_pos = pos;
  m_size = size;
}

KArchiveFile::~KArchiveFile()
{
}

qint64 KArchiveFile::position() const
{
  return m_pos;
}

qint64 KArchiveFile::size() const
{
  return m_size;
}

QByteArray KArchiveFile::data() const
{
  archive()->device()->seek( m_pos );

  // Read content
  QByteArray arr;
  if ( m_size )
  {
    assert( arr.data() );
    arr = archive()->device()->read( m_size );
    Q_ASSERT( arr.size() == m_size );
  }
  return arr;
}

QIODevice * KArchiveFile::device() const
{
  return new KLimitedIODevice( archive()->device(), m_pos, m_size );
}

void KArchiveFile::copyTo(const QString& dest) const
{
  QFile f( dest + "/"  + name() );
  if ( f.open( QIODevice::ReadWrite | QIODevice::Truncate ) )
  {
      f.write( data() );
      f.close();
  }
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
}

KArchiveDirectory::~KArchiveDirectory()
{
  qDeleteAll(m_entries);
}

QStringList KArchiveDirectory::entries() const
{
  return m_entries.keys();
}

const KArchiveEntry* KArchiveDirectory::entry( const QString& _name ) const
{
  QString name = _name;
  int pos = name.indexOf( '/' );
  if ( pos == 0 ) // ouch absolute path (see also KArchive::findOrCreate)
  {
    if (name.length()>1)
    {
      name = name.mid( 1 ); // remove leading slash
      pos = name.indexOf( '/' ); // look again
    }
    else // "/"
      return this;
  }
  // trailing slash ? -> remove
  if ( pos != -1 && pos == name.length()-1 )
  {
    name = name.left( pos );
    pos = name.indexOf( '/' ); // look again
  }
  if ( pos != -1 )
  {
    QString left = name.left( pos );
    QString right = name.mid( pos + 1 );

    //kDebug() << "KArchiveDirectory::entry left=" << left << " right=" << right << endl;

    const KArchiveEntry* e = m_entries.value( left );
    if ( !e || !e->isDirectory() )
      return 0;
    return static_cast<const KArchiveDirectory*>(e)->entry( right );
  }

  return m_entries.value( name );
}

void KArchiveDirectory::addEntry( KArchiveEntry* entry )
{
  Q_ASSERT( !entry->name().isEmpty() );
  if( m_entries.value( entry->name() ) ) {
      kWarning() << "KArchiveDirectory::addEntry: directory " << name()
                  << " has entry " << entry->name() << " already" << endl;
  }
  m_entries.insert( entry->name(), entry );
}

void KArchiveDirectory::copyTo(const QString& dest, bool recursiveCopy ) const
{
  QDir root;

  PosSortedPtrList fileList;
  QMap<qint64, QString> fileToDir;

  QStringList::Iterator it;

  // placeholders for iterated items
  QStringList dirEntries;

  QStack<const KArchiveDirectory *> dirStack;
  QStack<QString> dirNameStack;

  dirStack.push( this );     // init stack at current directory
  dirNameStack.push( dest ); // ... with given path
  do {
    const KArchiveDirectory* curDir = dirStack.pop();
    const QString curDirName = dirNameStack.pop();
    root.mkdir(curDirName);

    dirEntries = curDir->entries();
    for ( it = dirEntries.begin(); it != dirEntries.end(); ++it ) {
      const KArchiveEntry* curEntry = curDir->entry(*it);
      if ( curEntry->isFile() ) {
        const KArchiveFile* curFile = dynamic_cast<const KArchiveFile*>( curEntry );
	if (curFile) {
          fileList.append( curFile );
          fileToDir.insert( curFile->position(), curDirName );
        }
      }

      if ( curEntry->isDirectory() && recursiveCopy ) {
        const KArchiveDirectory *ad = dynamic_cast<const KArchiveDirectory*>( curEntry );
        if (ad) {
          dirStack.push( ad );
          dirNameStack.push( curDirName + "/" + curEntry->name() );
        }
      }
    }
  } while (!dirStack.isEmpty());

  fileList.sort();  // sort on m_pos, so we have a linear access

  KArchiveFile* f;
  for ( f = fileList.first(); f; f = fileList.next() ) {
    qint64 pos = f->position();
    f->copyTo( fileToDir[pos] );
  }
}

void KArchive::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data )*/; }

void KArchiveEntry::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KArchiveFile::virtual_hook( int id, void* data )
{ KArchiveEntry::virtual_hook( id, data ); }

void KArchiveDirectory::virtual_hook( int id, void* data )
{ KArchiveEntry::virtual_hook( id, data ); }
