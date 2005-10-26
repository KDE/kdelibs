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

#ifndef DNSSDSERVICEBASE_H
#define DNSSDSERVICEBASE_H

#include <qmap.h>
#include <QString>
#include <ksharedptr.h>
#include <kdelibs_export.h>

class QDataStream;
namespace DNSSD
{
class ServiceBasePrivate;

/**
This class is used to carry information about service. It can be remote, local,
metaservice or domain. Metaservice has only type and domain - it means that
services of given type are present in given domain.
@short Describes any type of service.
@author Jakub Stachowski
 */
class KDNSSD_EXPORT ServiceBase : public KShared
{
public:
	typedef KSharedPtr<ServiceBase> Ptr;

	/**
	@param name Service name - empty for metaservices
	@param type Service type - empty for domains
	@param domain Domain name for service
	@param host   Host name for service
	@param port   Port number for service

	@todo Explain if @p domain is the FQDN, or if host + "." + domain is.
	@todo Explain when @p port is needed (does it override @p name?).
	 */
	ServiceBase(const QString& name=QString::null,const QString& type=QString::null,
		    const QString& domain=QString::null, const QString& host=QString::null,
		    unsigned short port=0);

	virtual  ~ServiceBase();

	/**
	Returns name of service. This is empty for metaservices
	 */
	const QString& serviceName() const;

	/**
	Returns type of service. It always in format _sometype._udp or _sometype._tcp and
	it is empty for domains.
	 */
	const QString& type() const;

	/**
	Returns domain that given service belongs to. It is "local." for link-local services.
	 */
	const QString& domain() const;

	/**
	Returns hostname. It is only valid for local and resolved remote services.
	 */
	const QString& hostName() const;

	/**
	Returns port number. It is only valid for local and resolved remote services.
	 */
	unsigned short port() const;

	/**
	Returns read only map of text properties.  It is only valid for local and resolved remote services.
	 */
	const QMap<QString,QString>& textData() const;

protected:
	QString m_serviceName;
	QString m_type;
	QString m_domain;
	QString m_hostName;
	unsigned short m_port;

	/**
	Map of TXT properties
	 */
	QMap<QString,QString> m_textData;
	/**
	Encode service name, type and domain into string that can be used as DNS-SD PTR label
	 */
	QString encode();
	/**
	Decode PTR label returned by DNS resolver into service name, type and domain. It also
	handles special cases - metaservices and domains.
	 */
	void decode(const QString& name);

	friend KDNSSD_EXPORT QDataStream & operator<< (QDataStream & s, const ServiceBase & a);
	friend KDNSSD_EXPORT QDataStream & operator>> (QDataStream & s, ServiceBase & a);

	virtual void virtual_hook(int, void*);
private:
	ServiceBasePrivate* d;

};

}

#endif
