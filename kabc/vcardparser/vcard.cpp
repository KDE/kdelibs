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

class VCard::VCardPrivate
{
  public:
    QMap< QString, VCardLine::List > mLineMap;
};

VCard::VCard()
  : d( new VCardPrivate )
{
}

VCard::~VCard()
{
  delete d;
  d = 0;
}

void VCard::clear()
{
  d->mLineMap.clear();
}

QStringList VCard::identifiers() const
{
  return d->mLineMap.keys();
}

void VCard::addLine( const VCardLine& line )
{
  d->mLineMap[ line.identifier() ].append( line );
}

VCardLine::List VCard::lines( const QString& identifier )
{
  return d->mLineMap[ identifier.lower() ];
}

VCardLine VCard::line( const QString& identifier )
{
  return d->mLineMap[ identifier.lower() ][0];
}

void VCard::setVersion( Version version )
{
  d->mLineMap.erase( "version" );

  VCardLine line;
  line.setIdentifier( "version" );
  if ( version == v2_1 )
    line.setIdentifier( "2.1" );
  if ( version == v3_0 )
    line.setIdentifier( "3.0" );

  d->mLineMap[ "version" ].append( line );
}

VCard::Version VCard::version() const
{
  VCardLine line = d->mLineMap[ "version" ][0];
  if ( line.value() == "2.1" )
    return v2_1;
  else
    return v3_0;
}
