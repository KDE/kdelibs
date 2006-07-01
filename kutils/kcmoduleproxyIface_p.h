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

#ifndef __KCMODULEPROXYIFACE_P_H__
#define __KCMODULEPROXYIFACE_P_H__

#include <QtDBus/QtDBus>

/**
 * @short DCOP Interface for KCModule.
 *
 * A module (which is loaded via KCModuleProxy) does always have
 * this DCOP interface, whether it's in root mode or not.
 *
 * @internal
 * @author Frans Englich <frans.englich@telia.com>
 */
class KCModuleProxyIfaceAdaptor : public QDBusAbstractAdaptor
{
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.kde.KCModuleProxyIfaceAdaptor")

public:
	KCModuleProxyIfaceAdaptor( QObject *parent );

signals:

	/**
	 * Emitted when the state of the module changes. @p c
	 * is true when the content is changed, otherwise false.
	 *
	 * @param c true if the module is modified, false if its not.
	 * @param module a string identifying the module which was changed. This
	 * is typically "KCModuleProx-X" where X is the module's name.
	 */
	void changed( bool c );

	void quickHelpChanged();

};

#endif // __KCMODULEPROXYIFACE_P_H__
