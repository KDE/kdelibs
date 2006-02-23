/*
    This file is part of libkabc.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include <QFile>

#include <kdebug.h>
#include <kio/netaccess.h>
#include <kio/scheduler.h>
#include <klocale.h>
#include <ksavefile.h>
#include <ktempfile.h>
#include <kurlrequester.h>

#include "addressbook.h"
#include "formatfactory.h"
#include "resourcenetconfig.h"
#include "stdaddressbook.h"

#include "resourcenet.h"

using namespace KABC;

class ResourceNet::ResourceNetPrivate
{
  public:
    KIO::Job *mLoadJob;
    bool mIsLoading;

    KIO::Job *mSaveJob;
    bool mIsSaving;
};

ResourceNet::ResourceNet( const KConfig *config )
  : Resource( config ), mFormat( 0 ),
    mTempFile( 0 ),
    d( new ResourceNetPrivate )
{
  if ( config ) {
    init( KUrl( config->readPathEntry( "NetUrl" ) ), config->readEntry( "NetFormat" ) );
  } else {
    init( KUrl(), "vcard" );
  }
}

ResourceNet::ResourceNet( const KUrl &url, const QString &format )
  : Resource( 0 ), mFormat( 0 ),
    mTempFile( 0 ),
    d( new ResourceNetPrivate )
{
  init( url, format );
}

void ResourceNet::init( const KUrl &url, const QString &format )
{
  d->mLoadJob = 0;
  d->mIsLoading = false;
  d->mSaveJob = 0;
  d->mIsSaving = false;

  mFormatName = format;

  FormatFactory *factory = FormatFactory::self();
  mFormat = factory->format( mFormatName );
  if ( !mFormat ) {
    mFormatName = "vcard";
    mFormat = factory->format( mFormatName );
  }

  setUrl( url );
}

ResourceNet::~ResourceNet()
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

void ResourceNet::writeConfig( KConfig *config )
{
  Resource::writeConfig( config );

  config->writePathEntry( "NetUrl", mUrl.url() );
  config->writeEntry( "NetFormat", mFormatName );
}

Ticket *ResourceNet::requestSaveTicket()
{
  kDebug(5700) << "ResourceNet::requestSaveTicket()" << endl;

  return createTicket( this );
}

void ResourceNet::releaseSaveTicket( Ticket *ticket )
{
  delete ticket;
}

bool ResourceNet::doOpen()
{
  return true;
}

void ResourceNet::doClose()
{
}

bool ResourceNet::load()
{
  QString tempFile;

  if ( !KIO::NetAccess::download( mUrl, tempFile, 0 ) ) {
    addressBook()->error( i18n( "Unable to download file '%1'." ).arg( mUrl.prettyURL() ) );
    return false;
  }

  QFile file( tempFile );
  if ( !file.open( QIODevice::IO_ReadOnly ) ) {
    addressBook()->error( i18n( "Unable to open file '%1'." ).arg( tempFile ) );
    KIO::NetAccess::removeTempFile( tempFile );
    return false;
  }

  bool result = clearAndLoad( &file );
  if ( !result )
      addressBook()->error( i18n( "Problems during parsing file '%1'." ).arg( tempFile ) );

  KIO::NetAccess::removeTempFile( tempFile );

  return result;
}

bool ResourceNet::clearAndLoad( QFile *file )
{
  clear();
  return mFormat->loadAll( addressBook(), this, file );
}

bool ResourceNet::asyncLoad()
{
  if ( d->mIsLoading ) {
    abortAsyncLoading();
  }

  if (d->mIsSaving) {
    kWarning(5700) << "Aborted asyncLoad() because we're still asyncSave()ing!" << endl;
    return false;
  }

  bool ok = createLocalTempFile();
  if ( ok )
    ok = mTempFile->close();

  if ( !ok ) {
    emit loadingError( this, i18n( "Unable to open file '%1'." ).arg( mTempFile->name() ) );
    deleteLocalTempFile();
    return false;
  }

  KUrl dest;
  dest.setPath( mTempFile->name() );

  KIO::Scheduler::checkSlaveOnHold( true );
  d->mLoadJob = KIO::file_copy( mUrl, dest, -1, true, false, false );
  d->mIsLoading = true;
  connect( d->mLoadJob, SIGNAL( result( KIO::Job* ) ),
           this, SLOT( downloadFinished( KIO::Job* ) ) );

  return true;
}

void ResourceNet::abortAsyncLoading()
{
  kDebug(5700) << "ResourceNet::abortAsyncLoading()" << endl;

  if ( d->mLoadJob ) {
    d->mLoadJob->kill(); // result not emitted
    d->mLoadJob = 0;
  }

  deleteLocalTempFile();
  d->mIsLoading = false;
}

void ResourceNet::abortAsyncSaving()
{
  kDebug(5700) << "ResourceNet::abortAsyncSaving()" << endl;

  if ( d->mSaveJob ) {
    d->mSaveJob->kill(); // result not emitted
    d->mSaveJob = 0;
  }

  deleteLocalTempFile();
  d->mIsSaving = false;
}

bool ResourceNet::save( Ticket* )
{
  kDebug(5700) << "ResourceNet::save()" << endl;

  if (d->mIsSaving) {
    abortAsyncSaving();
  }

  KTempFile tempFile;
  tempFile.setAutoDelete( true );
  bool ok = false;

  if ( tempFile.status() == 0 && tempFile.file() ) {
    saveToFile( tempFile.file() );
    ok = tempFile.close();
  }

  if ( !ok ) {
    addressBook()->error( i18n( "Unable to save file '%1'." ).arg( tempFile.name() ) );
    return false;
  }

  ok = KIO::NetAccess::upload( tempFile.name(), mUrl, 0 );
  if ( !ok )
    addressBook()->error( i18n( "Unable to upload to '%1'." ).arg( mUrl.prettyURL() ) );

  return ok;
}

bool ResourceNet::asyncSave( Ticket* )
{
  kDebug(5700) << "ResourceNet::asyncSave()" << endl;

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
    emit savingError( this, i18n( "Unable to save file '%1'." ).arg( mTempFile->name() ) );
    deleteLocalTempFile();
    return false;
  }

  KUrl src;
  src.setPath( mTempFile->name() );

  KIO::Scheduler::checkSlaveOnHold( true );
  d->mIsSaving = true;
  d->mSaveJob = KIO::file_copy( src, mUrl, -1, true, false, false );
  connect( d->mSaveJob, SIGNAL( result( KIO::Job* ) ),
           this, SLOT( uploadFinished( KIO::Job* ) ) );

  return true;
}

bool ResourceNet::createLocalTempFile()
{
  deleteStaleTempFile();
  mTempFile = new KTempFile();
  mTempFile->setAutoDelete( true );
  return mTempFile->status() == 0;
}

void ResourceNet::deleteStaleTempFile()
{
  if ( hasTempFile() ) {
    kDebug(5700) << "stale temp file detected " << mTempFile->name() << endl;
    deleteLocalTempFile();
  }
}

void ResourceNet::deleteLocalTempFile()
{
  delete mTempFile;
  mTempFile = 0;
}

void ResourceNet::saveToFile( QFile *file )
{
  mFormat->saveAll( addressBook(), this, file );
}

void ResourceNet::setUrl( const KUrl &url )
{
  mUrl = url;
}

KUrl ResourceNet::url() const
{
  return mUrl;
}

void ResourceNet::setFormat( const QString &name )
{
  mFormatName = name;
  if ( mFormat )
    delete mFormat;

  FormatFactory *factory = FormatFactory::self();
  mFormat = factory->format( mFormatName );
}

QString ResourceNet::format() const
{
  return mFormatName;
}

void ResourceNet::downloadFinished( KIO::Job* )
{
  kDebug(5700) << "ResourceNet::downloadFinished()" << endl;

  d->mIsLoading = false;

  if ( !hasTempFile() || mTempFile->status() != 0 ) {
    emit loadingError( this, i18n( "Download failed in some way!" ) );
    return;
  }

  QFile file( mTempFile->name() );
  if ( file.open( QIODevice::IO_ReadOnly ) ) {
    if ( clearAndLoad( &file ) )
      emit loadingFinished( this );
    else
      emit loadingError( this, i18n( "Problems during parsing file '%1'." ).arg( mTempFile->name() ) );
  }
  else {
    emit loadingError( this, i18n( "Unable to open file '%1'." ).arg( mTempFile->name() ) );
  }

  deleteLocalTempFile();
}

void ResourceNet::uploadFinished( KIO::Job *job )
{
  kDebug(5700) << "ResourceFile::uploadFinished()" << endl;

  d->mIsSaving = false;

  if ( job->error() )
    emit savingError( this, job->errorString() );
  else
    emit savingFinished( this );

  deleteLocalTempFile();
}

#include "resourcenet.moc"
