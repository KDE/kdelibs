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
  : Resource( config ), mFormat( 0 ), mTempFile( 0 ),
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
  : Resource( 0 ), mFormat( 0 ), mTempFile( 0 ),
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

  deleteLocalTempFile();
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
  kDebug(5700) << "ResourceFile::requestSaveTicket()" << endl;

  if ( !addressBook() ) return 0;

  delete mLock;
  mLock = new Lock( mFileName );

  if ( mLock->lock() ) {
    addressBook()->emitAddressBookLocked();
  } else {
    addressBook()->error( mLock->error() );
    kDebug(5700) << "ResourceFile::requestSaveTicket(): Unable to lock file '"
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
  kDebug(5700) << "ResourceFile::load(): '" << mFileName << "'" << endl;

  if ( d->mIsLoading ) {
    abortAsyncLoading();
  }

  mAsynchronous = false;

  QFile file( mFileName );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    addressBook()->error( i18n( "Unable to open file '%1'." ,  mFileName ) );
    return false;
  }

  if ( !clearAndLoad( &file ) ) {
      addressBook()->error( i18n( "Problems during parsing file '%1'." ,  mFileName ) );
    return false;
  }

  return true;
}

bool ResourceFile::clearAndLoad( QFile *file )
{
  clear();
  return mFormat->loadAll( addressBook(), this, file );
}

bool ResourceFile::asyncLoad()
{
  if ( d->mIsLoading ) {
    abortAsyncLoading();
  }

  if (d->mIsSaving) {
    kWarning(5700) << "Aborted asyncSave() because we're still asyncSave()ing!" << endl;
    return false;
  }

  mAsynchronous = true;

  bool ok = createLocalTempFile();
  if ( ok )
    ok = mTempFile->close(); // we only need the filename

  if ( !ok ) {
    emit loadingError( this, i18n( "Unable to open file '%1'." ,  mTempFile->name() ) );
    deleteLocalTempFile();
    return false;
  }

  KUrl dest, src;
  dest.setPath( mTempFile->name() );
  src.setPath( mFileName );

  KIO::Scheduler::checkSlaveOnHold( true );
  d->mLoadJob = KIO::file_copy( src, dest, -1, true, false, false );
  d->mIsLoading = true;
  connect( d->mLoadJob, SIGNAL( result( KIO::Job* ) ),
           this, SLOT( downloadFinished( KIO::Job* ) ) );

  return true;
}

void ResourceFile::abortAsyncLoading()
{
  kDebug(5700) << "ResourceFile::abortAsyncLoading()" << endl;

  if ( d->mLoadJob ) {
    d->mLoadJob->kill(); // result not emitted
    d->mLoadJob = 0;
  }

  deleteLocalTempFile();
  d->mIsLoading = false;
}

void ResourceFile::abortAsyncSaving()
{
  kDebug(5700) << "ResourceFile::abortAsyncSaving()" << endl;

  if ( d->mSaveJob ) {
    d->mSaveJob->kill(); // result not emitted
    d->mSaveJob = 0;
  }

  deleteLocalTempFile();
  d->mIsSaving = false;
}

bool ResourceFile::save( Ticket * )
{
  kDebug(5700) << "ResourceFile::save()" << endl;

  if (d->mIsSaving) {
    abortAsyncSaving();
  }

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

  if ( !ok ) {
    saveFile.abort();
    addressBook()->error( i18n( "Unable to save file '%1'." ,  mFileName ) );
  }

  mDirWatch.startScan();

  return ok;
}

bool ResourceFile::asyncSave( Ticket * )
{
  kDebug(5700) << "ResourceFile::asyncSave()" << endl;

  if (d->mIsSaving) {
    abortAsyncSaving();
  }

  if (d->mIsLoading) {
    kWarning(5700) << "Aborted asyncSave() because we're still asyncLoad()ing!" << endl;
    return false;
  }

  bool ok = createLocalTempFile();
  if ( ok ) {
    saveToFile( mTempFile->file() );
    ok = mTempFile->close();
  }

  if ( !ok ) {
    emit savingError( this, i18n( "Unable to save file '%1'." ,  mTempFile->name() ) );
    deleteLocalTempFile();
    return false;
  }

  KUrl src, dest;
  src.setPath( mTempFile->name() );
  dest.setPath( mFileName );

  KIO::Scheduler::checkSlaveOnHold( true );
  d->mIsSaving = true;
  mDirWatch.stopScan(); // restarted in uploadFinished()
  d->mSaveJob = KIO::file_copy( src, dest, -1, true, false, false );
  connect( d->mSaveJob, SIGNAL( result( KIO::Job* ) ),
           this, SLOT( uploadFinished( KIO::Job* ) ) );

  return true;
}

bool ResourceFile::createLocalTempFile()
{
  deleteStaleTempFile();
  mTempFile = new KTempFile();
  mTempFile->setAutoDelete( true );
  return mTempFile->status() == 0;
}

void ResourceFile::deleteStaleTempFile()
{
  if ( hasTempFile() ) {
    kDebug(5700) << "stale temp file detected " << mTempFile->name() << endl;
    deleteLocalTempFile();
  }
}

void ResourceFile::deleteLocalTempFile()
{
  delete mTempFile;
  mTempFile = 0;
}

void ResourceFile::saveToFile( QFile *file )
{
  mFormat->saveAll( addressBook(), this, file );
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
    kDebug(5700) << "ResourceFile::fileChanged(): " << mFileName << endl;

  if ( !addressBook() )
    return;

//  clear(); // moved to clearAndLoad()
  if ( mAsynchronous )
    asyncLoad();
  else {
    load();
    kDebug() << "addressBookChanged() " << endl;
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
  kDebug(5700) << "ResourceFile::downloadFinished()" << endl;

  d->mIsLoading = false;

  if ( !hasTempFile() || mTempFile->status() != 0 ) {
    emit loadingError( this, i18n( "Download failed in some way!" ) );
    return;
  }

  QFile file( mTempFile->name() );
  if ( file.open( QIODevice::ReadOnly ) ) {
    if ( clearAndLoad( &file ) )
      emit loadingFinished( this );
    else
      emit loadingError( this, i18n( "Problems during parsing file '%1'." ,  mTempFile->name() ) );
  }
  else {
    emit loadingError( this, i18n( "Unable to open file '%1'." ,  mTempFile->name() ) );
  }

  deleteLocalTempFile();
}

void ResourceFile::uploadFinished( KIO::Job *job )
{
  kDebug(5700) << "ResourceFile::uploadFinished()" << endl;

  d->mIsSaving = false;

  if ( job->error() )
    emit savingError( this, job->errorString() );
  else
    emit savingFinished( this );

  deleteLocalTempFile();
  mDirWatch.startScan();
}

#include "resourcefile.moc"
