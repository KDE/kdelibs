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

Resource::Resource( const KConfig *config )
    : KRES::Resource( config ), mAddressBook( 0 )
{
}

Resource::~Resource()
{
}

void Resource::writeConfig( KConfig* )
{
}

void Resource::setAddressBook( AddressBook *ab )
{
  mAddressBook = ab;
}

AddressBook *Resource::addressBook()
{
  return mAddressBook;
}

bool Resource::doOpen()
{
  return true;
}

void Resource::doClose()
{
}

Ticket *Resource::requestSaveTicket()
{
  return 0;
}

bool Resource::load()
{
  return true;
}

bool Resource::save( Ticket * )
{
  return false;
}

Ticket *Resource::createTicket( Resource *resource )
{
  return new Ticket( resource );
}

void Resource::removeAddressee( const Addressee& )
{
  // do nothing
}

void Resource::cleanUp()
{
  // do nothing
}
