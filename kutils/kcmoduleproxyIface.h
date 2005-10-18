/*  This file is part of the KDE libraries
    Copyright (C) 2004 Frans Englich <frans.englich@telia.com>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef __KCMODULEPROXYIFACE_H__
#define __KCMODULEPROXYIFACE_H__

#include <dcopobject.h>

/**
 * @short DCOP Interface for KCModule.
 * 
 * A module (which is loaded via KCModuleProxy) does always have 
 * this DCOP interface, whether it's in root mode or not.
 *
 * @since 3.4
 * @internal
 * @author Frans Englich <frans.englich@telia.com>
 */
class KCModuleProxyIface : virtual public DCOPObject
{
	K_DCOP

k_dcop:

	/**
	 * Return the caption of the host application which the module 
	 * is part in. This is the application name, or similar.
	 *
	 * @returns the host's name
	 */
	virtual QString applicationName() = 0;

	/**
	 * Save settings.
	 */
	virtual void save() = 0;

	/**
	 * Load settings.
	 */
	virtual void load() = 0;

	/**
	 * Load defaults.
	 */
	virtual void defaults() = 0;

	/**
	 * Returns the module's quick help.
	 */
	virtual QString quickHelp() = 0;

	/**
	 * @returns true if the module has unsaved 
	 * data, typically.
	 */
	virtual bool changed() = 0;

k_dcop_signals:

	/**
	 * Emitted when the state of the module changes. @p c
	 * is true when the content is changed, otherwise false.
	 *
	 * @param c true if the module is modified, false if its not.
	 * @param module a string identifying the module which was changed. This 
	 * is typically "KCModuleProx-X" where X is the module's name.
	 */
	virtual void changed( bool c );

	virtual void quickHelpChanged();

};

/**
 * @short DCOP interface for communicating with the real module running in root mode.
 * 
 * @description When a KCModuleProxy tells kcmshell to load "itself" and embed into the 
 * KCModuleProxy, this DCOP interface is used to communicate to the real KCModuleProxy, since 
 * the KCModuleProxy which told kcmshell to load itself, is nothing but a shell.
 *
 * Currently is only the changed signal routed, but it's possible to proxy
 * the rest of the KCModuleProxy API, if it turns out necessary.
 *
 * @since 3.4
 * @internal
 * @author Frans Englich <frans.englich@telia.com>
 */
class KCModuleProxyRootDispatcher : virtual public DCOPObject
{
	K_DCOP

k_dcop:

	/**
	 * KCModuleProxyIface::changed() gets connected to this.
	 */
	virtual void changed( bool c ) = 0;

	virtual void quickHelpChanged() = 0;
};

#endif // __KCMODULEPROXYIFACE_H__
