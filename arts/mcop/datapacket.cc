    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

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

#include <string.h>
#include "datapacket.h"
#include "buffer.h"

using namespace Arts;

long GenericDataPacket::_staticDataPacketCount = 0;

/**** specific DataPacket types ****/

void FloatDataPacket::read(Buffer& stream)
{
	size = stream.readLong();
	ensureCapacity(size);
	for(int i=0;i<size;i++) contents[i] = stream.readFloat();
}

void FloatDataPacket::write(Buffer& stream)
{
	stream.writeLong(size);
	for(int i=0;i<size;i++) stream.writeFloat(contents[i]);
}

void ByteDataPacket::read(Buffer& stream)
{
	size = stream.readLong();
	ensureCapacity(size);

	// we know that bytes are marshalled as bytes,
	// so we can read them as block
	unsigned char *buffer = (unsigned char *)stream.read(size);
	if(buffer)
		memcpy(contents,buffer,size);
}

void ByteDataPacket::write(Buffer& stream)
{
	// we know that bytes are marshalled as bytes,
	// so we can write them as block

	stream.writeLong(size);
	stream.write(contents,size);
}
