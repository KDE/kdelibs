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

#include <kapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>

#include "stdaddressbook.h"

#include "resourcefactory.h"
#include "resourcefile.h"
#include "vcardformatplugin.h"

using namespace KABC;

AddressBook *StdAddressBook::mSelf = 0;

QString StdAddressBook::fileName()
{
  return locateLocal( "data", "kabc/std.vcf" );
}

QString StdAddressBook::directoryName()
{
  return locateLocal( "data", "kabc/stdvcf" );
}

AddressBook *StdAddressBook::self()
{
  kdDebug(5700) << "StdAddressBook::self()" << endl;

  if ( !mSelf ) {
    mSelf = new StdAddressBook;
  }

  return mSelf;
}

bool StdAddressBook::save()
{
  kdDebug(5700) << "StdAddressBook::save()" << endl;

  return self()->saveAll();
}


StdAddressBook::StdAddressBook()
{
  kdDebug(5700) << "StdAddressBook::StdAddressBook()" << endl;

  init( false );
}

StdAddressBook::StdAddressBook( bool onlyFastResources )
{
  kdDebug(5700) << "StdAddressBook::StdAddressBook( bool )" << endl;

  init( onlyFastResources );
}

StdAddressBook::~StdAddressBook()
{
  save();
}

void StdAddressBook::init( bool onlyFastResources )
{
  KSimpleConfig config( "kabcrc", true );
  ResourceFactory *factory = ResourceFactory::self();
  config.setGroup( "General" );

  QStringList keys = config.readListEntry( "ResourceKeys" );
  QString stdKey = config.readEntry( "Standard" );
  for ( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
    config.setGroup( "Resource_" + (*it) );
    QString type = config.readEntry( "ResourceType" );


    if ( onlyFastResources && !config.readBoolEntry( "ResourceIsFast" ) )
        continue;

    Resource *resource = factory->resource( type, this, &config );

    if ( !resource ) continue;

    resource->setReadOnly( config.readBoolEntry( "ResourceIsReadOnly" ) );
    resource->setFastResource( config.readBoolEntry( "ResourceIsFast" ) );
    resource->setName( config.readEntry( "ResourceName" ).latin1() );

    if ( !addResource( resource ) ) {
      delete resource;
      continue;
    }

    if ( stdKey == (*it) )
      setStandardResource( resource );
  }

  QPtrList<Resource> list = resources();
  if ( list.count() == 0 ) {  // default resource
    kdDebug(5700) << "StdAddressBook(): using default resource" << endl;

    Resource *resource = new ResourceFile( this, fileName(),
                                           new VCardFormatPlugin );
    resource->setReadOnly( false );
    resource->setFastResource( true );

    if ( !addResource( resource ) ) delete resource;

    setStandardResource( resource );
  }

  load();
}

void StdAddressBook::close()
{
  delete mSelf;
  mSelf = 0;
}
