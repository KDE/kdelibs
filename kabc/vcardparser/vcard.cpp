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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "vcard.h"

using namespace KABC;

VCard::VCard()
{
}

VCard::VCard( const VCard& vcard )
{
  mLineMap = vcard.mLineMap;
}

VCard::~VCard()
{
}

VCard& VCard::operator=( const VCard& vcard )
{
  if ( &vcard == this )
    return *this;

  mLineMap = vcard.mLineMap;

  return *this;
}

void VCard::clear()
{
  mLineMap.clear();
}

QStringList VCard::identifiers() const
{
  return mLineMap.keys();
}

void VCard::addLine( const VCardLine& line )
{
  mLineMap[ line.identifier() ].append( line );
}

VCardLine::List VCard::lines( const QString& identifier ) const
{
  LineMap::ConstIterator it = mLineMap.find( identifier );
  if ( it == mLineMap.end() )
    return VCardLine::List();

  return *it;
}

VCardLine VCard::line( const QString& identifier ) const
{
  LineMap::ConstIterator it = mLineMap.find( identifier );
  if ( it == mLineMap.end() )
    return VCardLine();

  if ( (*it).isEmpty() )
    return VCardLine();
  else
    return (*it).first();
}

void VCard::setVersion( Version version )
{
  mLineMap.erase( "VERSION" );

  VCardLine line;
  line.setIdentifier( "VERSION" );
  if ( version == v2_1 )
    line.setIdentifier( "2.1" );
  else if ( version == v3_0 )
    line.setIdentifier( "3.0" );

  mLineMap[ "VERSION" ].append( line );
}

VCard::Version VCard::version() const
{
  LineMap::ConstIterator versionEntry = mLineMap.find( "VERSION" );
  if ( versionEntry == mLineMap.end() )
    return v3_0;

  VCardLine line = ( *versionEntry )[ 0 ];
  if ( line.value() == "2.1" )
    return v2_1;
  else
    return v3_0;
}
