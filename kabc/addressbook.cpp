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

#include <qfile.h>
#include <qregexp.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kinstance.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "errorhandler.h"
#include "resource.h"

#include "addressbook.h"
#include "addressbook.moc"

using namespace KABC;

struct AddressBook::AddressBookData
{
  Addressee::List mAddressees;
  Addressee::List mRemovedAddressees;
  Field::List mAllFields;
  ErrorHandler *mErrorHandler;
  KConfig *mConfig;
  KRES::Manager<Resource> *mManager;
};

struct AddressBook::Iterator::IteratorData
{
  Addressee::List::Iterator mIt;
};

struct AddressBook::ConstIterator::ConstIteratorData
{
  Addressee::List::ConstIterator mIt;
};

AddressBook::Iterator::Iterator()
{
  d = new IteratorData;
}

AddressBook::Iterator::Iterator( const AddressBook::Iterator &i )
{
  d = new IteratorData;
  d->mIt = i.d->mIt;
}

AddressBook::Iterator &AddressBook::Iterator::operator=( const AddressBook::Iterator &i )
{
  if( this == &i ) return *this; // guard against self assignment
  delete d; // delete the old data the Iterator was completely constructed before
  d = new IteratorData;
  d->mIt = i.d->mIt;
  return *this;
}

AddressBook::Iterator::~Iterator()
{
  delete d;
}

const Addressee &AddressBook::Iterator::operator*() const
{
  return *(d->mIt);
}

Addressee &AddressBook::Iterator::operator*()
{
  return *(d->mIt);
}

Addressee *AddressBook::Iterator::operator->()
{
  return &(*(d->mIt));
}

AddressBook::Iterator &AddressBook::Iterator::operator++()
{
  (d->mIt)++;
  return *this;
}

AddressBook::Iterator &AddressBook::Iterator::operator++(int)
{
  (d->mIt)++;
  return *this;
}

AddressBook::Iterator &AddressBook::Iterator::operator--()
{
  (d->mIt)--;
  return *this;
}

AddressBook::Iterator &AddressBook::Iterator::operator--(int)
{
  (d->mIt)--;
  return *this;
}

bool AddressBook::Iterator::operator==( const Iterator &it )
{
  return ( d->mIt == it.d->mIt );
}

bool AddressBook::Iterator::operator!=( const Iterator &it )
{
  return ( d->mIt != it.d->mIt );
}


AddressBook::ConstIterator::ConstIterator()
{
  d = new ConstIteratorData;
}

AddressBook::ConstIterator::ConstIterator( const AddressBook::ConstIterator &i )
{
  d = new ConstIteratorData;
  d->mIt = i.d->mIt;
}

AddressBook::ConstIterator &AddressBook::ConstIterator::operator=( const AddressBook::ConstIterator &i )
{
  if( this  == &i ) return *this; // guard for self assignment
  delete d; // delete the old data because the Iterator was really constructed before
  d = new ConstIteratorData;
  d->mIt = i.d->mIt;
  return *this;
}

AddressBook::ConstIterator::~ConstIterator()
{
  delete d;
}

const Addressee &AddressBook::ConstIterator::operator*() const
{
  return *(d->mIt);
}

const Addressee* AddressBook::ConstIterator::operator->() const
{
  return &(*(d->mIt));
}

AddressBook::ConstIterator &AddressBook::ConstIterator::operator++()
{
  (d->mIt)++;
  return *this;
}

AddressBook::ConstIterator &AddressBook::ConstIterator::operator++(int)
{
  (d->mIt)++;
  return *this;
}

AddressBook::ConstIterator &AddressBook::ConstIterator::operator--()
{
  (d->mIt)--;
  return *this;
}

AddressBook::ConstIterator &AddressBook::ConstIterator::operator--(int)
{
  (d->mIt)--;
  return *this;
}

bool AddressBook::ConstIterator::operator==( const ConstIterator &it )
{
  return ( d->mIt == it.d->mIt );
}

bool AddressBook::ConstIterator::operator!=( const ConstIterator &it )
{
  return ( d->mIt != it.d->mIt );
}


AddressBook::AddressBook()
{
  d = new AddressBookData;
  d->mErrorHandler = 0;
  d->mConfig = 0;
  d->mManager = new KRES::Manager<Resource>( "contact" );
  d->mManager->readConfig();
}

