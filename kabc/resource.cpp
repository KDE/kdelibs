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

Resource::Resource( AddressBook *ab ) 
    : mAddressBook( ab )
{
  mReadOnly = false;
  mFastResource = true;
}

Resource::~Resource()
{
}

bool Resource::open()
{
  return true;
}

void Resource::close()
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

QString Resource::identifier() const
{
  return "NoIdentifier";
}

void Resource::removeAddressee( const Addressee& )
{
  // do nothing
}

void Resource::cleanUp()
{
  // do nothing
}

void Resource::setReadOnly( bool value )
{
  mReadOnly = value;
}

bool Resource::readOnly() const
{
  return mReadOnly;
}

void Resource::setFastResource( bool value )
{
  mFastResource = value;
}

bool Resource::fastResource() const
{
  return mFastResource;
}

void Resource::setName( const QString &name )
{
  mName = name;
}

QString Resource::name() const
{
  return mName;
}

QString Resource::cryptStr( const QString &str )
{
  QString result;
  for ( uint i = 0; i < str.length(); ++i )
    result += ( str[ i ].unicode() < 0x20 ) ? str[ i ] :
        QChar( 0x1001F - str[ i ].unicode() );
                
  return result;
}
