/*
    This file is part of libkabc.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#include <kabc/field.h>

#include "sortmode.h"

using namespace KABC;

NameSortMode::NameSortMode()
 : mNameType( FormattedName ), d( 0 )
{
  mNameType = FormattedName;
}

NameSortMode::NameSortMode( NameType type )
  : mNameType( type ), d( 0 )
{
}

bool NameSortMode::lesser( const KABC::Addressee &first, const KABC::Addressee &second ) const
{
  switch ( mNameType ) {
    case FormattedName:
      return ( QString::localeAwareCompare( first.formattedName(), second.formattedName() ) < 0 );
      break;
    case FamilyName:
      return ( QString::localeAwareCompare( first.familyName(), second.familyName() ) < 0 );
      break;
    case GivenName:
      return ( QString::localeAwareCompare( first.givenName(), second.givenName() ) < 0 );
      break;
    default:
      return false;
      break;
  }
}

FieldSortMode::FieldSortMode( KABC::Field *field )
  : mField( field ), d( 0 )
{
}

bool FieldSortMode::lesser( const KABC::Addressee &first, const KABC::Addressee &second ) const
{
  if ( !mField )
    return false;
  else
    return ( QString::localeAwareCompare( mField->value( first ), mField->value( second ) ) < 0 );
}

