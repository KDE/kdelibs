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

#include "picture.h"

using namespace KABC;

Picture::Picture()
  : mIntern( false )
{
}

Picture::Picture( const QString &url )
  : mUrl( url ), mIntern( false )
{
}

Picture::Picture( const QImage &data )
  : mData( data ), mIntern( true )
{
}

Picture::~Picture()
{
}

bool Picture::operator==( const Picture &p ) const
{
  if ( mIntern != p.mIntern ) return false;

  if ( mIntern ) {
    if ( mData != p.mData )
      return false;
  } else {
    if ( mUrl != p.mUrl )
      return false;
  }

  return true;
}

bool Picture::operator!=( const Picture &p ) const
{
  return !( p == *this );
}

void Picture::setUrl( const QString &url )
{
  mUrl = url;
  mIntern = false;
}

void Picture::setData( const QImage &data )
{
  mData = data;
  mIntern = true;
}

void Picture::setType( const QString &type )
{
  mType = type;
}

bool Picture::isIntern() const
{
  return mIntern;
}

QString Picture::url() const
{
  return mUrl;
}

QImage Picture::data() const
{
  return mData;
}

QString Picture::type() const
{
  return mType;
}

QString Picture::asString() const
{
  if ( mIntern )
    return "intern picture";
  else
    return mUrl;
}

QDataStream &KABC::operator<<( QDataStream &s, const Picture &picture )
{
  return s << picture.mIntern << picture.mUrl << picture.mType;
}

QDataStream &KABC::operator>>( QDataStream &s, Picture &picture )
{
  s >> picture.mIntern >> picture.mUrl >> picture.mType;
  return s;
}
