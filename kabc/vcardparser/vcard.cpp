/*
    This file is part of libkabc.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include "vcard.h"

using namespace KABC;

VCard::VCard()
  : mLineMap( 0 )
{
}

VCard::VCard( const VCard& vcard )
  : mLineMap( 0 )
{
  if ( vcard.mLineMap ) {
    if ( !mLineMap )
      mLineMap = new QMap<QString, QValueList<VCardLine> >;

    *mLineMap = *(vcard.mLineMap);
  } else {
    delete mLineMap;
    mLineMap = 0;
  }
}

VCard::~VCard()
{
  delete mLineMap;
  mLineMap = 0;
}

VCard& VCard::operator=( const VCard& vcard )
{
  if ( &vcard == this )
    return *this;

  if ( vcard.mLineMap ) {
    if ( !mLineMap )
      mLineMap = new QMap<QString, QValueList<VCardLine> >;

    *mLineMap = *(vcard.mLineMap);
  } else {
    delete mLineMap;
    mLineMap = 0;
  }

  return *this;
}

void VCard::clear()
{
  if ( mLineMap )
    mLineMap->clear();
}

QStringList VCard::identifiers() const
{
  if ( !mLineMap )
    return QStringList();
  else
    return mLineMap->keys();
}

void VCard::addLine( const VCardLine& line )
{
  if ( !mLineMap )
    mLineMap = new QMap<QString, QValueList<VCardLine> >;

  (*mLineMap)[ line.identifier() ].append( line );
}

VCardLine::List VCard::lines( const QString& identifier )
{
  if ( !mLineMap )
    return VCardLine::List();
  else
    return (*mLineMap)[ identifier ];
}

VCardLine VCard::line( const QString& identifier )
{
  if ( !mLineMap )
    return VCardLine();
  else
    return (*mLineMap)[ identifier ][ 0 ];
}

void VCard::setVersion( Version version )
{
  if ( !mLineMap )
    mLineMap = new QMap<QString, QValueList<VCardLine> >;
  else
    mLineMap->erase( "VERSION" );

  VCardLine line;
  line.setIdentifier( "VERSION" );
  if ( version == v2_1 )
    line.setIdentifier( "2.1" );
  if ( version == v3_0 )
    line.setIdentifier( "3.0" );

  (*mLineMap)[ "VERSION" ].append( line );
}

VCard::Version VCard::version() const
{
  if ( !mLineMap )
    return v3_0;

  VCardLine line = (*mLineMap)[ "VERSION" ][ 0 ];
  if ( line.value() == "2.1" )
    return v2_1;
  else
    return v3_0;
}
