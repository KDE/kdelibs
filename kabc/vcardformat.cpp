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

#include "vcardformat.h"
#include "vcardformatimpl.h"

using namespace KABC;

VCardFormat::VCardFormat()
{
  mImpl = new VCardFormatImpl;
}

VCardFormat::~VCardFormat()
{
  delete mImpl;
}

bool VCardFormat::load( Addressee &addressee, QFile *file )
{
  return mImpl->load( addressee, file );
}

bool VCardFormat::loadAll( AddressBook *addressBook, Resource *resource, QFile *file )
{
  return mImpl->loadAll( addressBook, resource, file );
}

void VCardFormat::save( const Addressee &addressee, QFile *file )
{
  return mImpl->save( addressee, file );
}

void VCardFormat::saveAll( AddressBook *addressBook, Resource *resource, QFile *file )
{
  return mImpl->saveAll( addressBook, resource, file );
}

bool VCardFormat::checkFormat( QFile *file ) const
{
  QString line;

  file->readLine( line, 1024 );
  line = line.stripWhiteSpace();
  if ( line == "BEGIN:VCARD" )
    return true;
  else
    return false;
}
