/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#include <klocale.h>

#include "secrecy.h"

using namespace KABC;

Secrecy::Secrecy( int type )
  : mType( type )
{
}

bool Secrecy::operator==( const Secrecy &s ) const
{
  return ( mType == s.mType );
}

bool Secrecy::operator!=( const Secrecy &s ) const
{
  return !( *this == s );
}

void Secrecy::setType( int type )
{
  mType = type;
}

int Secrecy::type() const
{
  return mType;
}

Secrecy::TypeList Secrecy::typeList()
{
  TypeList list;
  list << Public;
  list << Private;
  list << Confidential;

  return list;
}

QString Secrecy::typeLabel( int type )
{
  switch ( type ) {
    case Public:
      return i18n( "Public" );
      break;
    case Private:
      return i18n( "Private" );
      break;
    case Confidential:
      return i18n( "Confidential" );
      break;
    default:
      return i18n( "Unknown type" );
      break;
  }
}

QString Secrecy::asString() const
{
  return typeLabel( mType );
}

QDataStream &KABC::operator<<( QDataStream &s, const Secrecy &secrecy )
{
    return s << secrecy.mType;
}

QDataStream &KABC::operator>>( QDataStream &s, Secrecy &secrecy )
{
    s >> secrecy.mType;

    return s;
}
