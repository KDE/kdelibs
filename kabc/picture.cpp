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
  : mIntern( true )
{
}

Picture::Picture( const KURL &url )
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

bool Picture::operator==( const Picture &s ) const
{
  if ( mIntern != s.mIntern ) return false;
  if ( mIntern )
    if ( mData != s.mData ) return false;
  else
    if ( mUrl != s.mUrl ) return false;
  
  return true;
}

bool Picture::operator!=( const Picture &s ) const
{
  return !( s == *this );
}

void Picture::setUrl( const KURL &url )
{
  mUrl = url;
  mIntern = false;
}

void Picture::setData( const QImage &data )
{
  mData = data;
  mIntern = true;
}

bool Picture::isIntern()
{
  return mIntern;
}

KURL Picture::url()
{
  return mUrl;
}

QImage Picture::data()
{
  return mData;
}

QString Picture::asString() const
{
  if ( mIntern )
    return "intern picture";
  else
    return mUrl.url();
}

QDataStream &KABC::operator<<( QDataStream &s, const Picture &picture )
{
  return s << picture.mIntern << picture.mUrl;
}

QDataStream &KABC::operator>>( QDataStream &s, Picture &picture )
{
  s >> picture.mIntern >> picture.mUrl;
  return s;
}
