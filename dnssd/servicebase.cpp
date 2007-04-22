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
#include "servicebase_p.h"
#include <QtCore/QRegExp>
#include <QDataStream>
#include <QtCore/QUrl>

namespace DNSSD
{

ServiceBase::ServiceBase(const QString& name, const QString& type, const QString& domain,
			 const QString& host, unsigned short port) :  dptr(new ServiceBasePrivate(name,type,domain,host,port))
{}

ServiceBase::ServiceBase(ServiceBasePrivate* const d) :  dptr(d)
{}

ServiceBase::~ServiceBase()
{}

QString ServiceBase::serviceName() const
{
	return dptr->m_serviceName;
}

QString ServiceBase::type() const
{
	return dptr->m_type;
}

QString ServiceBase::domain() const
{
	return dptr->m_domain;
}

QString ServiceBase::hostName() const
{
	return dptr->m_hostName;
}

unsigned short ServiceBase::port() const
{
	return dptr->m_port;
}
QMap<QString,QByteArray> ServiceBase::textData() const
{
	return dptr->m_textData;
}

void ServiceBase::virtual_hook(int, void*)
{}

bool domainIsLocal(const QString& domain)
{
	return domain.section('.',-1,-1).toLower()==QLatin1String("local");
}

}
