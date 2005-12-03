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

#include "servicebase.h"
#include <qregexp.h>
#include <QDataStream>

namespace DNSSD
{

ServiceBase::ServiceBase(const QString& name, const QString& type, const QString& domain,
			 const QString& host, unsigned short port) : 
    		m_serviceName(name), m_type(type), m_domain(domain), m_hostName(host), m_port(port)
{}

ServiceBase::~ServiceBase()
{}

QString ServiceBase::encode()
{
	return  m_serviceName.replace(".","\\.").replace("\\","\\\\") + QString(".") + m_type +
			 QString(".") + m_domain;
}

// example: 3rd\.\032Floor\032Copy\032Room._ipp._tcp.dns-sd.org.  - normal service
// 3rd\.\032Floor\032Copy\032Room.dns-sd.org  - domain
// 	_ipp._tcp.dns-sd.org	- metaquery

void ServiceBase::decode(const QString& name)
{
	QString rest;
	if (name[0]=='_') { 		// metaquery
		m_serviceName="";
		rest=name;
	} else {		// normal service or domain
		QString decoded_name=name;
		decoded_name=decoded_name.replace("\\\\","\\");
		int i = decoded_name.find(QRegExp("[^\\\\]\\."));
		if (i==-1) return;            // first find service name
		rest = decoded_name.mid(i+2);
		m_serviceName=decoded_name.left(i+1).replace("\\.",".");
	}
	m_type = rest.section('.',0,1);
	// does it really have a type?
	if (m_type[0]=='_' && m_type[m_type.find('.')+1]=='_')
		m_domain = rest.section('.',2,-1,QString::SectionIncludeTrailingSep);
	else {
		m_type="";
		m_domain=rest;
	}
}

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
const QMap<QString,QString>& ServiceBase::textData() const
{
	return m_textData;
}

void ServiceBase::virtual_hook(int, void*)
{}

QDataStream & operator<< (QDataStream & s, const ServiceBase & a)
{
	s << a.m_serviceName << a.m_type << a.m_domain << a.m_hostName << Q_INT16(a.m_port) << a.m_textData;
	return s;
}

QDataStream & operator>> (QDataStream & s, ServiceBase & a)
{
	Q_INT16 port;
	s >> a.m_serviceName >> a.m_type >> a.m_domain >> a.m_hostName >> port >> a.m_textData;
	a.m_port = port;	
	return s;
}

}