AddressBook::AddressBook( const QString &config )
{
  d = new AddressBookData;
  d->mErrorHandler = 0;
  d->mConfig = new KConfig( config );
  d->mManager = new KRES::Manager<Resource>( "contact" );
  d->mManager->readConfig( d->mConfig );
}

AddressBook::~AddressBook()
{
  delete d->mManager; d->mManager = 0;
  delete d->mConfig; d->mConfig = 0;
  delete d->mErrorHandler; d->mErrorHandler = 0;
  delete d; d = 0;
}

bool AddressBook::load()
{
  kdDebug(5700) << "AddressBook::load()" << endl;

  clear();

  KRES::Manager<Resource>::ActiveIterator it;
  bool ok = true;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it )
    if ( !(*it)->load() ) {
      error( i18n("Unable to load resource '%1'").arg( (*it)->resourceName() ) );
      ok = false;
    }

  // mark all addressees as unchanged
  Addressee::List::Iterator addrIt;
  for ( addrIt = d->mAddressees.begin(); addrIt != d->mAddressees.end(); ++addrIt )
    (*addrIt).setChanged( false );

  return ok;
}

bool AddressBook::save( Ticket *ticket )
{
  kdDebug(5700) << "AddressBook::save()"<< endl;

  if ( ticket->resource() ) {
    deleteRemovedAddressees();
    return ticket->resource()->save( ticket );
  }

  return false;
}

AddressBook::Iterator AddressBook::begin()
{
  Iterator it = Iterator();
  it.d->mIt = d->mAddressees.begin();
  return it;
}

AddressBook::ConstIterator AddressBook::begin() const
{
  ConstIterator it = ConstIterator();
  it.d->mIt = d->mAddressees.begin();
  return it;
}

AddressBook::Iterator AddressBook::end()
{
  Iterator it = Iterator();
  it.d->mIt = d->mAddressees.end();
  return it;
}

AddressBook::ConstIterator AddressBook::end() const
{
  ConstIterator it = ConstIterator();
  it.d->mIt = d->mAddressees.end();
  return it;
}

void AddressBook::clear()
{
  d->mAddressees.clear();
}

Ticket *AddressBook::requestSaveTicket( Resource *resource )
{
  kdDebug(5700) << "AddressBook::requestSaveTicket()" << endl;

  if ( !resource )
    resource = standardResource();

  KRES::Manager<Resource>::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    if ( (*it) == resource ) {
      if ( (*it)->readOnly() || !(*it)->isOpen() )
        return 0;
      else
        return (*it)->requestSaveTicket();
    }
  }

  return 0;
}

void AddressBook::insertAddressee( const Addressee &a )
{
  Addressee::List::Iterator it;
  for ( it = d->mAddressees.begin(); it != d->mAddressees.end(); ++it ) {
    if ( a.uid() == (*it).uid() ) {
      bool changed = false;
      Addressee addr = a;
      if ( addr != (*it) )
        changed = true;

      (*it) = a;
      if ( (*it).resource() == 0 )
        (*it).setResource( standardResource() );

      if ( changed ) {
        (*it).setRevision( QDateTime::currentDateTime() );
        (*it).setChanged( true ); 
      }

      return;
    }
  }
  d->mAddressees.append( a );
  Addressee& addr = d->mAddressees.last();
  if ( addr.resource() == 0 )
    addr.setResource( standardResource() );

  addr.setChanged( true );
}

void AddressBook::removeAddressee( const Addressee &a )
{
  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( a.uid() == (*it).uid() ) {
      removeAddressee( it );
      return;
    }
  }
}

void AddressBook::removeAddressee( const Iterator &it )
{
  d->mRemovedAddressees.append( (*it) );
  d->mAddressees.remove( it.d->mIt );
}

AddressBook::Iterator AddressBook::find( const Addressee &a )
{
  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( a.uid() == (*it).uid() ) {
      return it;
    }
  }
  return end();
}

Addressee AddressBook::findByUid( const QString &uid )
{
  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( uid == (*it).uid() ) {
      return *it;
    }
  }
  return Addressee();
}

Addressee::List AddressBook::allAddressees()
{
  return d->mAddressees;
}

Addressee::List AddressBook::findByName( const QString &name )
{
  Addressee::List results;

  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( name == (*it).name() ) {
      results.append( *it );
    }
  }

  return results;
}

