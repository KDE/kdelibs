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

#include <qdatastream.h>

#include <kdebug.h>
#include <klocale.h>

#include "addressbook.h"
#include "addressee.h"

#include "binaryformat.h"

#define BINARY_FORMAT_VERSION 1

using namespace KABC;

extern "C"
{
  Format *format() {
    return new BinaryFormat;
  }
}

bool BinaryFormat::load( Addressee &addressee, QFile *file )
{
  kdDebug(5700) << "BinaryFormat::load()" << endl;
  QDataStream stream( file );

  if ( !checkHeader( stream ) )
    return false;

  loadAddressee( addressee, stream );

  return true;
}

bool BinaryFormat::loadAll( AddressBook *addressBook, Resource *resource, QFile *file )
{
  kdDebug(5700) << "BinaryFormat::loadAll()" << endl;

  QDataStream stream( file );

  if ( !checkHeader( stream ) )
    return false;

  Q_UINT32 entries;

  stream >> entries;

  for ( uint i = 0; i < entries; ++i ) {
    Addressee addressee;
    loadAddressee( addressee, stream );
    addressee.setResource( resource );
    addressBook->insertAddressee( addressee );
  }

  return true;
}

void BinaryFormat::save( const Addressee &addressee, QFile *file )
{
  kdDebug(5700) << "BinaryFormat::save()" << endl;

  QDataStream stream( file );

  writeHeader( stream );

  Q_UINT32 entries = 1;
  stream << entries;
  saveAddressee( addressee, stream );
}

void BinaryFormat::saveAll( AddressBook *ab, Resource *resource, QFile *file )
{
  kdDebug(5700) << "BinaryFormat::saveAll()" << endl;

  Q_UINT32 counter = 0;
  QDataStream stream( file );

  writeHeader( stream );
  // set dummy number of entries
  stream << counter;

  AddressBook::Iterator it;
  for ( it = ab->begin(); it != ab->end(); ++it ) {
    if ( (*it).resource() == resource ) {
      saveAddressee( (*it), stream );
      counter++;
      (*it).setChanged( false );
    }
  }

  // set real number of entries
  stream.device()->at( 2 * sizeof( Q_UINT32 ) );
  stream << counter;
}

bool BinaryFormat::checkFormat( QFile *file ) const
{
  kdDebug(5700) << "BinaryFormat::checkFormat()" << endl;

  QDataStream stream( file );

  return checkHeader( stream );
}

bool BinaryFormat::checkHeader( QDataStream &stream ) const
{
  Q_UINT32 magic, version;
    
  stream >> magic >> version;

  QFile *file = dynamic_cast<QFile*>( stream.device() );

  kdDebug() << "magic=" << magic << endl;

  if ( magic != 0x2e93e ) {
    kdError() << i18n("File '%1' is not binary format.").arg( file->name() ) << endl;
    return false;
  }

  if ( version != BINARY_FORMAT_VERSION ) {
    kdError() << i18n("File '%1' is the wrong version.").arg( file->name() ) << endl;
    return false;
  }

  return true;
}

void BinaryFormat::writeHeader( QDataStream &stream )
{
  Q_UINT32 magic, version;
    
  magic = 0x2e93e;
  version = BINARY_FORMAT_VERSION;

  stream << magic << version;
}

void BinaryFormat::loadAddressee( Addressee &addressee, QDataStream &stream )
{
  stream >> addressee;
}

void BinaryFormat::saveAddressee( const Addressee &addressee, QDataStream &stream )
{
  stream << addressee;
}
