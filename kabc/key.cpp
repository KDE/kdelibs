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

#include <kapplication.h>
#include <klocale.h>

#include "key.h"

using namespace KABC;

Key::Key( const QString &text, int type )
  : mTextData( text ), mIsBinary( false ), mType( type )
{
  mId = KApplication::randomString(8);
}

Key::Key( const QCString &binary, int type )
  : mBinaryData( binary ), mIsBinary( true ), mType( type )
{
  mId = KApplication::randomString(8);
}

Key::~Key()
{
}

bool Key::operator==( const Key &k ) const
{
  if ( mIsBinary != k.mIsBinary ) return false;
  if ( mIsBinary )
    if ( mBinaryData != k.mBinaryData ) return false;
  else
    if ( mTextData != k.mTextData ) return false;
  if ( mType != k.mType ) return false;
  if ( mCustomTypeString != k.mCustomTypeString ) return false;
  
  return true;
}

bool Key::operator!=( const Key &k ) const
{
  return !( k == *this );
}

void Key::setId( const QString &id )
{
  mId = id;
}

QString Key::id() const
{
  return mId;
}

void Key::setBinaryData( const QCString &binary )
{
  mBinaryData = binary;
  mIsBinary = true;
}

QCString Key::binaryData() const
{
  return mBinaryData;
}

void Key::setTextData( const QString &text )
{
  mTextData = text;
  mIsBinary = false;
}

QString Key::textData() const
{
  return mTextData;
}

bool Key::isBinary() const
{
  return mIsBinary;
}

void Key::setType( int type )
{
  mType = type;
}

void Key::setCustomTypeString( const QString &custom )
{
  mCustomTypeString = custom;
}

int Key::type() const
{
  return mType;
}

QString Key::customTypeString() const
{
  return mCustomTypeString;
}

Key::TypeList Key::typeList()
{
  TypeList list;
  list << X509;
  list << PGP;
  list << Custom;
  
  return list;
}
  
QString Key::typeLabel( int type )
{
  switch ( type ) {
    case X509:
      return i18n( "X509" );
      break;
    case PGP:
      return i18n( "PGP" );
      break;
    case Custom:
      return i18n( "Custom" );
      break;
    default:
      return i18n( "Unknown type" );
      break;
  }
}

QDataStream &KABC::operator<<( QDataStream &s, const Key &key )
{
    return s << key.mId << key.mIsBinary << key.mTextData << key.mBinaryData <<
             key.mCustomTypeString << key.mType;
}

QDataStream &KABC::operator>>( QDataStream &s, Key &key )
{
    s >> key.mId >> key.mIsBinary >> key.mTextData >> key.mBinaryData >>
    key.mCustomTypeString >> key.mType;

    return s;
}
