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

#ifndef DNSSDQUERY_H
#define DNSSDQUERY_H

#include <qobject.h>
#include <dnssd/remoteservice.h>


namespace DNSSD
{
class QueryPrivate;

/**
This class provides way to search for specified service type in one domain. Depending on domain
name, either multicast or unicast DNS will be used.
 
@short Class that represents service query in one domain.
@author Jakub Stachowski
 */
class KDNSSD_EXPORT Query : public QObject
{
	Q_OBJECT
public:
	/**
	Creates new query. 

	@param type Type of services to browse for
	@param domain Domain name - if set to "local." multicast query will be performed,
			otherwise unicast
	 */
	Query(const QString& type, const QString& domain);

	virtual ~Query();

	/**
	Starts query. Ignored if query is already running
	 */
	virtual void startQuery();

	/**
	Returns true if query is already running
	 */
	bool isRunning() const;

	/**
	Returns true if all currently announced services has
	been reported. It does not mean that no more services can
	be found later and it is not related to isRunning()
	 */
	bool isFinished() const;

	/**
	Returns queried domain
	 */
	const QString& domain() const;

Q_SIGNALS:
	/**
	Emitted when new service has been discovered
	 */
	void serviceAdded(DNSSD::RemoteService::Ptr);

	/**
	Emitted when previously discovered service is not longer published
	 */
	void serviceRemoved(DNSSD::RemoteService::Ptr);

	/**
	Emitted when all announced services has been reported. 
	 */
	void finished();

protected:
	virtual void virtual_hook(int, void*);
	virtual void customEvent(QCustomEvent* event);
private:
	QueryPrivate* const d;
private Q_SLOTS:
	void timeout();
};

}

#endif
