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

#include <QtCore/QObject>
#include <QHostAddress>

#include <dnssd/remoteservice.h>


namespace DNSSD
{
class DomainBrowser;
class ServiceBrowserPrivate;

/**
\class ServiceBrowser servicebrowser.h DNSSD/ServiceBrowser

Most important class for applications that want to discover specific services on network.
Suppose that you need list of web servers running. Example:
 
\code
DNSSD::ServiceBrowser* browser = new DNSSD::ServiceBrowser("_http._tcp");
connect(browser,SIGNAL(serviceAdded(RemoteService::Ptr)),this,SLOT(addService(RemoteService::Ptr)));
connect(browser,SIGNAL(serviceRemoved(RemoteService::Ptr)),this,SLOT(delService(RemoteService::Ptr)));
browser->startBrowse();
\endcode
 
In above example addService will be called for every web server already running or just appearing 
on network and delService will be called when server is stopped. Because no domain was passed 
to constructor, default domain will be searched.
 
 
@author Jakub Stachowski
@short Browsing for specific type of services or all available service types
 */
class KDNSSD_EXPORT ServiceBrowser : public QObject
{
	Q_OBJECT
public:
	/**
	Availability of DNS-SD services.
	@li Working - available
	@li Stopped - not available because mdnsd daemon is not running. This flag is currently unused
	@li Unsupported - not available because KDE was compiled without DNS-SD support	
	*/
	enum State { Working, Stopped, Unsupported };

	
	/**
	ServiceBrowser constructor.
	 
	@param type Service types to browse for (example: "_http._tcp"). 
	@param autoResolve - after disovering new service it will be resolved and then 
	reported with serviceAdded() signal. It raises network usage by resolving all services, 
	so use it only when necessary.
	@param domain Domain name. Can be left as null string to use default domain
	@param subtypes Set it to browse only for specific subtype
	 */
	explicit ServiceBrowser(const QString& type, bool autoResolve=false, const QString& domain=QString(), 
	    const QString& subtype=QString());
	
	~ServiceBrowser();

	/**
	Returns list of services 
	 */
	QList<RemoteService::Ptr> services() const;

	/**
	Starts browsing for services.
	To stop it just destroy the object.
	 */
	virtual void startBrowse();

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
	static State isAvailable();
	
	/**
	Returns true if newly discovered services are resolved before being reported with serviceAdded()
	\since 4.1
	 */
	bool isAutoResolving() const;

	/**
	 * @brief Resolved a hostname via the dnssd service into a dotted decimal IP Address
	 *
	 * The can only be used to resolve an mdns hostname
	 * @param hostname The hostname to be resolved
	 * @return A QString containing the IP address. QString() returned if failed
	 */
	static QHostAddress resolveHostName(const QString &hostname);

	/**
	 * @brief This function simply returns the name of this machine's hostname
	 * @return A QString representing the hostname. Returns QString() if failed
	 */
	static QString getLocalHostName();

Q_SIGNALS:
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

protected:
	virtual void virtual_hook(int, void*);
private:
	friend class ServiceBrowserPrivate;
	ServiceBrowserPrivate* const d;

};

}

#endif
