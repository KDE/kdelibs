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

extern "C" {

// Crash recovery signal handler
static void crashHandler( int sigId )
{
  /**
    To avoid all such 'KAddressBook crashes' mails I comment it out now.
    There seems to be a problem with KCrash, since this crashHandler is
    called, even if you close KAddressBook the normal way. Nevertheless
    if you comment out KCrash::setEmergencySaveFunction in init(), no
    crash happens at closing the app :/
   */
//  fprintf( stderr, "*** libkabc got signal %d (Crashing)\n", sigId );

  // try to cleanup all lock files
  AddressBook *ab = StdAddressBook::self();
  if ( ab )
    ab->cleanUp();

  ::exit( 0 );
  // Return to DrKonqi.
}

}

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

bool StdAddressBook::save()
{
  kdDebug(5700) << "StdAddressBook::save()" << endl;

  bool ok = true;
  Resource *resource = 0;

  AddressBook *ab = self();

  ab->deleteRemovedAddressees();

  QPtrList<Resource> list = ab->resources();
  for ( uint i = 0; i < list.count(); ++i ) {
    resource = list.at( i );
    if ( !resource->readOnly() ) {
      Ticket *ticket = ab->requestSaveTicket( resource );
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
  if ( mAutomaticSave )
    save();
}

void StdAddressBook::init( bool )
{
  KRES::ResourceManager<Resource> manager( "contact" );

  KRES::ResourceManager<Resource>::Iterator it;
  for( it = manager.begin(); it != manager.end(); ++it ) {
    if ( (*it)->isActive() ) {
      addResource( *it );
    }
  }

  Resource *res = manager.standardResource();
  if ( !res ) {
    res = manager.createResource( "file" );
    if ( res )
      addResource( res );
    else
      kdDebug(5700) << "No resource available!!!" << endl;
  }

  setStandardResource( res );

  load();

  KCrash::setEmergencySaveFunction( crashHandler );
}

void StdAddressBook::close()
{
  delete mSelf;
  mSelf = 0;
}

void StdAddressBook::setAutomaticSave( bool enable )
{
  mAutomaticSave = enable;
}

bool StdAddressBook::automaticSave()
{
  return mAutomaticSave;
}
