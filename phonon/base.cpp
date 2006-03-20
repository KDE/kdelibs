/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#include "base.h"
#include "base_p.h"
#include "ifaces/base.h"
#include "factory.h"
#include "basedestructionhandler.h"

namespace Phonon
{
	Base::Base( BasePrivate& d )
		: k_ptr( &d )
	{
		k_ptr->q_ptr = this;
		k_ptr->setIface( 0 );
		// cannot call k_ptr->createIface(); from here as that calls setupIface
		// on the classes that inherit Base - and they're not constructed at
		// this point
	}

	Base::~Base()
	{
		K_D( Base );
		foreach( BaseDestructionHandler* handler, d->handlers )
			handler->phononObjectDestroyed( this );
		delete k_ptr;
		k_ptr = 0;
	}

	void Base::addDestructionHandler( BaseDestructionHandler* handler )
	{
		K_D( Base );
		d->handlers.append( handler );
	}

	void Base::removeDestructionHandler( BaseDestructionHandler* handler )
	{
		K_D( Base );
		d->handlers.removeAll( handler );
	}
} //namespace Phonon
// vim: sw=4 ts=4 noet
