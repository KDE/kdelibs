/*
    This file is part of libkabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <qfile.h>
#include <qregexp.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kio/scheduler.h>
#include <klocale.h>
#include <ksavefile.h>
#include <kstandarddirs.h>
#include <ktempfile.h>

#include "formatfactory.h"
#include "resourcefileconfig.h"
#include "stdaddressbook.h"

#include "resourcefile.h"

using namespace KABC;

ResourceFile::ResourceFile( const KConfig *config )
  : Resource( config ), mFormat( 0 )
{
  QString fileName, formatName;

  if ( config ) {
    fileName = config->readPathEntry( "FileName", StdAddressBook::fileName() );
    formatName = config->readEntry( "FileFormat", "vcard" );
  } else {
    fileName = StdAddressBook::fileName();
    formatName = "vcard";
  }

  init( fileName, formatName );
}

ResourceFile::ResourceFile( const QString &fileName,
                            const QString &formatName )
  : Resource( 0 )
{
  init( fileName, formatName );
}

void ResourceFile::init( const QString &fileName, const QString &formatName )
{
  mFormatName = formatName;

  FormatFactory *factory = FormatFactory::self();
  mFormat = factory->format( mFormatName );

  if ( !mFormat ) {
    mFormatName = "vcard";
    mFormat = factory->format( mFormatName );
  }

  connect( &mDirWatch, SIGNAL( dirty(const QString&) ), SLOT( fileChanged() ) );
  connect( &mDirWatch, SIGNAL( created(const QString&) ), SLOT( fileChanged() ) );
  connect( &mDirWatch, SIGNAL( deleted(const QString&) ), SLOT( fileChanged() ) );

  setFileName( fileName );
}

ResourceFile::~ResourceFile()
{
  delete mFormat;
  mFormat = 0;
}

void ResourceFile::writeConfig( KConfig *config )
{
  Resource::writeConfig( config );

  config->writePathEntry( "FileName", mFileName );
  config->writeEntry( "FileFormat", mFormatName );
}

Ticket *ResourceFile::requestSaveTicket()
{
  kdDebug(5700) << "ResourceFile::requestSaveTicket()" << endl;

  if ( !addressBook() ) return 0;

  if ( !lock( mFileName ) ) {
    kdDebug(5700) << "ResourceFile::requestSaveTicket(): Unable to lock file '"
                  << mFileName << "'" << endl;
    return 0;
  }
  return createTicket( this );
}

void ResourceFile::releaseSaveTicket( Ticket *ticket )
{
  delete ticket;
  unlock( mFileName );
}

bool ResourceFile::doOpen()
{
  QFile file( mFileName );

  if ( !file.exists() ) {
    // try to create the file
    bool ok = file.open( IO_WriteOnly );
    if ( ok )
      file.close();

    return ok;
  } else {
    if ( !file.open( IO_ReadWrite ) )
      return false;

    if ( file.size() == 0 ) {
      file.close();
      return true;
    }

    bool ok = mFormat->checkFormat( &file );
    file.close();

    return ok;
  }
}

void ResourceFile::doClose()
{
}

bool ResourceFile::load()
{
  kdDebug(5700) << "ResourceFile::load(): '" << mFileName << "'" << endl;

  QFile file( mFileName );
  if ( !file.open( IO_ReadOnly ) ) {
    addressBook()->error( i18n( "Unable to open file '%1'." ).arg( mFileName ) );
    return false;
  }

  return mFormat->loadAll( addressBook(), this, &file );
}

bool ResourceFile::asyncLoad()
{
  if ( mLocalTempFile ) {
    kdDebug(5700) << "stale temp file dedected " << mLocalTempFile->name() << endl;
    mLocalTempFile->setAutoDelete( true );
    delete mLocalTempFile;
  }

  mLocalTempFile = new KTempFile();
  mTempFile = mLocalTempFile->name();

  KURL dest, src;
  dest.setPath( mTempFile );
  src.setPath( mFileName );

  KIO::Scheduler::checkSlaveOnHold( true );
  KIO::Job * job = KIO::file_copy( src, dest, -1, true, false );
  connect( job, SIGNAL( result( KIO::Job* ) ),
           this, SLOT( downloadFinished( KIO::Job* ) ) );

  return true;
}

bool ResourceFile::save( Ticket* )
{
  kdDebug(5700) << "ResourceFile::save()" << endl;

  // create backup file
  QString extension = "_" + QString::number( QDate::currentDate().dayOfWeek() );
  (void) KSaveFile::backupFile( mFileName, QString::null /*directory*/,
                                extension );

  KSaveFile saveFile( mFileName );
  bool ok = false;
  if ( saveFile.status() == 0 && saveFile.file() )
  {
    mFormat->saveAll( addressBook(), this, saveFile.file() );
    ok = saveFile.close();
  }

  if ( !ok )
    addressBook()->error( i18n( "Unable to save file '%1'." ).arg( mFileName ) );

  return ok;
}

