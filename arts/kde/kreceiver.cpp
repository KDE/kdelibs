    /*

    Copyright (C) 2001 Matthias Kretz
                       kretz@kde.org

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

#include "kreceiver.h"

#include <qstring.h>

using namespace std;
using namespace Arts;

KReceiver::KReceiver( SimpleSoundServer server, int rate, int bits, int channels, QString title )
	: _samplingRate( rate ), _bits( bits ), _channels( channels ),
	m_server( server ), m_attached( false ), m_blocking( false ),
	m_pos( 0 )
{
	m_bswrapper = ByteSoundReceiver::_from_base( this );
	_title = title.local8Bit();
}

KReceiver::~KReceiver()
{
	end();
}

void KReceiver::setBlockingIO( bool blocking )
{
	m_blocking = blocking;
}

bool KReceiver::blockingIO() const 
{
	return m_blocking;
}

int KReceiver::read( char * buffer, int size )
{
	int remaining = size;
	while( remaining )
	{
		if( m_blocking )
			while( inqueue.empty() )
				Dispatcher::the()->ioManager()->processOneEvent( true );
		else
		{
			if( inqueue.empty() )
				Dispatcher::the()->ioManager()->processOneEvent( false );
			if( inqueue.empty() )
				return size - remaining;
		}
		DataPacket<mcopbyte> * packet = inqueue.front();
		int tocopy = min( remaining, packet->size - m_pos );
		memcpy( buffer, &packet->contents[m_pos], tocopy );
		m_pos += tocopy;
		buffer += tocopy;
		remaining -= tocopy;
		if( m_pos == packet->size )
		{
			packet->processed();
			inqueue.pop();
			m_pos = 0;
		}
	}
	return size;
}

void KReceiver::pause()
{
	if( m_attached )
	{
		m_server.detachRecorder( m_bswrapper );
		m_attached = false;
	}
}

void KReceiver::end()
{
	pause();
	while( ! inqueue.empty() )
	{
		inqueue.front()->processed();
		inqueue.pop();
	}
}

void KReceiver::process_indata( DataPacket<mcopbyte> * inpacket )
{
	inqueue.push( inpacket );
}

void KReceiver::attach()
{
	if( ! m_attached )
	{
		m_server.attachRecorder( m_bswrapper );
		start(); //why not put this into SimpleSoundServer?
		Dispatcher::the()->ioManager()->processOneEvent( false );
		m_attached = true;
	}
}

