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
  Addressee::List::Iterator mIt;
};

struct Resource::ConstIterator::ConstIteratorData
{
  Addressee::List::ConstIterator mIt;
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
  return *(d->mIt);
}

Addressee &Resource::Iterator::operator*()
{
  return *(d->mIt);
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
  it.d->mIt = mAddressees.begin();

  return it;
}

Resource::ConstIterator Resource::begin() const
{
  ConstIterator it;
  it.d->mIt = mAddressees.constBegin();

  return it;
}

Resource::Iterator Resource::end()
{
  Iterator it;
  it.d->mIt = mAddressees.end();

  return it;
}

Resource::ConstIterator Resource::end() const
{
  ConstIterator it;
  it.d->mIt = mAddressees.constEnd();

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
  mAddressees.append( addr );
}

void Resource::removeAddressee( const Addressee &addr )
{
  mAddressees.remove( addr );
}

void Resource::cleanUp()
{
  // do nothing
}

void Resource::clear()
{
  mAddressees.clear();
}

#include "resource.moc"
