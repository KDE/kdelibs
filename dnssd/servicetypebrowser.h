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
 * @class ServiceBrowser servicebrowser.h DNSSD/ServiceBrowser
 * @short Browses the service types being published on a domain
 *
 * This class is mostly useful for generic utilities for
 * browsing all the published services on a local network.
 * Applications that wish to find out about available services
 * of a particular type (such as web servers) should use
 * ServiceBrowser.
 *
 * ServiceTypeBrowser provides a list of all the service types
 * published by at least one service on a given domain.
 *
 * @author Jakub Stachowski
 */
class KDNSSD_EXPORT ServiceTypeBrowser : public QObject
{
	Q_OBJECT

public:
	/**
	 * Create a ServiceTypeBrowser for a domain
	 *
	 * The link-local domain (the LAN subnet for this computer) will
	 * be used if no @p domain is given.  DomainBrowser can be used
	 * to get a list of browsing domains.
	 *
	 * Note that WAN domains may not support service type browsing.
	 *
	 * @param domain a browsing domain to search
	 * @param parent the parent object (see QObject documentation)
	 *
	 * @see startBrowse() and ServiceBrowser::isAvailable()
	 */
	explicit ServiceTypeBrowser(const QString& domain = QString(),
	                            QObject* parent = 0);

	~ServiceTypeBrowser();

	/**
	 * All the service types currently being published
	 *
	 * @return a list of service types, in the form _type._tcp or _type._udp
	 */
	QStringList serviceTypes() const;

	/**
	 * Starts browsing
	 *
	 * Only the first call to this function will have any effect.
	 *
	 * Browsing stops when the ServiceBrowser object is destroyed.
	 *
	 * @warning The serviceTypeAdded() signal may be emitted before this
	 *          function returns.
	 *
	 * @see serviceTypeAdded(), serviceTypeRemoved() and finished()
	 */
	void startBrowse();

	/**
	 * @deprecated
	 * This method is unnecessary, since it is safe to call startBrowse()
	 * multiple times.
	 */
	KDE_DEPRECATED bool isRunning() const;

Q_SIGNALS:
	/**
	 * Emitted when there are no more services of this type
	 *
	 * @warning
	 * This signal is not reliable: it is possible that it will not be
	 * emitted even after last service of this type disappeared
	 *
	 * @param type the service type
	 *
	 * @see serviceTypeAdded() and finished()
	 */
	void serviceTypeRemoved(const QString& type);

	/**
	 * A new type of service has been found
	 *
	 * @param type the service type
	 *
	 * @see serviceTypeAdded() and finished()
	 */
	void serviceTypeAdded(const QString& type);

	/**
	 * Emitted when the list of published service types has settled
	 *
	 * This signal is emitted once after startBrowse() is called
	 * when the types of all the services that are
	 * currently published have been reported (even if no services
	 * are available or the DNS-SD service is not available).
	 * It is emitted again when a new batch of service types become
	 * available or disappear.
	 *
	 * For example, if a new host is connected to network and
	 * announces services of several new types,
	 * they will be reported by several serviceTypeAdded() signals
	 * and the whole batch will be concluded by finished().
	 *
	 * This signal can be used by applications that just want to
	 * get a list of the currently available service types
	 * (similar to a directory listing) and do not care about
	 * adding or removing service types that appear or disappear later.
	 *
	 * @see serviceTypeAdded() and serviceTypeRemoved()
	 */
	void finished();

private:
	friend class ServiceTypeBrowserPrivate;
	ServiceTypeBrowserPrivate* const d;
};

}

#endif
