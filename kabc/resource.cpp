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

#include <kdebug.h>
#include <klocale.h>

#include "resource.h"

using namespace KABC;

Ticket::Ticket( Resource *resource )
  : mResource( resource )
{
}

Ticket::~Ticket()
{
/* FIXME: avoid cycle deletion
  if ( mResource )
    mResource->releaseSaveTicket( this );
*/
}

Resource *Ticket::resource()
{
  return mResource;
}

struct Resource::Iterator::IteratorData
{
  Addressee::Map::Iterator mIt;
};

struct Resource::ConstIterator::ConstIteratorData
{
  Addressee::Map::ConstIterator mIt;
};

Resource::Iterator::Iterator()
{
  d = new IteratorData;
}

Resource::Iterator::Iterator( const Resource::Iterator &i )
{
  d = new IteratorData;
  d->mIt = i.d->mIt;
}

Resource::Iterator &Resource::Iterator::operator=( const Resource::Iterator &i )
{
  if ( this == &i )
    return *this;
  delete d;

  d = new IteratorData;
  d->mIt = i.d->mIt;
  return *this;
}

Resource::Iterator::~Iterator()
{
  delete d;
}

const Addressee &Resource::Iterator::operator*() const
{
  return d->mIt.data();
}

Addressee &Resource::Iterator::operator*()
{
  return d->mIt.data();
}

Resource::Iterator &Resource::Iterator::operator++()
{
  (d->mIt)++;
  return *this;
}

Resource::Iterator &Resource::Iterator::operator++( int )
{
  (d->mIt)++;
  return *this;
}

Resource::Iterator &Resource::Iterator::operator--()
{
  (d->mIt)--;
  return *this;
}

Resource::Iterator &Resource::Iterator::operator--( int )
{
  (d->mIt)--;
  return *this;
}

bool Resource::Iterator::operator==( const Iterator &it )
{
  return ( d->mIt == it.d->mIt );
}

bool Resource::Iterator::operator!=( const Iterator &it )
{
  return ( d->mIt != it.d->mIt );
}

Resource::ConstIterator::ConstIterator()
{
  d = new ConstIteratorData;
}

Resource::ConstIterator::ConstIterator( const Resource::ConstIterator &i )
{
  d = new ConstIteratorData;
  d->mIt = i.d->mIt;
}

Resource::ConstIterator::ConstIterator( const Resource::Iterator &i )
{
  d = new ConstIteratorData;
  d->mIt = i.d->mIt;
}

Resource::ConstIterator &Resource::ConstIterator::operator=( const Resource::ConstIterator &i )
{
  if ( this  == &i )
    return *this;
  delete d;

  d = new ConstIteratorData;
  d->mIt = i.d->mIt;
  return *this;
}

Resource::ConstIterator::~ConstIterator()
{
  delete d;
}

const Addressee &Resource::ConstIterator::operator*() const
{
  return *(d->mIt);
}

Resource::ConstIterator &Resource::ConstIterator::operator++()
{
  (d->mIt)++;
  return *this;
}

Resource::ConstIterator &Resource::ConstIterator::operator++( int )
{
  (d->mIt)++;
  return *this;
}

Resource::ConstIterator &Resource::ConstIterator::operator--()
{
  (d->mIt)--;
  return *this;
}

Resource::ConstIterator &Resource::ConstIterator::operator--( int )
{
  (d->mIt)--;
  return *this;
}

bool Resource::ConstIterator::operator==( const ConstIterator &it )
{
  return ( d->mIt == it.d->mIt );
}

bool Resource::ConstIterator::operator!=( const ConstIterator &it )
{
  return ( d->mIt != it.d->mIt );
}


Resource::Resource( const KConfig *config )
  : KRES::Resource( config ), mAddressBook( 0 )
{
}

Resource::~Resource()
{
}

Resource::Iterator Resource::begin()
{
  Iterator it;
  it.d->mIt = mAddrMap.begin();

  return it;
}

Resource::ConstIterator Resource::begin() const
{
  ConstIterator it;
#if QT_VERSION >= 0x030200
  it.d->mIt = mAddrMap.constBegin();
#else
  it.d->mIt = mAddrMap.begin();
#endif

  return it;
}

Resource::Iterator Resource::end()
{
  Iterator it;
  it.d->mIt = mAddrMap.end();

  return it;
}

Resource::ConstIterator Resource::end() const
{
  ConstIterator it;
#if QT_VERSION >= 0x030200
  it.d->mIt = mAddrMap.constEnd();
#else
  it.d->mIt = mAddrMap.end();
#endif

  return it;
}

void Resource::writeConfig( KConfig *config )
{
  KRES::Resource::writeConfig( config );
}

void Resource::setAddressBook( AddressBook *ab )
{
  mAddressBook = ab;
}

AddressBook *Resource::addressBook()
{
  return mAddressBook;
}

Ticket *Resource::createTicket( Resource *resource )
{
  return new Ticket( resource );
}

void Resource::insertAddressee( const Addressee &addr )
{
  mAddrMap.insert( addr.uid(), addr );
}

void Resource::removeAddressee( const Addressee &addr )
{
  mAddrMap.erase( addr.uid() );
}

Addressee Resource::findByUid( const QString &uid )
{
  Addressee::Map::iterator itr = mAddrMap.find( uid );

  if ( itr != mAddrMap.end() )
    return itr.data();

  return Addressee();
}

Addressee::List Resource::findByName( const QString &name )
{
  Addressee::List results;

  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( name == (*it).name() )
      results.append( *it );
  }

  return results;
}

Addressee::List Resource::findByEmail( const QString &email )
{
  Addressee::List results;
  QStringList mailList;
  QString lowerEmail = email.lower();

  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    mailList = (*it).emails();
    for ( QStringList::Iterator ite = mailList.begin(); ite != mailList.end(); ++ite ) {
      if ( lowerEmail == (*ite).lower() )
        results.append( *it );
    }
  }

  return results;
}

Addressee::List Resource::findByCategory( const QString &category )
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

void Resource::clear()
{
  mAddrMap.clear();
}

bool Resource::asyncLoad()
{
  bool ok = load();
  if ( !ok )
    emit loadingError( this, i18n( "Loading resource '%1' failed!" )
                       .arg( resourceName() ) );
  else
    emit loadingFinished( this );

  return ok;
}

bool Resource::asyncSave( Ticket *ticket ) {
  bool ok = save( ticket );
  if ( !ok )
    emit savingError( this, i18n( "Saving resource '%1' failed!" )
                      .arg( resourceName() ) );
  else
    emit savingFinished( this );

  return ok;
}

#include "resource.moc"
