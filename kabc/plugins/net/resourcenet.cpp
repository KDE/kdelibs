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

#include <qfile.h>

#include <kdebug.h>
#include <kio/netaccess.h>
#include <kio/scheduler.h>
#include <klocale.h>
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
    mLocalTempFile( 0 ), mUseLocalTempFile( false ),
    d( new ResourceNetPrivate )
{
  if ( config ) {
    init( KURL( config->readPathEntry( "NetUrl" ) ), config->readEntry( "NetFormat" ) );
  } else {
    init( KURL(), "vcard" );
  }
}

ResourceNet::ResourceNet( const KURL &url, const QString &format )
  : Resource( 0 ), mFormat( 0 ),
    mLocalTempFile( 0 ), mUseLocalTempFile( false ),
    d( new ResourceNetPrivate )
{
  init( url, format );
}

void ResourceNet::init( const KURL &url, const QString &format )
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

  delete mLocalTempFile;
  mLocalTempFile = 0;
}

void ResourceNet::writeConfig( KConfig *config )
{
  Resource::writeConfig( config );

  config->writePathEntry( "NetUrl", mUrl.url() );
  config->writeEntry( "NetFormat", mFormatName );
}

Ticket *ResourceNet::requestSaveTicket()
{
  kdDebug(5700) << "ResourceNet::requestSaveTicket()" << endl;

  if ( mTempFile.isEmpty() )
    return 0;

  return createTicket( this );
}

void ResourceNet::releaseSaveTicket( Ticket *ticket )
{
  KIO::NetAccess::removeTempFile( mTempFile );
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
  if ( !KIO::NetAccess::exists( mUrl, true, 0 ) ) {
    mLocalTempFile = new KTempFile();
    mLocalTempFile->setAutoDelete( true );
    mUseLocalTempFile = true;
    mTempFile = mLocalTempFile->name();
  }

  if ( !KIO::NetAccess::download( mUrl, mTempFile, 0 ) ) {
    addressBook()->error( i18n( "Unable to download file '%1'." ).arg( mUrl.url() ) );
    return false;
  }

  QFile file( mTempFile );
  if ( !file.open( IO_ReadOnly ) ) {
    addressBook()->error( i18n( "Unable to open file '%1'." ).arg( mUrl.url() ) );
    return false;
  }

  return mFormat->loadAll( addressBook(), this, &file );
}

bool ResourceNet::asyncLoad()
{
  if ( mLocalTempFile ) {
    kdDebug(5700) << "stale temp file detected " << mLocalTempFile->name() << endl;
    mLocalTempFile->setAutoDelete( true );
    delete mLocalTempFile;
  }

  mLocalTempFile = new KTempFile();
  mUseLocalTempFile = true;
  mTempFile = mLocalTempFile->name();

  KURL dest;
  dest.setPath( mTempFile );

  KIO::Scheduler::checkSlaveOnHold( true );
  d->mLoadJob = KIO::file_copy( mUrl, dest, -1, true, false, false );
  d->mIsLoading = true;
  connect( d->mLoadJob, SIGNAL( result( KIO::Job* ) ),
           this, SLOT( downloadFinished( KIO::Job* ) ) );

  return true;
}

bool ResourceNet::save( Ticket* )
{
  QFile file( mTempFile );

  if ( !file.open( IO_WriteOnly ) ) {
    addressBook()->error( i18n( "Unable to open file '%1'." ).arg( mUrl.url() ) );
    return false;
  }

  mFormat->saveAll( addressBook(), this, &file );
  file.close();

  return KIO::NetAccess::upload( mTempFile, mUrl, 0 );
}

bool ResourceNet::asyncSave( Ticket* )
{
  QFile file( mTempFile );

  if ( !file.open( IO_WriteOnly ) ) {
    emit savingError( this, i18n( "Unable to open file '%1'." ).arg( mTempFile ) );
    return false;
  }

  mFormat->saveAll( addressBook(), this, &file );
  file.close();

  KURL src;
  src.setPath( mTempFile );

  KIO::Scheduler::checkSlaveOnHold( true );
  d->mSaveJob = KIO::file_copy( src, mUrl, -1, true, false, false );
  d->mIsSaving = true;
  connect( d->mSaveJob, SIGNAL( result( KIO::Job* ) ),
           this, SLOT( uploadFinished( KIO::Job* ) ) );

  return true;
}

void ResourceNet::setUrl( const KURL &url )
{
  mUrl = url;
}

KURL ResourceNet::url() const
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
  d->mIsLoading = false;

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

void ResourceNet::uploadFinished( KIO::Job *job )
{
  d->mIsSaving = false;

  if ( job->error() )
    emit savingError( this, job->errorString() );
  else
    emit savingFinished( this );
}

#include "resourcenet.moc"
