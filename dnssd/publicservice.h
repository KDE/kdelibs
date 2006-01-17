/* This file is part of the KDE project
 *
 * Copyright (C) 2004, 2005 Jakub Stachowski <qbast@go2.pl>
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

#ifndef DNSSDPUBLICSERVICE_H
#define DNSSDPUBLICSERVICE_H

#include <qobject.h>
#include <dnssd/servicebase.h>
#include <QCustomEvent>

class KURL;
namespace DNSSD
{
class PublicServicePrivate;

/**
This class is most important for application that wants to announce its service on network. 
Suppose that you want to make your web server public - this is simplest way:
 
\code
DNSSD::PublicService *service = new DNSSD::PublicService("My files","_http._tcp",80);
bool isOK = service->publish();
\endcode
 
In this example publish() is synchronous - it will not return until publishing is complete.
This is usually not too long but it can freeze application's GUI for a moment.
Asynchronous publishing is better for responsiveness. Example:
 
\code
DNSSD::PublicService *service = new DNSSD::PublicService("My files","_http._tcp",80);
connect(service,SIGNAL(published(bool)),this,SLOT(wasPublished(bool)));
service->publishAsync();
\endcode
 
 
@short This class represents local service being published
@author Jakub Stachowski
 */

class KDNSSD_EXPORT PublicService : public QObject, public ServiceBase
{
	Q_OBJECT
public:
	/**
	@param name Service name. If set to QString(), computer name will be used and will be
	available via serviceName() after successful registration
	@param type Service type. Has to be in form _sometype._udp or _sometype._tcp
	@param port Port number. Set to 0 to "reserve" service name. 
	@param domain Domain name. If left as QString:null, user configuration will be used. "local."
		means local LAN
	 */
	PublicService(const QString& name=QString(),const QString& type=QString(),
		      unsigned int port=0,const QString& domain=QString());

	~PublicService();
	
	/**
	Stops publishing or abort incomplete publish request. Useful when you want to disable service 
	for some time.
	 */
	void stop();
	
	/**
	Synchrounous publish. Application will be freezed until publishing is complete.
	@return true if successfull.
	 */
	bool publish();
	
	/**
	Returns true is currently published
	 */
	bool isPublished() const;
	
	/**
	Asynchronous version of publish(). It return immediately and emits signal published(bool)
	when completed. Note that in case of early detected error (like bad service type) signal may be 
	emitted before return of this function.
	 */
	void publishAsync();

	/**
	Sets new text properties. If services is already published, it will be re-announced with new data.
	*/
	void setTextData(const QMap<QString,QString>& textData);
	
	/**
	Sets name of the service.  If service is currently published, it will be re-announced with new data.
	 */
	void setServiceName(const QString& serviceName);
	
	/**
	Sets type of service. It has to in form of _type._udp or _type._tcp.  If service is 
	currently published, it will be re-announced with new data.
	 */
	void setType(const QString& type);

	/** 
	Sets port. If service is currently published, it will be re-announced with new data.
	 */
	void setPort(unsigned short port); 
	
	/**
	Sets domain where service is published. "local." means local LAN. If service is currently 
	published, it will be re-announced with new data.
	 */
	void setDomain(const QString& domain);
	
	/**
	Translates service into URL that can be sent to another user. 
	@param host Use specified hostname. If left empty, public IP address (the one used for 
	default route) will be used.
	@since 3.5
	 */
	const KURL toInvitation(const QString& host=QString());

Q_SIGNALS:
	/**
	Emitted when publishing is complete - parameter is set to true if it was successfull. It will also
	emitted when name, port or type of already published service is changed.
	*/
	void published(bool);
private:
	PublicServicePrivate* const d;

protected:
	virtual void customEvent(QCustomEvent* event);
	virtual void virtual_hook(int, void*);
};


}

#endif
