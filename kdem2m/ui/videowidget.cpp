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

#include "videowidget.h"
#include "factory.h"

#include <kdem2m/ifaces/ui/videowidget.h>

namespace Kdem2m
{
namespace Ui
{

/* class VideoWidget::Private
{
	public:
		Private()
		{ }
};*/

VideoWidget::VideoWidget( QWidget* parent )
	: QWidget( parent )
	, m_iface( 0 )
	//, d( new Private() )
{
	slotCreateIface();
	connect( Factory::self(), SIGNAL( deleteYourObjects() ), SLOT( slotDeleteIface() ) );
	connect( Factory::self(), SIGNAL( recreateObjects() ), SLOT( slotCreateIface() ) );
}

VideoWidget::VideoWidget( Ifaces::Ui::VideoWidget* iface, QWidget* parent )
	: QWidget( parent )
	, m_iface( iface )
	//, d( new Private() )
{
	connect( Factory::self(), SIGNAL( deleteYourObjects() ), SLOT( slotDeleteIface() ) );
	connect( Factory::self(), SIGNAL( recreateObjects() ), SLOT( slotCreateIface() ) );
}

VideoWidget::~VideoWidget()
{
	slotDeleteIface();
	//delete d;
	//d = 0;
}

bool VideoWidget::aboutToDeleteIface()
{
	return true;
}

void VideoWidget::slotDeleteIface()
{
	if( aboutToDeleteIface() )
	{
		delete m_iface;
		m_iface = 0;
	}
}

void VideoWidget::slotCreateIface()
{
	if( !m_iface )
		m_iface = createIface();
	setupIface();
}

Ifaces::Ui::VideoWidget* VideoWidget::createIface()
{
  	delete m_iface;
  	m_iface = Factory::self()->createVideoWidget( this );
	return m_iface;
}

void VideoWidget::setupIface()
{
}

Ifaces::Ui::VideoWidget* VideoWidget::iface()
{
	if( !m_iface )
		slotCreateIface();
	return m_iface;
}

}} //namespace Kdem2m::Ui

#include "videowidget.moc"

// vim: sw=4 ts=4 tw=80 noet
