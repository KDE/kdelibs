/*  This file is part of the KDE project
    Copyright (C) 2005 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/
#include "bytestream.h"
#include "ifaces/bytestream.h"
#include "factory.h"

namespace KDEM2M
{
class ByteStream::Private
{
};

ByteStream::ByteStream( QObject* parent )
	: MediaProducer( createIface( false ), parent )
	, d( new Private() )
{
	setupIface();
}

ByteStream::ByteStream( Ifaces::MediaProducer* iface, QObject* parent )
	: MediaProducer( iface, parent )
	, m_iface( dynamic_cast<Ifaces::ByteStream*>( iface ) )
	, d( new Private() )
{
}

ByteStream::~ByteStream()
{
	slotDeleteIface();
	delete d;
	d = 0;
}

bool ByteStream::aboutToDeleteIface()
{
	return MediaProducer::aboutToDeleteIface();
}

Ifaces::MediaProducer* ByteStream::createIface( bool initialized )
{
	if( initialized )
		delete m_iface;
  	m_iface = Factory::self()->createByteStream( 0 );
	return m_iface;
}

void ByteStream::setupIface()
{
	MediaProducer::setupIface();

	if( !m_iface )
		return;
}

Ifaces::ByteStream* ByteStream::iface()
{
	if( !m_iface )
		slotCreateIface();
	return m_iface;
}

} //namespace KDEM2M

#include "bytestream.moc"

// vim: sw=4 ts=4 tw=80 noet
