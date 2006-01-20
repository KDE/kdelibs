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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qdatastream.h>

#include "geo.h"

using namespace KABC;

Geo::Geo()
  : mLatitude( 91 ), mLongitude( 181 ), mValidLat( false ), mValidLong( false )
{
}

Geo::Geo( float latitude, float longitude )
{
  setLatitude( latitude );
  setLongitude( longitude );
}

void Geo::setLatitude( float latitude )
{
  if ( latitude >= -90 && latitude <= 90 ) {
    mLatitude = latitude;
    mValidLat = true;
  } else {
    mLatitude = 91;
    mValidLat = false;
  }
}

float Geo::latitude() const
{
  return mLatitude;
}

void Geo::setLongitude( float longitude)
{
  if ( longitude >= -180 && longitude <= 180 ) {
    mLongitude = longitude;
    mValidLong = true;
  } else {
    mLongitude = 181;
    mValidLong = false;
  }
}

float Geo::longitude() const
{
  return mLongitude;
}

bool Geo::isValid() const
{
  return mValidLat && mValidLong;
}

bool Geo::operator==( const Geo &g ) const
{
  if ( !g.isValid() && !isValid() ) return true;
  if ( !g.isValid() || !isValid() ) return false;
  if ( g.mLatitude == mLatitude && g.mLongitude == mLongitude ) return true;
  return false;
}

bool Geo::operator!=( const Geo &g ) const
{
  if ( !g.isValid() && !isValid() ) return false;
  if ( !g.isValid() || !isValid() ) return true;
  if ( g.mLatitude == mLatitude && g.mLongitude == mLongitude ) return false;
  return true;
}

QString Geo::asString() const
{
  if ( !isValid() )
    return QString();
  else
    return "(" + QString::number(mLatitude) + "," + QString::number(mLongitude) + ")";
}

QDataStream &KABC::operator<<( QDataStream &s, const Geo &geo )
{
    return s << (float)geo.mLatitude << (float)geo.mLongitude;
}

QDataStream &KABC::operator>>( QDataStream &s, Geo &geo )
{
    s >> geo.mLatitude >> geo.mLongitude;

    geo.mValidLat = true;
    geo.mValidLong = true;

    return s;
}
