/*
 * Copyright (C) 2004 Frans Englich <frans.englich@telia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __KCMODULEPROXYIFACEIMPL_H__
#define __KCMODULEPROXYIFACEIMPL_H__

#include "kcmoduleproxyIface.h"

class KCModuleProxy;

/***************************************************************/
class KCModuleProxyIfaceImpl: public QObject, virtual public KCModuleProxyIface
{
	/* KDE4 Merge KCModuleProxyIfaceImpl with KCModuleProxy(MI)
	 * if it doesn't break what DCOPClient it binds to.
	 * Update: This is probably not possible, since we don't want the DCOPObject when 
	 * we're running in root mode. */

	Q_OBJECT

public:

	/* Reimplementations of DCOP members */
	KCModuleProxyIfaceImpl( const QCString& name, KCModuleProxy* const client );

	virtual void save();

	virtual void load();

	virtual void defaults();

	virtual QString applicationName();

	virtual QString quickHelp();

	virtual bool changed();
public slots:

	/**
	 * Emits the changed(bool) DCOP signal.
	 */
	void changedRelay( bool c );

	/**
	 * Simply relays KCModuleProxy's signal with the same name.
	 */
	void quickHelpRelay();
	

private:

	KCModuleProxy* p;
};
/***************************************************************/




/***************************************************************/
class KCModuleProxyRootCommunicatorImpl: public QObject, 
	virtual public KCModuleProxyRootDispatcher
{
	Q_OBJECT

public:
	KCModuleProxyRootCommunicatorImpl( const QCString& name, KCModuleProxy* const client );

	/* Reimplementations of DCOP members */
	virtual void changed( bool c );

	virtual void quickHelpChanged();

KCModuleProxy* p;
};
/***************************************************************/

#endif // __KCMODULEPROXYIFACEIMPL_H__

