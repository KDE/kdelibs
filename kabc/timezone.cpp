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

#include <qdatastream.h>

#include "timezone.h"

using namespace KABC;

TimeZone::TimeZone() :
  mOffset( 0 ), mValid( false )
{
}

TimeZone::TimeZone( int offset ) :
  mOffset( offset ), mValid( true )
{
}

void TimeZone::setOffset( int offset )
{
  mOffset = offset;
  mValid = true;
}

int TimeZone::offset() const
{
  return mOffset;
}

bool TimeZone::isValid() const
{
  return mValid;
}

bool TimeZone::operator==( const TimeZone &t ) const
{
  if ( !t.isValid() && !isValid() ) return true;
  if ( !t.isValid() || !isValid() ) return false;
  if ( t.mOffset == mOffset ) return true;
  return false;
}

bool TimeZone::operator!=( const TimeZone &t ) const
{
  if ( !t.isValid() && !isValid() ) return false;
  if ( !t.isValid() || !isValid() ) return true;
  if ( t.mOffset != mOffset ) return true;
  return false;
}

QString TimeZone::asString() const
{
  return QString::number( mOffset );
}

QDataStream &KABC::operator<<( QDataStream &s, const TimeZone &zone )
{
    return s << zone.mOffset;
}

QDataStream &KABC::operator>>( QDataStream &s, TimeZone &zone )
{
    s >> zone.mOffset;
    zone.mValid = true;

    return s;
}
