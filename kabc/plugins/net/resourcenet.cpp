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
#include <klocale.h>
#include <ktempfile.h>
#include <kurlrequester.h>

#include "addressbook.h"
#include "formatfactory.h"
#include "resourcenetconfig.h"
#include "stdaddressbook.h"

#include "resourcenet.h"

using namespace KABC;

extern "C"
{
  void *init_kabc_net()
  {
    return new KRES::PluginFactory<ResourceNet,ResourceNetConfig>();
  }
}


ResourceNet::ResourceNet( const KConfig *config )
    : Resource( config ), mFormat( 0 ),
      mLocalTempFile( 0 ), mUseLocalTempFile( false )
{
  KURL url;

  if ( config ) {
    url = config->readPathEntry( "NetUrl" );
    mFormatName = config->readEntry( "NetFormat" );
  } else {
    url = "";
    mFormatName = "vcard";
  }

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

  if ( !addressBook() )
    return 0;

  if ( mTempFile.isEmpty() )
    return 0;

  return createTicket( this );
}


bool ResourceNet::doOpen()
{
  if ( !KIO::NetAccess::exists( mUrl ) ) {
    mLocalTempFile = new KTempFile();
    mLocalTempFile->setAutoDelete( true );
    mUseLocalTempFile = true;
    mTempFile = mLocalTempFile->name();
    return true;
  }

  return KIO::NetAccess::download( mUrl, mTempFile );
}

void ResourceNet::doClose()
{
  if ( !mUseLocalTempFile )
    KIO::NetAccess::removeTempFile( mTempFile );
}

bool ResourceNet::load()
{
  QFile file( mTempFile );
  if ( !file.open( IO_ReadOnly ) ) {
    addressBook()->error( i18n( "Unable to open file '%1'." ).arg( mUrl.url() ) );
    return false;
  }

  return mFormat->loadAll( addressBook(), this, &file );
}

bool ResourceNet::save( Ticket *ticket )
{
  QFile file( mTempFile );

  if ( !file.open( IO_WriteOnly ) ) {
    addressBook()->error( i18n( "Unable to open file '%1'." ).arg( mUrl.url() ) );
    return false;
  }
  
  mFormat->saveAll( addressBook(), this, &file );
  file.close();

  delete ticket;

  return KIO::NetAccess::upload( mTempFile, mUrl );
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
