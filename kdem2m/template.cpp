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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include "template.h"
#include "ifaces/template.h"
#include "factory.h"

namespace KDEM2M
{
class Template::Private
{
	public:
		Private()
		{ }
};

Template::Template( QObject * parent )
	: QObject( parent )
	, m_iface( 0 )
	, d( new Private() )
{
	slotCreateIface();
	connect( Factory::self(), SIGNAL( deleteYourObjects() ), SLOT( slotDeleteIface() ) );
	connect( Factory::self(), SIGNAL( recreateObjects() ), SLOT( slotCreateIface() ) );
}

Template::~Template()
{
	slotDeleteIface();
	delete d;
	d = 0;
}

bool Template::aboutToDeleteIface()
{
	return true;
}

void Template::slotDeleteIface()
{
	if( aboutToDeleteIface() )
	{
		delete m_iface;
		m_iface = 0;
	}
}

void Template::slotCreateIface()
{
	if( !m_iface )
		m_iface = createIface();
}

Ifaces::Template* Template::createIface()
{
	if( m_iface )
		delete m_iface;
	m_iface = Factory::self()->createTemplate( this );

	// set up parameters
	//if( m_iface )
	//{
	//}
	return m_iface;
}

Ifaces::Template* Template::iface()
{
	if( !m_iface )
		slotCreateIface();
	return m_iface;
}

} // namespace KDEM2M

#include "template.moc"

// vim: sw=4 ts=4 noet
