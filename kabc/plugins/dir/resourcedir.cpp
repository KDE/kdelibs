/*
    This file is part of libkabc.
    Copyright (c) 2002 - 2003 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <qregexp.h>
#include <qtimer.h>
#include <qwidget.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>

#include "addressbook.h"

#include "formatfactory.h"

#include "resourcedirconfig.h"
#include "stdaddressbook.h"

#include "resourcedir.h"

using namespace KABC;

extern "C"
{
  void *init_kabc_dir()
  {
    return new KRES::PluginFactory<ResourceDir,ResourceDirConfig>();
  }
}


ResourceDir::ResourceDir( const KConfig *config )
  : Resource( config ), mAsynchronous( false )
{
  if ( config ) {
    init( config->readPathEntry( "FilePath" ), config->readEntry( "FileFormat" ) );
  } else {
    init( StdAddressBook::directoryName(), "vcard" );
  }
}

ResourceDir::ResourceDir( const QString &path, const QString &format )
  : Resource( 0 ), mAsynchronous( false )
{
  init( path, format );
}

void ResourceDir::init( const QString &path, const QString &format )
{
  mFormatName = format;

  FormatFactory *factory = FormatFactory::self();
  mFormat = factory->format( mFormatName );

  if ( !mFormat ) {
    mFormatName = "vcard";
    mFormat = factory->format( mFormatName );
  }

  connect( &mDirWatch, SIGNAL( dirty(const QString&) ), SLOT( pathChanged() ) );
  connect( &mDirWatch, SIGNAL( created(const QString&) ), SLOT( pathChanged() ) );
  connect( &mDirWatch, SIGNAL( deleted(const QString&) ), SLOT( pathChanged() ) );

  setPath( path );
}

ResourceDir::~ResourceDir()
{
  delete mFormat;
  mFormat = 0;
}

void ResourceDir::writeConfig( KConfig *config )
{
  Resource::writeConfig( config );

  config->writePathEntry( "FilePath", mPath );
  config->writeEntry( "FileFormat", mFormatName );
}

Ticket *ResourceDir::requestSaveTicket()
{
  kdDebug(5700) << "ResourceDir::requestSaveTicket()" << endl;

  if ( !lock( mPath ) ) {
    kdDebug(5700) << "ResourceDir::requestSaveTicket(): Unable to lock path '"
                  << mPath << "'" << endl;
    return 0;
  }
  return createTicket( this );
}

void ResourceDir::releaseSaveTicket( Ticket *ticket )
{
  delete ticket;
  unlock( mPath );
}

bool ResourceDir::doOpen()
{
  QDir dir( mPath );
  if ( !dir.exists() ) { // no directory available
    return dir.mkdir( dir.path() );
  } else {
    QString testName = dir.entryList( QDir::Files )[0];
    if ( testName.isNull() || testName.isEmpty() ) // no file in directory
      return true;

    QFile file( mPath + "/" + testName );
    if ( file.open( IO_ReadOnly ) )
      return true;

    if ( file.size() == 0 )
      return true;

    bool ok = mFormat->checkFormat( &file );
    file.close();
    return ok;
  }
}

void ResourceDir::doClose()
{
}

bool ResourceDir::load()
{
  kdDebug(5700) << "ResourceDir::load(): '" << mPath << "'" << endl;

  mAsynchronous = false;

  QDir dir( mPath );
  QStringList files = dir.entryList( QDir::Files );

  QStringList::Iterator it;
  bool ok = true;
  for ( it = files.begin(); it != files.end(); ++it ) {
    QFile file( mPath + "/" + (*it) );

    if ( !file.open( IO_ReadOnly ) ) {
      addressBook()->error( i18n( "Unable to open file '%1' for reading" ).arg( file.name() ) );
      ok = false;
      continue;
    }

    if ( !mFormat->loadAll( addressBook(), this, &file ) )
      ok = false;

    file.close();
  }

  return ok;
}

bool ResourceDir::asyncLoad()
{
  mAsynchronous = true;

  bool ok = load();
  if ( !ok )
    emit loadingError( this, i18n( "Loading resource '%1' failed!" )
                       .arg( resourceName() ) );
  else
    emit loadingFinished( this );

  return ok;
}

bool ResourceDir::save( Ticket* )
{
  kdDebug(5700) << "ResourceDir::save(): '" << mPath << "'" << endl;

  Addressee::Map::Iterator it;
  bool ok = true;

  for ( it = mAddrMap.begin(); it != mAddrMap.end(); ++it ) {
    if ( !it.data().changed() )
      continue;

    QFile file( mPath + "/" + (*it).uid() );
    if ( !file.open( IO_WriteOnly ) ) {
      addressBook()->error( i18n( "Unable to open file '%1' for writing" ).arg( file.name() ) );
      continue;
    }

    mFormat->save( *it, &file );

    // mark as unchanged
    (*it).setChanged( false );

    file.close();
  }

  return ok;
}

bool ResourceDir::asyncSave( Ticket *ticket )
{
  bool ok = save( ticket );
  if ( !ok )
    emit savingError( this, i18n( "Saving resource '%1' failed!" )
                      .arg( resourceName() ) );
  else
    emit savingFinished( this );

  return ok;
}

bool ResourceDir::lock( const QString &path )
{
  kdDebug(5700) << "ResourceDir::lock()" << endl;

  QString p = path;
  p.replace( "/", "_" );

  QString lockName = locateLocal( "data", "kabc/lock/" + p + ".lock" );
  kdDebug(5700) << "-- lock name: " << lockName << endl;

  if ( QFile::exists( lockName ) ) {  // check if it is a stale lock file
    QFile file( lockName );
    if ( !file.open( IO_ReadOnly ) )
      return false;

    QDataStream t( &file );

    QString app; int pid;
    t >> pid >> app;

    int retval = ::kill( pid, 0 );
    if ( retval == -1 && errno == ESRCH ) { // process doesn't exists anymore
      QFile::remove( lockName );
      kdError() << "dedect stale lock file from process '" << app << "'" << endl;
      file.close();
    } else {
      addressBook()->error( i18n( "The resource '%1' is locked by application '%2'." )
                            .arg( resourceName() ).arg( app )  );
      return false;
    }
  }

  QString lockUniqueName;
  lockUniqueName = p + kapp->randomString( 8 );
  mLockUniqueName = locateLocal( "data", "kabc/lock/" + lockUniqueName );
  kdDebug(5700) << "-- lock unique name: " << mLockUniqueName << endl;

  // Create unique file
  QFile file( mLockUniqueName );
  file.open( IO_WriteOnly );
  QDataStream t( &file );
  t << ::getpid() << QString( KGlobal::instance()->instanceName() );
  file.close();

  // Create lock file
  int result = ::link( QFile::encodeName( mLockUniqueName ),
                       QFile::encodeName( lockName ) );

  if ( result == 0 ) {
    addressBook()->emitAddressBookLocked();
    return true;
  }

  // TODO: check stat

  return false;
}

void ResourceDir::unlock( const QString &path )
{
  QString p = path;
  p.replace( "/" , "_" );

  QString lockName = locate( "data", "kabc/lock/" + p + ".lock" );
  ::unlink( QFile::encodeName( lockName ) );
  QFile::remove( mLockUniqueName );
  addressBook()->emitAddressBookUnlocked();
}

void ResourceDir::setPath( const QString &path )
{
  mDirWatch.stopScan();
  if ( mDirWatch.contains( mPath ) )
    mDirWatch.removeDir( mPath );

  mPath = path;
  mDirWatch.addDir( mPath, true );
  mDirWatch.startScan();
}

QString ResourceDir::path() const
{
  return mPath;
}

void ResourceDir::setFormat( const QString &format )
{
  mFormatName = format;

  if ( mFormat )
    delete mFormat;

  FormatFactory *factory = FormatFactory::self();
  mFormat = factory->format( mFormatName );
}

QString ResourceDir::format() const
{
  return mFormatName;
}

void ResourceDir::pathChanged()
{
  if ( !addressBook() )
    return;

  clear();
  if ( mAsynchronous )
    asyncLoad();
  else {
    load();
    addressBook()->emitAddressBookChanged();
  }
}

void ResourceDir::removeAddressee( const Addressee& addr )
{
  QFile::remove( mPath + "/" + addr.uid() );
  mAddrMap.erase( addr.uid() );
}

void ResourceDir::cleanUp()
{
  unlock( mPath );
}

#include "resourcedir.moc"
