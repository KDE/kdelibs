/*
    This file is part of libkabc.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <QFile>
#include <QFileInfo>

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
#include "lock.h"

#include "resourcefile.h"

using namespace KABC;

class ResourceFile::ResourceFilePrivate
{
  public:
    KIO::Job *mLoadJob;
    bool mIsLoading;

    KIO::Job *mSaveJob;
    bool mIsSaving;
};

ResourceFile::ResourceFile( const KConfig *config )
  : Resource( config ), mFormat( 0 ), mLocalTempFile( 0 ),
    mAsynchronous( false ), d( new ResourceFilePrivate )
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
  : Resource( 0 ), mFormat( 0 ), mLocalTempFile( 0 ),
    mAsynchronous( false ), d( new ResourceFilePrivate )
{
  init( fileName, formatName );
}

void ResourceFile::init( const QString &fileName, const QString &formatName )
{
  d->mLoadJob = 0;
  d->mIsLoading = false;
  d->mSaveJob = 0;
  d->mIsSaving = false;

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

  mLock = 0;
}

ResourceFile::~ResourceFile()
{
  if ( d->mIsLoading )
    d->mLoadJob->kill();
  if ( d->mIsSaving )
    d->mSaveJob->kill();

  delete d;
  d = 0;
  delete mFormat;
  mFormat = 0;
  delete mLocalTempFile;
  mLocalTempFile = 0;
}

void ResourceFile::writeConfig( KConfig *config )
{
  Resource::writeConfig( config );

  if ( mFileName == StdAddressBook::fileName() )
    config->deleteEntry( "FileName" );
  else
    config->writePathEntry( "FileName", mFileName );

  config->writeEntry( "FileFormat", mFormatName );
}

Ticket *ResourceFile::requestSaveTicket()
{
  kdDebug(5700) << "ResourceFile::requestSaveTicket()" << endl;

  if ( !addressBook() ) return 0;

  delete mLock;
  mLock = new Lock( mFileName );

  if ( mLock->lock() ) {
    addressBook()->emitAddressBookLocked();
  } else {
    addressBook()->error( mLock->error() );
    kdDebug(5700) << "ResourceFile::requestSaveTicket(): Unable to lock file '"
                  << mFileName << "': " << mLock->error() << endl;
    return 0;
  }

  return createTicket( this );
}

void ResourceFile::releaseSaveTicket( Ticket *ticket )
{
  delete ticket;

  delete mLock;
  mLock = 0;

  addressBook()->emitAddressBookUnlocked();
}

bool ResourceFile::doOpen()
{
  QFile file( mFileName );

  if ( !file.exists() ) {
    // try to create the file
    bool ok = file.open( QIODevice::WriteOnly );
    if ( ok )
      file.close();

    return ok;
  } else {
    QFileInfo fileInfo( mFileName );
    if ( readOnly() || !fileInfo.isWritable() ) {
      if ( !file.open( QIODevice::ReadOnly ) )
        return false;
    } else {
      if ( !file.open( QIODevice::ReadWrite ) )
        return false;
    }

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

  mAsynchronous = false;

  QFile file( mFileName );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    addressBook()->error( i18n( "Unable to open file '%1'." ).arg( mFileName ) );
    return false;
  }

  return mFormat->loadAll( addressBook(), this, &file );
}

bool ResourceFile::asyncLoad()
{
  mAsynchronous = true;

  if ( mLocalTempFile ) {
    kdDebug(5700) << "stale temp file detected " << mLocalTempFile->name() << endl;
    delete mLocalTempFile;
  }

  mLocalTempFile = new KTempFile();
  mLocalTempFile->setAutoDelete( true );
  mTempFile = mLocalTempFile->name();

  KUrl dest, src;
  dest.setPath( mTempFile );
  src.setPath( mFileName );

  KIO::Scheduler::checkSlaveOnHold( true );
  d->mLoadJob = KIO::file_copy( src, dest, -1, true, false, false );
  d->mIsLoading = true;
  connect( d->mLoadJob, SIGNAL( result( KIO::Job* ) ),
           this, SLOT( downloadFinished( KIO::Job* ) ) );

  return true;
}

bool ResourceFile::save( Ticket * )
{
  kdDebug(5700) << "ResourceFile::save()" << endl;

  // create backup file
  QString extension = "_" + QString::number( QDate::currentDate().dayOfWeek() );
  (void) KSaveFile::simpleBackupFile( mFileName, QString() /*directory*/,
                                extension );

  mDirWatch.stopScan();
  KSaveFile saveFile( mFileName );
  bool ok = false;
  if ( saveFile.status() == 0 && saveFile.file() )
  {
    mFormat->saveAll( addressBook(), this, saveFile.file() );
    ok = saveFile.close();
  }

  if ( !ok )
    addressBook()->error( i18n( "Unable to save file '%1'." ).arg( mFileName ) );
  mDirWatch.startScan();

  return ok;
}

bool ResourceFile::asyncSave( Ticket * )
{
  QFile file( mTempFile );

  if ( !file.open( QIODevice::WriteOnly ) ) {
    emit savingError( this, i18n( "Unable to open file '%1'." ).arg( mTempFile ) );
    return false;
  }

  mDirWatch.stopScan();
  mFormat->saveAll( addressBook(), this, &file );
  file.close();

  KUrl src, dest;
  src.setPath( mTempFile );
  dest.setPath( mFileName );

  KIO::Scheduler::checkSlaveOnHold( true );
  d->mSaveJob = KIO::file_copy( src, dest, -1, true, false, false );
  d->mIsSaving = true;
  connect( d->mSaveJob, SIGNAL( result( KIO::Job* ) ),
           this, SLOT( uploadFinished( KIO::Job* ) ) );

  return true;
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

  clear();
  if ( mAsynchronous )
    asyncLoad();
  else {
    load();
    kdDebug() << "addressBookChanged() " << endl;
    addressBook()->emitAddressBookChanged();
  }
}

void ResourceFile::removeAddressee( const Addressee &addr )
{
  QFile::remove( QFile::encodeName( locateLocal( "data", "kabc/photos/" ) + addr.uid() ) );
  QFile::remove( QFile::encodeName( locateLocal( "data", "kabc/logos/" ) + addr.uid() ) );
  QFile::remove( QFile::encodeName( locateLocal( "data", "kabc/sounds/" ) + addr.uid() ) );

  mAddrMap.remove( addr.uid() );
}

void ResourceFile::downloadFinished( KIO::Job* )
{
  d->mIsLoading = false;

  if ( !mLocalTempFile )
    emit loadingError( this, i18n( "Download failed in some way!" ) );

  QFile file( mTempFile );
  if ( !file.open( QIODevice::ReadOnly ) ) {
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
  d->mIsSaving = false;

  if ( job->error() )
    emit savingError( this, job->errorString() );
  else
    emit savingFinished( this );
  mDirWatch.startScan();
}

#include "resourcefile.moc"
