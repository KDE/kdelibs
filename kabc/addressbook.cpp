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

#include <qtimer.h>
#include <qfile.h>
#include <qregexp.h>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kinstance.h>

#include "simpleformat.h"
#include "vcardformat.h"
#include "resource.h"

#include "addressbook.h"
#include "addressbook.moc"

using namespace KABC;

struct AddressBook::AddressBookData
{
  Addressee::List mAddressees;
  Field::List mAllFields;
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
  d = new IteratorData;
  d->mIt = i.d->mIt;
  return *this;
}

AddressBook::Iterator::~Iterator()
{
  delete d;
}

const Addressee AddressBook::Iterator::operator*() const
{
  return *(d->mIt);
}

Addressee AddressBook::Iterator::operator*()
{
  return *(d->mIt);
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
  d = new ConstIteratorData;
  d->mIt = i.d->mIt;
  return *this;
}

AddressBook::ConstIterator::~ConstIterator()
{
  delete d;
}

const Addressee AddressBook::ConstIterator::operator*() const
{
  return *(d->mIt);
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
}

AddressBook::~AddressBook()
{
  delete d;
}

bool AddressBook::load()
{
  kdDebug(5700) << "AddressBook::load()" << endl;

  clear();

  Resource *r;
  for( r = mResources.first(); r; r = mResources.next() )
    if ( !r->load() ) return false;

  return true;
}

bool AddressBook::save( Ticket *ticket )
{
  kdDebug(5700) << "AddressBook::save()"<< endl;

  if ( ticket->resource() ) {
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

  if ( !resource ) {
    resource = mResources.first();
  }

  if ( mResources.find( resource ) < 0 ) {
    return 0;
  } else {
    return resource->requestSaveTicket();
  }
}

void AddressBook::insertAddressee( const Addressee &a )
{
  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( a.uid() == (*it).uid() ) {
      (*it) = a;
      return;
    }
  }
  d->mAddressees.append( a );
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
  return "NoIdentifier";
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
    if (!ab.d)
	return s;

    return s << ab.d->mAddressees;
}

QDataStream &KABC::operator>>( QDataStream &s, AddressBook &ab )
{
    if (!ab.d)
	return s;

    s >> ab.d->mAddressees;

    return s;
}

bool AddressBook::addResource( Resource *resource )
{
  if ( !resource->open() ) return false;
  mResources.append( resource );
  return true;
}

bool AddressBook::removeResource( Resource *resource )
{
  return mResources.remove( resource );
}
