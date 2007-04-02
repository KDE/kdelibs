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

#include <dnssd/servicebase.h>
#include <qregexp.h>
#include <QDataStream>

namespace DNSSD
{

ServiceBase::ServiceBase(const QString& name, const QString& type, const QString& domain,
			 const QString& host, unsigned short port) : 
    		m_serviceName(name), m_type(type), m_domain(domain), m_hostName(host), m_port(port), d(0)
{}

ServiceBase::~ServiceBase()
{}

const QString& ServiceBase::serviceName() const
{
	return m_serviceName;
}

const QString& ServiceBase::type() const
{
	return m_type;
}

const QString& ServiceBase::domain() const
{
	return m_domain;
}

const QString& ServiceBase::hostName() const
{
	return m_hostName;
}

unsigned short ServiceBase::port() const
{
	return m_port;
}
const QMap<QString,QByteArray>& ServiceBase::textData() const
{
	return m_textData;
}

void ServiceBase::virtual_hook(int, void*)
{}

QDataStream & operator<< (QDataStream & s, const ServiceBase & a)
{
	s << a.m_serviceName << a.m_type << a.m_domain << a.m_hostName << qint16(a.m_port) << a.m_textData;
	return s;
}

QDataStream & operator>> (QDataStream & s, ServiceBase & a)
{
	qint16 port;
	s >> a.m_serviceName >> a.m_type >> a.m_domain >> a.m_hostName >> port >> a.m_textData;
	a.m_port = port;	
	return s;
}

}
