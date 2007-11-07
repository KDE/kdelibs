/* This file is part of the KDE project
 *
 * Copyright (C) 2004, 2007 Jakub Stachowski <qbast@go2.pl>
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

#ifndef SERVICEBASE_P_H
#define SERVICEBASE_P_H

#include <QtCore/QMap>
#include <QtCore/QString>

namespace DNSSD {

class ServiceBasePrivate
{
public:
	ServiceBasePrivate(const QString& name, const QString& type, const QString& domain, 
		const QString& host, unsigned short port) : m_serviceName(name), m_type(type),
		m_domain(domain), m_hostName(host), m_port(port) {}

    virtual ~ServiceBasePrivate() {}

	QString m_serviceName;
	QString m_type;
	QString m_domain;
	QString m_hostName;
	unsigned short m_port;

	/**
	Map of TXT properties
	 */
	QMap<QString,QByteArray> m_textData;
};
}
#endif