Addressee::List AddressBook::findByEmail( const QString &email )
{
  Addressee::List results;
  QStringList mailList;

  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    mailList = (*it).emails();    
    for ( QStringList::Iterator ite = mailList.begin(); ite != mailList.end(); ++ite ) {
      if ( email == (*ite) ) {
        results.append( *it );
      }
    }
  }

  return results;
}

Addressee::List AddressBook::findByCategory( const QString &category )
{
  Addressee::List results;

  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( (*it).hasCategory( category) ) {
      results.append( *it );
    }
  }

  return results;
}

void AddressBook::dump() const
{
  kdDebug(5700) << "AddressBook::dump() --- begin ---" << endl;

  ConstIterator it;
  for( it = begin(); it != end(); ++it ) {
    (*it).dump();
  }

  kdDebug(5700) << "AddressBook::dump() ---  end  ---" << endl;
}

QString AddressBook::identifier()
{
  QStringList identifier;


  KRES::Manager<Resource>::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    if ( !(*it)->identifier().isEmpty() )
      identifier.append( (*it)->identifier() );
  }

  return identifier.join( ":" );
}

Field::List AddressBook::fields( int category )
{
  if ( d->mAllFields.isEmpty() ) {
    d->mAllFields = Field::allFields();
  }
  
  if ( category == Field::All ) return d->mAllFields;
  
  Field::List result;
  Field::List::ConstIterator it;
  for( it = d->mAllFields.begin(); it != d->mAllFields.end(); ++it ) {
    if ( (*it)->category() & category ) result.append( *it );
  }
  
  return result;
}

bool AddressBook::addCustomField( const QString &label, int category,
                                  const QString &key, const QString &app )
{
  if ( d->mAllFields.isEmpty() ) {
    d->mAllFields = Field::allFields();
  }
  
  QString a = app.isNull() ? KGlobal::instance()->instanceName() : app;
  QString k = key.isNull() ? label : key;

  Field *field = Field::createCustomField( label, category, k, a );

  if ( !field ) return false;

  d->mAllFields.append( field );

  return true;
}

QDataStream &KABC::operator<<( QDataStream &s, const AddressBook &ab )
{
  if (!ab.d) return s;

  return s << ab.d->mAddressees;
}

QDataStream &KABC::operator>>( QDataStream &s, AddressBook &ab )
{
  if (!ab.d) return s;

  s >> ab.d->mAddressees;

  return s;
}

bool AddressBook::addResource( Resource *resource )
{
  if ( !resource->open() ) {
    kdDebug(5700) << "AddressBook::addResource(): can't add resource" << endl;
    return false;
  }

  resource->setAddressBook( this );

  d->mManager->add( resource );
  return true;
}

bool AddressBook::removeResource( Resource *resource )
{
  resource->close();

  if ( resource == standardResource() )
    d->mManager->setStandardResource( 0 );

  resource->setAddressBook( 0 );

  d->mManager->remove( resource );
  return true;
}

QPtrList<Resource> AddressBook::resources()
{
  QPtrList<Resource> list;

  KRES::Manager<Resource>::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it )
    list.append( *it );

  return list;
}

void AddressBook::setErrorHandler( ErrorHandler *handler )
{
  delete d->mErrorHandler;
  d->mErrorHandler = handler;
}

void AddressBook::error( const QString& msg )
{
  if ( !d->mErrorHandler ) // create default error handler
    d->mErrorHandler = new ConsoleErrorHandler;

  if ( d->mErrorHandler )
    d->mErrorHandler->error( msg );
  else
    kdError(5700) << "no error handler defined" << endl;
}

void AddressBook::deleteRemovedAddressees()
{
  Addressee::List::Iterator it;
  for ( it = d->mRemovedAddressees.begin(); it != d->mRemovedAddressees.end(); ++it ) {
    Resource *resource = (*it).resource();
    if ( resource && !resource->readOnly() && resource->isOpen() )
      resource->removeAddressee( *it );
  }

  d->mRemovedAddressees.clear();
}

void AddressBook::setStandardResource( Resource *resource )
{
  d->mManager->setStandardResource( resource );
}

Resource *AddressBook::standardResource()
{
  return d->mManager->standardResource();
}

KRES::Manager<Resource> *AddressBook::resourceManager()
{
  return d->mManager;
}

void AddressBook::cleanUp()
{
  KRES::Manager<Resource>::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    if ( !(*it)->readOnly() && (*it)->isOpen() )
      (*it)->cleanUp();
  }
}
