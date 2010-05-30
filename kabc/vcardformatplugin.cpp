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

#include <qfile.h>

#include "address.h"
#include "addressee.h"
#include "vcardconverter.h"

#include "vcardformatplugin.h"

using namespace KABC;

VCardFormatPlugin::VCardFormatPlugin()
{
}

VCardFormatPlugin::~VCardFormatPlugin()
{
}

bool VCardFormatPlugin::load( Addressee &addressee, QFile *file )
{
  const QByteArray rawData = file->readAll();
  const QCString data( rawData.data(), rawData.size() );

  VCardConverter converter;
  Addressee::List l = converter.parseVCardsRaw( data );

  if ( ! l.first().isEmpty() ) {
    addressee = l.first();
    return true;
  }

  return false;
}

bool VCardFormatPlugin::loadAll( AddressBook*, Resource *resource, QFile *file )
{
  const QByteArray rawData = file->readAll();
  const QCString data( rawData.data(), rawData.size() );

  VCardConverter converter;
  Addressee::List l = converter.parseVCardsRaw( data );

  Addressee::List::iterator itr;
  for ( itr = l.begin(); itr != l.end(); ++itr) {
    Addressee addressee = *itr;
    addressee.setResource( resource );
    addressee.setChanged( false );
    resource->insertAddressee( addressee );
  }

  return true;
}

void VCardFormatPlugin::save( const Addressee &addressee, QFile *file )
{
  VCardConverter converter ;
  Addressee::List vcardlist;


  vcardlist.append( addressee );

  const QCString data = converter.createVCardsRaw( vcardlist );
  file->writeBlock( data, data.length() );
}

void VCardFormatPlugin::saveAll( AddressBook*, Resource *resource, QFile *file )
{
  VCardConverter converter;
  Addressee::List vcardlist;

  Resource::Iterator it;
  for ( it = resource->begin(); it != resource->end(); ++it ) {
    (*it).setChanged( false );
    vcardlist.append( *it );
  }

  const QCString data = converter.createVCardsRaw( vcardlist );
  file->writeBlock( data, data.length() );
}

bool VCardFormatPlugin::checkFormat( QFile *file ) const
{
  QString line;

  file->readLine( line, 1024 );
  line = line.stripWhiteSpace();
  if ( line == "BEGIN:VCARD" )
    return true;
  else
    return false;
}
