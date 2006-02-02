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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <stdlib.h>

#include <kapplication.h>
#include <kcrash.h>
#include <kdebug.h>
#include <klocale.h>
#include <kresources/manager.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>

#include "resource.h"

#include "stdaddressbook.h"

using namespace KABC;

StdAddressBook *StdAddressBook::mSelf = 0;
bool StdAddressBook::mAutomaticSave = true;

static KStaticDeleter<StdAddressBook> addressBookDeleter;

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
}

StdAddressBook *StdAddressBook::self()
{
  kDebug(5700) << "StdAddressBook::self()" << endl;

  if ( !mSelf )
    addressBookDeleter.setObject( mSelf, new StdAddressBook );

  return mSelf;
}

StdAddressBook *StdAddressBook::self( bool asynchronous )
{
  kDebug(5700) << "StdAddressBook::self()" << endl;

  if ( !mSelf )
    addressBookDeleter.setObject( mSelf, new StdAddressBook( asynchronous ) );

  return mSelf;
}

StdAddressBook::StdAddressBook()
  : AddressBook( "" )
{
  kDebug(5700) << "StdAddressBook::StdAddressBook()" << endl;

  init( false );
}

StdAddressBook::StdAddressBook( bool asynchronous )
  : AddressBook( "" )
{
  kDebug(5700) << "StdAddressBook::StdAddressBook( bool )" << endl;

  init( asynchronous );
}

StdAddressBook::~StdAddressBook()
{
  if ( mAutomaticSave )
    saveAll();
}

void StdAddressBook::init( bool asynchronous )
{
  KRES::Manager<Resource> *manager = resourceManager();

  KRES::Manager<Resource>::ActiveIterator it;
  for ( it = manager->activeBegin(); it != manager->activeEnd(); ++it ) {
    (*it)->setAddressBook( this );
    if ( !(*it)->open() ) {
      error( QString( "Unable to open resource '%1'!" ).arg( (*it)->resourceName() ) );
      continue;
    }
    connect( *it, SIGNAL( loadingFinished( Resource* ) ),
             this, SLOT( resourceLoadingFinished( Resource* ) ) );
    connect( *it, SIGNAL( savingFinished( Resource* ) ),
             this, SLOT( resourceSavingFinished( Resource* ) ) );

    connect( *it, SIGNAL( loadingError( Resource*, const QString& ) ),
             this, SLOT( resourceLoadingError( Resource*, const QString& ) ) );
    connect( *it, SIGNAL( savingError( Resource*, const QString& ) ),
             this, SLOT( resourceSavingError( Resource*, const QString& ) ) );
  }

  Resource *res = standardResource();
  if ( !res ) {
    res = manager->createResource( "file" );
    if ( res )
      addResource( res );
    else
      kDebug(5700) << "No resource available!!!" << endl;
  }

  setStandardResource( res );
  manager->writeConfig();

  if ( asynchronous )
    asyncLoad();
  else
    load();
}

bool StdAddressBook::saveAll()
{
  kDebug(5700) << "StdAddressBook::saveAll()" << endl;
  bool ok = true;

  deleteRemovedAddressees();

  KRES::Manager<Resource>::ActiveIterator it;
  KRES::Manager<Resource> *manager = resourceManager();
  for ( it = manager->activeBegin(); it != manager->activeEnd(); ++it ) {
    if ( !(*it)->readOnly() && (*it)->isOpen() ) {
      Ticket *ticket = requestSaveTicket( *it );
      if ( !ticket ) {
        error( i18n( "Unable to save to resource '%1'. It is locked." )
                   .arg( (*it)->resourceName() ) );
        return false;
      }

      if ( !AddressBook::save( ticket ) ) {
        ok = false;
        releaseSaveTicket( ticket );
      }
    }
  }

  return ok;
}

bool StdAddressBook::save()
{
  kDebug(5700) << "StdAddressBook::save()" << endl;

  if ( mSelf ) 
    return mSelf->saveAll();
  else
    return true;  
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

// should get const for 4.X
Addressee StdAddressBook::whoAmI()
{
  KConfig config( "kabcrc" );
  config.setGroup( "General" );

  return findByUid( config.readEntry( "WhoAmI" ) );
}

void StdAddressBook::setWhoAmI( const Addressee &addr )
{
  KConfig config( "kabcrc" );
  config.setGroup( "General" );

  config.writeEntry( "WhoAmI", addr.uid() );
}