bool ResourceFile::asyncSave( Ticket* )
{
  QFile file( mTempFile );

  if ( !file.open( IO_WriteOnly ) ) {
    emit savingError( this, i18n( "Unable to open file '%1'." ).arg( mTempFile ) );
    return false;
  }
  
  mFormat->saveAll( addressBook(), this, &file );
  file.close();

  KURL src, dest;
  src.setPath( mTempFile );
  dest.setPath( mFileName );

  KIO::Scheduler::checkSlaveOnHold( true );
  KIO::Job * job = KIO::file_copy( src, dest, -1, true, false );
  connect( job, SIGNAL( result( KIO::Job* ) ),
           this, SLOT( uploadFinished( KIO::Job* ) ) );

  return true;
}

bool ResourceFile::lock( const QString &fileName )
{
  kdDebug(5700) << "ResourceFile::lock()" << endl;

  QString fn = fileName;
  fn.replace( "/", "_" );

  QString lockName = locateLocal( "data", "kabc/lock/" + fn + ".lock" );
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
  lockUniqueName = fn + kapp->randomString( 8 );
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

void ResourceFile::unlock( const QString &fileName )
{
  QString fn = fileName;
  fn.replace( "/" , "_" );

  QString lockName = locateLocal( "data", "kabc/lock/" + fn + ".lock" );
  QFile::remove( lockName );
  QFile::remove( mLockUniqueName );
  addressBook()->emitAddressBookUnlocked();
}

void ResourceFile::setFileName( const QString &fileName )
{
  mDirWatch.stopScan();
  if ( mDirWatch.contains( mFileName ) )
    mDirWatch.removeFile( mFileName );

  mFileName = fileName;

  mDirWatch.addFile( mFileName );
  mDirWatch.startScan();
}

QString ResourceFile::fileName() const
{
  return mFileName;
}

void ResourceFile::setFormat( const QString &format )
{
  mFormatName = format;
  delete mFormat;

  FormatFactory *factory = FormatFactory::self();
  mFormat = factory->format( mFormatName );
}

QString ResourceFile::format() const
{
  return mFormatName;
}

void ResourceFile::fileChanged()
{
  if ( !addressBook() )
    return;

  addressBook()->emitAddressBookChanged();
}

void ResourceFile::removeAddressee( const Addressee &addr )
{
  QFile::remove( QFile::encodeName( locateLocal( "data", "kabc/photos/" ) + addr.uid() ) );
  QFile::remove( QFile::encodeName( locateLocal( "data", "kabc/logos/" ) + addr.uid() ) );
  QFile::remove( QFile::encodeName( locateLocal( "data", "kabc/sounds/" ) + addr.uid() ) );

  mAddressees.remove( addr );
}

void ResourceFile::cleanUp()
{
  unlock( mFileName );
}

void ResourceFile::downloadFinished( KIO::Job* )
{
  if ( !mLocalTempFile )
    emit loadingError( this, i18n( "Download failed in some way!" ) );

  QFile file( mTempFile );
  if ( !file.open( IO_ReadOnly ) ) {
    emit loadingError( this, i18n( "Unable to open file '%1'." ).arg( mTempFile ) );
    return;
  }

  if ( !mFormat->loadAll( addressBook(), this, &file ) )
    emit loadingError( this, i18n( "Problems during parsing file '%1'." ).arg( mTempFile ) );
  else
    emit loadingFinished( this );
}

void ResourceFile::uploadFinished( KIO::Job *job )
{
  if ( job->error() )
    emit savingError( this, job->errorString() );
  else
    emit savingFinished( this );
}

#include "resourcefile.moc"
