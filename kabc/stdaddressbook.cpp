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

#include <stdlib.h>

#include <kapplication.h>
#include <kcrash.h>
#include <kdebug.h>
#include <klocale.h>
#include <kresources/resourcemanager.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>

#include "resource.h"

#include "stdaddressbook.h"

using namespace KABC;

AddressBook *StdAddressBook::mSelf = 0;
bool StdAddressBook::mAutomaticSave = true;

static KStaticDeleter<AddressBook> addressBookDeleter;

QString StdAddressBook::fileName()
{
  return locateLocal( "data", "kabc/std.vcf" );
}

QString StdAddressBook::directoryName()
{
  return locateLocal( "data", "kabc/stdvcf" );
}

void StdAddressBook::handleCrash()
{
  StdAddressBook::self()->cleanUp();
}

AddressBook *StdAddressBook::self()
{
  kdDebug(5700) << "StdAddressBook::self()" << endl;

  if ( !mSelf )
    addressBookDeleter.setObject( mSelf, new StdAddressBook );

  return mSelf;
}

AddressBook *StdAddressBook::self( bool onlyFastResources )
{
  kdDebug(5700) << "StdAddressBook::self()" << endl;

  if ( !mSelf )
    addressBookDeleter.setObject( mSelf, new StdAddressBook( onlyFastResources ) );

  return mSelf;
}

StdAddressBook::StdAddressBook()
  : AddressBook( "kabcrc" )
{
  kdDebug(5700) << "StdAddressBook::StdAddressBook()" << endl;

  init( false );
}

StdAddressBook::StdAddressBook( bool onlyFastResources )
  : AddressBook( "kabcrc" )
{
  kdDebug(5700) << "StdAddressBook::StdAddressBook( bool )" << endl;

  init( onlyFastResources );
}

StdAddressBook::~StdAddressBook()
{
  if ( mAutomaticSave )
    save();
}

void StdAddressBook::init( bool )
{
  KRES::ResourceManager<Resource> *manager = resourceManager();
  manager->load();

  KRES::ResourceManager<Resource>::ActiveIterator it;
  for ( it = manager->activeBegin(); it != manager->activeEnd(); ++it ) {
    (*it)->setAddressBook( this );
    if ( !(*it)->open() ) {
      error( QString( "Unable to open resource '%1'!" ).arg( (*it)->resourceName() ) );
      manager->remove( *it );
    }
  }

  Resource *res = standardResource();
  if ( !res ) {
    res = manager->createResource( "file" );
    if ( res )
      addResource( res );
    else
      kdDebug(5700) << "No resource available!!!" << endl;
  }

  setStandardResource( res );
  manager->sync(); // write config file

  load();
}

bool StdAddressBook::save()
{
  kdDebug(5700) << "StdAddressBook::save()" << endl;

  bool ok = true;
  AddressBook *ab = self();

  ab->deleteRemovedAddressees();

  KRES::ResourceManager<Resource>::ActiveIterator it;
  KRES::ResourceManager<Resource> *manager = ab->resourceManager();
  for ( it = manager->activeBegin(); it != manager->activeEnd(); ++it ) {
    if ( !(*it)->readOnly() ) {
      Ticket *ticket = ab->requestSaveTicket( *it );
      if ( !ticket ) {
        ab->error( i18n( "Unable to save to standard addressbook. It is locked." ) );
        return false;
      }

      if ( !ab->save( ticket ) )
        ok = false;
    }
  }

  return ok;
}

void StdAddressBook::close()
{
  addressBookDeleter.destructObject();
}

void StdAddressBook::setAutomaticSave( bool enable )
{
  mAutomaticSave = enable;
}

bool StdAddressBook::automaticSave()
{
  return mAutomaticSave;
}
