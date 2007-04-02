/* This file is part of the KDE project
 *
 * Copyright (C) 2004 Jakub Stachowski <qbast@go2.pl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#ifndef DNSSDSERVICETYPEBROWSER_H
#define DNSSDSERVICETYPEBROWSER_H

#include <QtCore/QObject>
#include <dnssd/remoteservice.h>

class QStringList;
namespace DNSSD
{
class ServiceTypeBrowserPrivate;

/**
@short Class used to find all service types on network
@author Jakub Stachowski
*/
class KDNSSD_EXPORT ServiceTypeBrowser : public QObject
{
	Q_OBJECT
public:
	/**
	Find all service types in the domain. 
	@param domain Browsing domain to search. WAN domains may not support service type browsing
	@param parent Parent object.
	 */
	explicit ServiceTypeBrowser(const QString& domain=QString(), QObject *parent=0);

	~ServiceTypeBrowser();

	/**
	Current list of found service types.
	 */
	QStringList serviceTypes() const;

	/**
	Starts browsing. To stop destroy this object.
	 */
	void startBrowse() ;

	/**
	Returns true when browse has already started
	 */
	bool isRunning() const;

Q_SIGNALS:
	/**
	Emitted when there is no more service of this type. This signal is not reliable - it is possible that it will not be
	emitted even after last service of this type disappeared.
	 */
	void serviceTypeRemoved(const QString&);
	/**
	New service type has been found. 
	 */
	void serviceTypeAdded(const QString&);

	/**
	Emitted when all service types has been reported. This signal can be used
	by application that just want to get list of currently available service types
	(similar to directory listing) and do not care about dynamic adding/removing later. 
	 */
	void finished();

private:
	friend class ServiceTypeBrowserPrivate;
	ServiceTypeBrowserPrivate* const d;
};

}

#endif
