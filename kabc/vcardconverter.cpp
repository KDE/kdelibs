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

#include "vcard21parser.h"
#include "vcardformatimpl.h"

#include "vcardconverter.h"

using namespace KABC;

struct VCardConverter::VCardConverterData
{
  VCard21Parser vcard21parser;
  VCardFormatImpl vcard30parser;
};

VCardConverter::VCardConverter()
  : d( new VCardConverterData )
{
}

VCardConverter::~VCardConverter()
{
  delete d;
  d = 0;
}

bool VCardConverter::vCardToAddressee( const QString &str, Addressee &addr, Version version )
{
  if ( version == v2_1 ) {
    addr = d->vcard21parser.readFromString( str );
    return true;
  }

  if ( version == v3_0 )
    return d->vcard30parser.readFromString( str, addr );

  return false;
}

bool VCardConverter::addresseeToVCard( const Addressee &addr, QString &str, Version version )
{
  if ( version == v2_1 )
    return false;

  if ( version == v3_0 )
    return d->vcard30parser.writeToString( addr, str );

  return false;
}
