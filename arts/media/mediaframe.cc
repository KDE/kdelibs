    /*

    Copyright (C) 2000 Stefan Schimanski <1Stein@gmx.de>

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

#include "mediaframe.h"

using namespace Arts;

MediaDataPacket::MediaDataPacket( int capacity, Arts::GenericDataChannel *channel )
    : Arts::DataPacket<MediaFrame>( channel ), capacity( capacity )
{
    contents = new MediaFrame;
    contents->data = new char[capacity];
    size = capacity;
}

MediaDataPacket::~MediaDataPacket()
{
    delete[] contents->data;
    delete contents;
}

void MediaDataPacket::read( Arts::Buffer& stream )
{
    /* get header */
    size = stream.readLong();
    ensureCapacity( size );

    unsigned char *buffer = (unsigned char *)stream.read( sizeof(MediaFrame) );
    if(buffer) memcpy( contents, buffer, sizeof(MediaFrame) );

    /* get frame data */
    buffer = (unsigned char *)stream.read( size );
    if(buffer)
        memcpy( contents->data, buffer, size );
}

void MediaDataPacket::write( Arts::Buffer& stream )
{
    /* send header */
    stream.writeLong( size );
    stream.write( contents, sizeof(MediaFrame) );

    /* send frame data */
    stream.write( contents->data, size );
}

void MediaDataPacket::ensureCapacity(int newCapacity)
{
    if( newCapacity>size )
    {
        delete[] contents->data;
        capacity = newCapacity;
        contents->data = new char[capacity];
    }
}

DataPacket<MediaFrame> *MediaAsyncStream::allocPacket(int capacity)
{
        return new MediaDataPacket( capacity, channel );
}

GenericAsyncStream *MediaAsyncStream::createNewStream()
{
        return new MediaAsyncStream;
}
