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

bool BinaryFormat::load( AddressBook *addressBook, Resource *resource, QFile *file )
{
  kdDebug(5700) << "BinaryFormat::load()" << endl;

  QDataStream s( file );

  Q_UINT32 magic, version;
    
  s >> magic >> version;

  if ( magic != 0x2e93e ) {
    addressBook->error( i18n("File '%1' has no binary format.").arg( file->name() ) );
    return false;
  }

  if ( version != BINARY_FORMAT_VERSION ) {
    addressBook->error( i18n("File '%1' is the wrong version.").arg( file->name() ) );
    return false;
  }

  Addressee addressee;
  s >> addressee;
  addressee.setResource( resource );
  addressBook->insertAddressee( addressee );

  return true;
}

bool BinaryFormat::save( const Addressee &addressee, QFile *file )
{
  kdDebug(5700) << "BinaryFormat::save()" << endl;

  QDataStream s( file );

  Q_UINT32 magic, version;

  // magic code
  magic = 0x2e93e;
  version = BINARY_FORMAT_VERSION;

  s << magic << version << addressee;

  return true;
}

bool BinaryFormat::checkFormat( QFile *file ) const
{
  kdDebug(5700) << "BinaryFormat::checkFormat()" << endl;

  QDataStream s( file );

  Q_UINT32 magic;
    
  s >> magic;

  if ( magic != 0x2e93e )
    return false;
  else
    return true;
}
