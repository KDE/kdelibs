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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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

ResourceNet::ResourceNet( const KConfig *config )
  : Resource( config ), mFormat( 0 ),
    mLocalTempFile( 0 ), mUseLocalTempFile( false )
{
  if ( config ) {
    init( config->readPathEntry( "NetUrl" ), config->readEntry( "NetFormat" ) );
  } else {
    init( KURL(), "vcard" );
  }
}

ResourceNet::ResourceNet( const KURL &url, const QString &format )
  : Resource( 0 ), mFormat( 0 ),
    mLocalTempFile( 0 ), mUseLocalTempFile( false )
{
  init( url, format );
}

void ResourceNet::init( const KURL &url, const QString &format )
{
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
  if ( !KIO::NetAccess::exists( mUrl ) ) {
    mLocalTempFile = new KTempFile();
    mLocalTempFile->setAutoDelete( true );
    mUseLocalTempFile = true;
    mTempFile = mLocalTempFile->name();
  }

  if ( !KIO::NetAccess::download( mUrl, mTempFile ) ) {
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
    kdDebug(5700) << "stale temp file dedected " << mLocalTempFile->name() << endl;
    mLocalTempFile->setAutoDelete( true );
    delete mLocalTempFile;
  }

  mLocalTempFile = new KTempFile();
  mUseLocalTempFile = true;
  mTempFile = mLocalTempFile->name();

  KURL dest;
  dest.setPath( mTempFile );

  KIO::Scheduler::checkSlaveOnHold( true );
  KIO::Job * job = KIO::file_copy( mUrl, dest, -1, true, false );
  connect( job, SIGNAL( result( KIO::Job* ) ),
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

  return KIO::NetAccess::upload( mTempFile, mUrl );
}

bool ResourceNet::asyncSave( Ticket* )
{
  QFile file( mTempFile );

  if ( !file.open( IO_WriteOnly ) ) {
    addressBook()->error( i18n( "Unable to open file '%1'." ).arg( mTempFile ) );
    return false;
  }
  
  mFormat->saveAll( addressBook(), this, &file );
  file.close();

  KURL source;
  source.setPath( mTempFile );

  KIO::Scheduler::checkSlaveOnHold( true );
  KIO::Job * job = KIO::file_copy( source, mUrl, -1, true, false );
  connect( job, SIGNAL( result( KIO::Job* ) ),
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

void ResourceNet::cleanUp()
{
  KIO::NetAccess::removeTempFile( mTempFile );
}

void ResourceNet::downloadFinished( KIO::Job* )
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

void ResourceNet::uploadFinished( KIO::Job *job )
{
  if ( job->error() )
    emit savingError( this, job->errorString() );
  else
    emit savingFinished( this );
}

#include "resourcenet.moc"
