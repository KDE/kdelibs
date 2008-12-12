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
#include <QtCore/QDataStream>
#include <QtCore/QUrl>

namespace DNSSD
{

ServiceBase::ServiceBase(const QString& name, const QString& type, const QString& domain,
			 const QString& host, unsigned short port) 
    :  d(new ServiceBasePrivate(name,type,domain,host,port))
{}

ServiceBase::ServiceBase(ServiceBasePrivate* const _d) 
:  d(_d)
{}

ServiceBase::~ServiceBase()
{
    delete d;
}

QString ServiceBase::serviceName() const
{
    return d->m_serviceName;
}

QString ServiceBase::type() const
{
    return d->m_type;
}

QString ServiceBase::domain() const
{
    return d->m_domain;
}

QString ServiceBase::hostName() const
{
    return d->m_hostName;
}

unsigned short ServiceBase::port() const
{
    return d->m_port;
}
QMap<QString,QByteArray> ServiceBase::textData() const
{
    return d->m_textData;
}

bool ServiceBase::operator==(const ServiceBase& o) const
{
    return d->m_domain==o.d->m_domain && d->m_serviceName==o.d->m_serviceName && d->m_type==o.d->m_type;
}

bool ServiceBase::operator!=(const ServiceBase& o) const 
{
    return !(*this == o);
}

void ServiceBase::virtual_hook(int, void*)
{}


bool domainIsLocal(const QString& domain)
{
	return domain.section('.',-1,-1).toLower()==QLatin1String("local");
}

}
