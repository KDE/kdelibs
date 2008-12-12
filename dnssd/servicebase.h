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

#ifndef DNSSDSERVICEBASE_H
#define DNSSDSERVICEBASE_H

#include <QtCore/QMap>
#include <QtCore/QString>
#include <ksharedptr.h>
#include <dnssd/dnssd_export.h>

namespace DNSSD
{
class ServiceBasePrivate;

/**
\class ServiceBase servicebase.h DNSSD/ServiceBase

This class is used to carry information about service. It can be remote, local,
metaservice or domain. Metaservice has only type and domain - it means that
services of given type are present in given domain.
@short Describes any type of service.
@author Jakub Stachowski
 */
class KDNSSD_EXPORT ServiceBase : public KShared //krazy:exclude=dpointer (protected)
{
public:
	typedef KSharedPtr<ServiceBase> Ptr;

	/**
	@param name Service name
	@param type Service type
	@param domain Domain name for service
	@param host   Host name for service
	@param port   Port number for service

	@todo Explain if @p domain is the FQDN, or if host + "." + domain is.
	@todo Explain when @p port is needed (does it override @p name?).
	 */
	explicit ServiceBase(const QString& name=QString(),const QString& type=QString(),
		    const QString& domain=QString(), const QString& host=QString(),
		    unsigned short port=0);

	virtual  ~ServiceBase();

	/**
	Returns name of service.
	 */
	QString serviceName() const;

	/**
	Returns type of service. It always in format _sometype._udp or _sometype._tcp.
	 */
	QString type() const;

	/**
	Returns domain that given service belongs to. It is "local." for link-local services.
	 */
	QString domain() const;

	/**
	Returns hostname. It is only valid for local and resolved remote services.
	 */
	QString hostName() const;

	/**
	Returns port number. It is only valid for local and resolved remote services.
	 */
	unsigned short port() const;

	/**
	Returns read only map of text properties.  It is only valid for local and resolved remote services.
	 */
	QMap<QString,QByteArray> textData() const;
	
	/**
	Compares service name, type and domain. This is enough to for unique identification and omitting
	port, host and text data allows to compare resolved and unresolved services */
	bool operator==(const ServiceBase& o) const;
	bool operator!=(const ServiceBase& o) const;

protected:
	ServiceBase(ServiceBasePrivate* const d);
	virtual void virtual_hook(int, void*);
	ServiceBasePrivate* const d;

};

/* Utils functions */

/** Return true if domain is link-local ('local.') */
bool domainIsLocal(const QString& domain);

}

#endif
