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

#include "geo.h"

using namespace KABC;

Geo::Geo() :
  mLatitude( 0 ), mLongitude( 0 ), mValidLat( false ), mValidLong( false )
{
}

Geo::Geo( float latitude, float longitude ) :
  mLatitude( latitude ), mLongitude( longitude ),
  mValidLat( true ), mValidLong( true )
{
}

void Geo::setLatitude( float latitude )
{
  mLatitude = latitude;
  mValidLat = true;
}

float Geo::latitude() const
{
  return mLatitude;
}

void Geo::setLongitude( float longitude)
{
  mLongitude = longitude;
  mValidLong = true;
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
  return "(" + QString::number(mLatitude) + "," + QString::number(mLongitude) + ")";
}
