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
#include "videopath.h"
#include "ifaces/videopath.h"
#include "factory.h"

#include "videoeffect.h"
#include "videooutputbase.h"

namespace Kdem2m
{
class VideoPath::Private
{
	public:
		Private()
		{ }

};

VideoPath::VideoPath( QObject* parent )
	: QObject( parent )
	, m_iface( 0 )
	, d( new Private() )
{
	slotCreateIface();
	connect( Factory::self(), SIGNAL( deleteYourObjects() ), SLOT( slotDeleteIface() ) );
	connect( Factory::self(), SIGNAL( recreateObjects() ), SLOT( slotCreateIface() ) );
}

VideoPath::VideoPath( Ifaces::VideoPath* iface, QObject* parent )
	: QObject( parent )
	, m_iface( iface )
	, d( new Private() )
{
	connect( Factory::self(), SIGNAL( deleteYourObjects() ), SLOT( slotDeleteIface() ) );
	connect( Factory::self(), SIGNAL( recreateObjects() ), SLOT( slotCreateIface() ) );
}

VideoPath::~VideoPath()
{
	slotDeleteIface();
	delete d;
	d = 0;
}

bool VideoPath::insertEffect( VideoEffect* newEffect, VideoEffect* insertBefore )
{
	return iface()->insertEffect( newEffect->iface(), insertBefore->iface() );
}

bool VideoPath::addOutput( VideoOutputBase* videoOutput )
{
	return iface()->addOutput( videoOutput->iface() );
}

bool VideoPath::removeOutput( VideoOutputBase* videoOutput )
{
	return iface()->removeOutput( videoOutput->iface() );
}

bool VideoPath::aboutToDeleteIface()
{
	return true;
}

void VideoPath::slotDeleteIface()
{
	if( aboutToDeleteIface() )
	{
		delete m_iface;
		m_iface = 0;
	}
}

void VideoPath::slotCreateIface()
{
	if( !m_iface )
		m_iface = createIface();
	setupIface();
}

Ifaces::VideoPath* VideoPath::createIface()
{
  	delete m_iface;
  	m_iface = Factory::self()->createVideoPath( this );
	return m_iface;
}

void VideoPath::setupIface()
{	if( !m_iface )
		return;


	// set up attributes
}

Ifaces::VideoPath* VideoPath::iface()
{
	if( !m_iface )
		slotCreateIface();
	return m_iface;
}

} //namespace Kdem2m

#include "videopath.moc"

// vim: sw=4 ts=4 tw=80 noet
