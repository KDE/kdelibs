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
#include <qfile.h>

#include <kdebug.h>
#include <klocale.h>

#include "addressbook.h"
#include "binaryformat.h"

#define BINARY_FORMAT_VERSION 1

using namespace KABC;

bool BinaryFormat::load( AddressBook *addressBook, Resource *resource, const QString &fileName )
{
    kdDebug(5700) << "BinaryFormat::load(): " << fileName << endl;

    QFile file( fileName );

    if ( !file.open( IO_ReadOnly ) ) {
	kdDebug( 5700 ) << "BinaryFormat::load(): " << "can't open file '" <<
	fileName << "'" << endl;
	return false;
    }

    QDataStream s( &file );

    Q_UINT32 magic, version;
    
    s >> magic >> version;

    if ( magic != 0x2e93e ) {
	kdDebug( 5700 ) << "BinaryFormat::load(): file is no addressbook" << endl;
	return false;
    }

    if ( version != BINARY_FORMAT_VERSION ) {
	kdDebug( 5700 ) << "BinaryFormat::load(): wrong version" << endl;
	return false;
    }

    Q_UINT32 entries;
    s >> entries;
    for (uint i = 0; i < entries; ++i ) {
	Addressee addressee;
	s >> addressee;
	addressee.setResource( resource );
	addressBook->insertAddressee( addressee );
	addressee.setChanged( false );
    }

    return true;
}

bool BinaryFormat::save( AddressBook *addressBook, Resource *resource, const QString &fileName )
{
    kdDebug( 5700 ) << "BinaryFormat::save(): " << fileName << endl;

    QFile file( fileName );

    if ( !file.open( IO_WriteOnly ) ) {
	kdDebug( 5700 ) << "BinaryFormat::save(): " << "can't open file '" <<
	fileName << "'" << endl;
	return false;
    }

    QDataStream s( &file );

    Q_UINT32 magic, version, entries;

    entries = 0; // dummy entry, we will change it later

    // magic code
    magic = 0x2e93e;
    version = BINARY_FORMAT_VERSION;

    s << magic << version << entries;

    AddressBook::Iterator it;
    uint counter = 0;
    for ( it = addressBook->begin(); it != addressBook->end(); ++it ) {
	if ( (*it).resource() != resource && (*it).resource() != 0 )
	    continue;

	s << (*it);
	counter++;
    }

    // change the dummy entry to correct number of entries
    s.device()->at( 2 * sizeof( Q_UINT32 ) );
    s << counter;

    file.close();

    return true;
}

void BinaryFormat::removeAddressee( const Addressee& )
{
    // FIXME: implement when splitting binary file into single files
}

QString BinaryFormat::typeInfo() const
{
    return i18n( "binary" );
}
