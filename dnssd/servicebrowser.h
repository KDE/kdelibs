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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef DNSSDSERVICEBROWSER_H
#define DNSSDSERVICEBROWSER_H

#include <qobject.h>
#include <qdict.h>
#include <dnssd/remoteservice.h>


class QStringList;
namespace DNSSD
{
class DomainBrowser;
class ServiceBrowserPrivate;

/**
Most important class for applications that want to discover specific services on network.
Suppose that you need list of web servers running. Example:
 
\code
DNSSD::ServiceBrowser* browser = new DNSSD::ServiceBrowser("_http._tcp");
connect(browser,SIGNAL(serviceAdded(RemoteService::Ptr),this,SLOT(addService(RemoteService::Ptr)));
connect(browser,SIGNAL(serviceRemoved(RemoteService::Ptr),this,SLOT(delService(RemoteService::Ptr)));
browser->startBrowse();
\endcode
 
In above example addService will be called for every web server already running or just appearing 
on network and delService will be called when server is stopped. Because no DomainBrowser was passed 
to constructor, domains configured by user will be searched.
 
 
@author Jakub Stachowski
@short Browsing for specific type of services or all available service types
 */
class KDE_EXPORT ServiceBrowser : public QObject
{
	Q_OBJECT
public:
	/**
	ServiceBrowser constructor.
	 
	@param type  Type of services to browse for (example: "_http._tcp"). 
	Can also be DNSSD::ServicesBrowser::AllServices to specify "metaquery" for all service types 
	present on network
	@param domains  DomainBrowser object used to specify domains to browse. You do not have to connect 
	its domainAdded() signal - it will be done automatically. You can left this parameter as NULL 
	for default domains.
	@param autoResolve If set to TRUE, after disovering new service it will be resolved and then 
	reported with serviceAdded() signal. It raises network usage by resolving all services, 
	so use it only when necessary.
	 */
	ServiceBrowser(const QString& type,DomainBrowser* domains=0,bool autoResolve=false);
	
	/**
	Overloaded constructor used to create browser for one domain
	
	@param domain Domain name. You can add more domains later using addDomain and remove them
	with removeDomain
	 */
	ServiceBrowser(const QString& type,const QString& domain, bool autoResolve=false);
	
	~ServiceBrowser();

	/**
	Returns list of services 
	 */
	QValueList<RemoteService::Ptr> services() const;

	/**
	Starts browsing for services.
	To stop it just destroy the object.
	 */
	virtual void startBrowse();

	/**
	Special service type to search for all available service types. Pass it as "type"
	parameter to ServiceBrowser constructor.
	 */
	static const QString AllServices;

signals:
	/**
	Emitted when new service is discovered (or resolved
	if autoresolve is set
	 */
	void serviceAdded(DNSSD::RemoteService::Ptr);
	/**
	Emitted when service is no longer announced. RemoteService object
	is deleted from services list and destroyed immediately after this
	signal returns.
	 */
	void serviceRemoved(DNSSD::RemoteService::Ptr);

	/**
	Emitted when all services has been reported. This signal can be used
	by application that just want to get list of currently available services
	(similar to directory listing) and do not care about dynamic adding/removing
	services later. This signal can be emitted many time: for example if new host 
	has been connected to network and is announcing some services interesting to
	this ServiceBrowser, they will be reported by several serviceAdded() signals and
	whole batch will be concluded by finished(). 
	 */
	void finished();

public slots:
	/**
	Remove one domain from list of domains to browse
	 */
	void removeDomain(const QString& domain);
	
	/**
	Add new domain to browse
	 */
	void addDomain(const QString& domain);

protected:
	virtual void virtual_hook(int, void*);
private:
	ServiceBrowserPrivate *d;

	bool allFinished();
	QValueList<RemoteService::Ptr>::Iterator findDuplicate(RemoteService::Ptr src);
private slots:
	void serviceResolved(bool success);
	void gotNewService(DNSSD::RemoteService::Ptr);
	void gotRemoveService(DNSSD::RemoteService::Ptr);
	void queryFinished();
	void init(const QString&, DomainBrowser*, bool);

};

}

#endif
