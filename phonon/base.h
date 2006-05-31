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

#ifndef PHONON_BASE_H
#define PHONON_BASE_H

#include "phonondefs.h"
#include <kdelibs_export.h>

class QObject;
namespace Phonon
{
	class BasePrivate;
	class BaseDestructionHandler;

	/**
	 * \internal
	 * Base class for all %Phonon frontend classes.
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class PHONONCORE_EXPORT Base
	{
		K_DECLARE_PRIVATE( Base )
		protected:
			/**
			 * \internal
			 * \param d private object
			 */
			Base( BasePrivate& d );

			virtual ~Base();

		public:
			/**
			 * \internal
			 * This class has its own destroyed signal since some cleanup calls
			 * need the pointer to the Ifaces object intact. The
			 * QObject::destroyed signals comes after the Ifaces object was
			 * deleted.
			 *
			 * As this class cannot derive from QObject a simple handler
			 * interface is used.
			 */
			void addDestructionHandler( BaseDestructionHandler* handler );
			/**
			 * \internal
			 * This class has its own destroyed signal since some cleanup calls
			 * need the pointer to the Ifaces object intact. The
			 * QObject::destroyed signals comes after the Ifaces object was
			 * deleted.
			 *
			 * As this class cannot derive from QObject a simple handler
			 * interface is used.
			 */
			void removeDestructionHandler( BaseDestructionHandler* handler );

		protected:
			/**
			 * \internal
			 * private data pointer
			 */
			BasePrivate* k_ptr;

			/**
			 * \internal
			 * Returns the backend object. If the object does not exist it tries to
			 * create it before returning.
			 *
			 * \return the Iface object, might return \c 0
			 */
			QObject* iface();
	};
} //namespace Phonon

// vim: sw=4 ts=4 tw=80 noet
#endif // PHONON_BASE_H
