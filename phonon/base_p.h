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

#ifndef PHONON_BASE_P_H
#define PHONON_BASE_P_H

#include "base.h"
#include "ifaces/base.h"
#include <QList>
#include "basedestructionhandler.h"
#include "factory.h"

namespace Phonon
{
	namespace Ifaces
	{
		class Base;
	}

class BasePrivate
{
	K_DECLARE_PUBLIC( Base )
	friend class Phonon::Factory;
	protected:
		BasePrivate()
		{
			Factory::self()->registerFrontendObject( this );
		}

		virtual ~BasePrivate()
		{
			Factory::self()->deregisterFrontendObject( this );
			delete iface_ptr;
		}

		/**
		 * \internal
		 * This method cleanly deletes the Iface object. It is called on
		 * destruction and before a backend change.
		 */
		void deleteIface()
		{
			if( iface_ptr && aboutToDeleteIface() )
			{
				delete iface_ptr;
				setIface( 0 );
			}
		}

		virtual bool aboutToDeleteIface() = 0;

		/**
		 * \internal
		 * Creates the Iface object belonging to this class. For most cases the
		 * implementation is
		 * \code
		 * Q_Q( ClassName );
		 * m_iface = Factory::self()->createClassName( this );
		 * return m_iface;
		 * \endcode
		 *
		 * This function should not be called except from slotCreateIface.
		 *
		 * \see slotCreateIface
		 */
		virtual void createIface() = 0;

		virtual void setIface( void* p )
		{
			iface_ptr = reinterpret_cast<Ifaces::Base*>( p );
		}

		Base* q_ptr;

	private:
		Ifaces::Base* iface_ptr;
		QList<BaseDestructionHandler*> handlers;
};
} //namespace Phonon

#endif // PHONON_BASE_P_H
// vim: sw=4 ts=4 tw=80
