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

#include "addressbook.h"
#include "binaryformat.h"

#define BINARY_FORMAT_VERSION 1

using namespace KABC;

bool BinaryFormat::load( AddressBook *addressBook, const QString &fileName )
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

    s >> (*addressBook);

    return true;
}

bool BinaryFormat::save( AddressBook *addressBook, const QString &fileName )
{
    kdDebug( 5700 ) << "BinaryFormat::save(): " << fileName << endl;

    QFile file( fileName );

    if ( !file.open( IO_WriteOnly ) ) {
	kdDebug( 5700 ) << "BinaryFormat::save(): " << "can't open file '" <<
	fileName << "'" << endl;
	return false;
    }

    QDataStream s( &file );

    Q_UINT32 magic, version;

    // magic code
    magic = 0x2e93e;
    version = BINARY_FORMAT_VERSION;

    s << magic << version;

    s << (*addressBook);

    file.close();

    return true;
}
