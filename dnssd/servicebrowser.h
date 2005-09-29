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
connect(browser,SIGNAL(serviceAdded(RemoteService::Ptr)),this,SLOT(addService(RemoteService::Ptr)));
connect(browser,SIGNAL(serviceRemoved(RemoteService::Ptr)),this,SLOT(delService(RemoteService::Ptr)));
browser->startBrowse();
\endcode

In above example addService will be called for every web server already running or just appearing
on network and delService will be called when server is stopped. Because no DomainBrowser was passed
to constructor, domains configured by user will be searched.


@author Jakub Stachowski
@short Browsing for specific type of services or all available service types
 */
class KDNSSD_EXPORT ServiceBrowser : public QObject
{
	Q_OBJECT
public:
	/**
	@li AutoDelete -  DomainBrowser object passes in constructor should be deleted when ServiceBrowser is deleted
	@li AutoResolve - after disovering new service it will be resolved and then
	reported with serviceAdded() signal. It raises network usage by resolving all services,
	so use it only when necessary.
	 */
	enum Flags {
	AutoDelete =1,
	AutoResolve = 2
	};

	/**
	Availability of DNS-SD services.
	@li Working - available
	@li Stopped - not available because mdnsd daemon is not running. 
	@li Unsupported - not available because KDE was compiled without DNS-SD support
	*/
	enum State { Working, Stopped, Unsupported };

	/**
	ServiceBrowser constructor.

	@param types List of service types to browse for (example: "_http._tcp").
	Can also be DNSSD::ServicesBrowser::AllServices to specify "metaquery" for all service types
	present on network
	@param domains DomainBrowser object used to specify domains to browse. You do not have to connect
	its domainAdded() signal - it will be done automatically. You can left this parameter as NULL
	for default domains.
	@param flags One or more values from #Flags

	@since 3.5
	@todo KDE4: set default values for domains and flags
	 */
	ServiceBrowser(const QStringList& types,DomainBrowser* domains,int flags);

	/**
	The same as above, but allows only one service type
	@param type Type of services to browse for
	@param domains  DomainBrowser object used to specify domains to browse. You do not have to connect its domainAdded() signal - it will be done automatically. You can left this parameter as NULL for default domains.
	@param autoResolve auto resolve, if set
	@deprecated use previous constructor instead
	 */
	ServiceBrowser(const QString& type,DomainBrowser* domains=0,bool autoResolve=false);

	/**
	Overloaded constructor used to create browser for one domain

	@param type Type of services to browse for
	@param domain Domain name. You can add more domains later using addDomain and remove them
	with removeDomain
	@param flags One or more values from #Flags. AutoDelete flag has no effect
	@since 3.5
	 */
	ServiceBrowser(const QString& type,const QString& domain, int flags);

	/**
	@deprecated user previous constructor instead
	 */
	ServiceBrowser(const QString& type,const QString& domain, bool autoResolve=false);

	~ServiceBrowser();

	/**
	Returns list of services
	 */
	const QValueList<RemoteService::Ptr>& services() const;

	/**
	Starts browsing for services.
	To stop it just destroy the object.
	 */
	virtual void startBrowse();

	/**
	Return DomainBrowser containing domains being browsed. Valid object will returned
	even if 'domains' parameters in constructor was set to NULL or single domain
	constructor was used.
	 */
	const DomainBrowser* browsedDomains() const;

	/**
	Special service type to search for all available service types. Pass it as "type"
	parameter to ServiceBrowser constructor.
	 */
	static const QString AllServices;

	/**
	Checks availability of DNS-SD services (this also covers publishing).

	If you use this function to report an error to the user, below is a suggestion
	on how to word the errors:

	\code
	switch(DNSSD::ServiceBrowser::isAvailable()) {
	  case DNSSD::ServiceBrowser::Working:
	    return "";
          case DNSSD::ServiceBrowser::Stopped:
            return i18n("<p>The Zeroconf daemon is not running. See the Service Discovery Handbook"
                        " for more information.<br/>"
	                "Other users will not see this system when browsing"
	                " the network via zeroconf, but normal access will still work.</p>");
          case DNSSD::ServiceBrowser::Unsupported:
            return i18n("<p>Zeroconf support is not available in this version of KDE."
                        " See the Service Discovery Handbook for more information.<br/>"
                        "Other users will not see this system when browsing"
                        " the network via zeroconf, but normal access will still work.</p>");
          default:
            return i18n("<p>Unknown error with Zeroconf.<br/>"
                        "Other users will not see this system when browsing"
                        " the network via zeroconf, but normal access will still work.</p>");
        }
	\endcode

	 */
	static const State isAvailable();

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
	void init(const QStringList&, DomainBrowser*, int);
	QValueList<RemoteService::Ptr>::Iterator findDuplicate(RemoteService::Ptr src);
private slots:
	void serviceResolved(bool success);
	void gotNewService(DNSSD::RemoteService::Ptr);
	void gotRemoveService(DNSSD::RemoteService::Ptr);
	void queryFinished();

};

}

#endif
