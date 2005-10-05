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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kabc/field.h>

#include "sortmode.h"

using namespace KABC;

NameSortMode::NameSortMode()
 : mNameType( FormattedName ), mAscendingOrder( true ), d( 0 )
{
  mNameType = FormattedName;
}

NameSortMode::NameSortMode( NameType type, bool ascending )
  : mNameType( type ), mAscendingOrder( ascending ), d( 0 )
{
}

bool NameSortMode::lesser( const KABC::Addressee &first, const KABC::Addressee &second ) const
{
  bool lesser = false;

  switch ( mNameType ) {
    case FormattedName:
      lesser = QString::localeAwareCompare( first.formattedName(), second.formattedName() ) < 0;
      break;
    case FamilyName:
      lesser = QString::localeAwareCompare( first.familyName(), second.familyName() ) < 0;
      break;
    case GivenName:
      lesser = QString::localeAwareCompare( first.givenName(), second.givenName() ) < 0;
      break;
    default:
      lesser = false;
      break;
  }

  if ( !mAscendingOrder )
    lesser = !lesser;

  return lesser;
}

FieldSortMode::FieldSortMode( KABC::Field *field, bool ascending )
  : mField( field ), mAscendingOrder( ascending ), d( 0 )
{
}

bool FieldSortMode::lesser( const KABC::Addressee &first, const KABC::Addressee &second ) const
{
  if ( !mField )
    return false;
  else {
    bool lesser = QString::localeAwareCompare( mField->value( first ), mField->value( second ) ) < 0;
    if ( !mAscendingOrder )
      lesser = !lesser;

    return lesser;
  }
}
