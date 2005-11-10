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
#include "videooutputbase.h"
#include "ifaces/videooutputbase.h"
#include "factory.h"

namespace Kdem2m
{
class VideoOutputBase::Private
{
	public:
		Private()
		{ }

		QString name;
};

VideoOutputBase::VideoOutputBase( QObject* parent )
	: QObject( parent )
	, m_iface( 0 )
	, d( new Private() )
{
	slotCreateIface();
	connect( Factory::self(), SIGNAL( deleteYourObjects() ), SLOT( slotDeleteIface() ) );
	connect( Factory::self(), SIGNAL( recreateObjects() ), SLOT( slotCreateIface() ) );
}

VideoOutputBase::VideoOutputBase( Ifaces::VideoOutputBase* iface, QObject* parent )
	: QObject( parent )
	, m_iface( iface )
	, d( new Private() )
{
	connect( Factory::self(), SIGNAL( deleteYourObjects() ), SLOT( slotDeleteIface() ) );
	connect( Factory::self(), SIGNAL( recreateObjects() ), SLOT( slotCreateIface() ) );
}

VideoOutputBase::~VideoOutputBase()
{
	slotDeleteIface();
	delete d;
	d = 0;
}

QString VideoOutputBase::name() const
{
	return m_iface ? m_iface->name() : d->name;
}

void VideoOutputBase::setName( const QString& newName )
{
	if( m_iface )
		d->name = m_iface->setName( newName );
	else
		d->name = newName;
}

bool VideoOutputBase::aboutToDeleteIface()
{
	if( m_iface )
	{
		d->name = m_iface->name();
	}
	return true;
}

void VideoOutputBase::slotDeleteIface()
{
	if( aboutToDeleteIface() )
	{
		delete m_iface;
		m_iface = 0;
	}
}

void VideoOutputBase::slotCreateIface()
{
	if( !m_iface )
		m_iface = createIface();
	setupIface();
}

Ifaces::VideoOutputBase* VideoOutputBase::createIface()
{
  	delete m_iface;
  	m_iface = Factory::self()->createVideoOutputBase( this );
	return m_iface;
}

void VideoOutputBase::setupIface()
{
	if( !m_iface )
		return;

	// set up attributes
	m_iface->setName( d->name );
}

Ifaces::VideoOutputBase* VideoOutputBase::iface()
{
	if( !m_iface )
		slotCreateIface();
	return m_iface;
}

} //namespace Kdem2m

#include "videooutputbase.moc"

// vim: sw=4 ts=4 tw=80 noet
