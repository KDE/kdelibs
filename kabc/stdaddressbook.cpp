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
#include "vcardformat.h"

using namespace KABC;

AddressBook *StdAddressBook::mSelf = 0;

QString StdAddressBook::fileName()
{
  return locateLocal( "data", "kabc/stdvcf" );
}

AddressBook *StdAddressBook::self( bool onlyFastResource )
{
  kdDebug(5700) << "StdAddressBook::self()" << endl;

  if ( !mSelf ) {
    mSelf = new StdAddressBook( onlyFastResource );
  }

  return mSelf;
}

bool StdAddressBook::save()
{
  kdDebug(5700) << "StdAddressBook::save()" << endl;

  return self()->saveAll();
}


StdAddressBook::StdAddressBook( bool onlyFastResource )
{
  kdDebug(5700) << "StdAddressBook::StdAddressBook()" << endl;

  KSimpleConfig config( "kabcrc", true );
  ResourceFactory *factory = ResourceFactory::self();
  config.setGroup( "General" );

  QStringList keys = config.readListEntry( "ResourceKeys" );
  for ( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
    config.setGroup( "Resource_" + (*it) );
    QString type = config.readEntry( "ResourceType" );

    if ( onlyFastResource && !config.readBoolEntry( "ResourceIsFast" ) )
        continue;

    Resource *resource = factory->resource( type, this, &config );

    if ( !resource ) continue;

    resource->setReadOnly( config.readBoolEntry( "ResourceIsReadOnly" ) );
    resource->setFastResource( config.readBoolEntry( "ResourceIsFast" ) );
    resource->setName( config.readEntry( "ResourceName" ) );

    if ( !addResource( resource ) ) delete resource;
  }

  QPtrList<Resource> list = resources();
  if ( list.count() == 0 ) {  // default resource
    kdDebug(5700) << "StdAddressBook(): using default resource" << endl;

    Resource *resource = new ResourceFile( this, fileName(), new VCardFormat );
    resource->setName( "Default" );
    resource->setReadOnly( false );
    resource->setFastResource( true );

    if ( !addResource( resource ) ) delete resource;
  }

  load();
}

StdAddressBook::~StdAddressBook()
{
  save();
}
