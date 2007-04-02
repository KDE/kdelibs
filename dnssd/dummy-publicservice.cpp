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

#include "publicservice.h"
#include <QStringList>

namespace DNSSD
{

PublicService::PublicService(const QString& name, const QString& type, unsigned int port,
			      const QString& domain, const QStringList&)
  		: QObject(), ServiceBase(name, type, QString(), domain, port), d(0)
{
	if (domain.isNull())  m_domain="local.";
}


PublicService::~PublicService()
{}

void PublicService::setServiceName(const QString& serviceName)
{
	m_serviceName = serviceName;
}

void PublicService::setDomain(const QString& domain)
{
	m_domain = domain;
}


void PublicService::setType(const QString& type)
{
	m_type = type;
}

void PublicService::setSubType(const QString& type)
{
	m_subtype = type;
}

void PublicService::setPort(unsigned short port)
{
	m_port = port;
}

QStringList PublicService::subtypes() const
{
	return QStringList();
}

bool PublicService::isPublished() const
{
	return false;
}

void PublicService::setTextData(const QMap<QString,QString>& textData)
{
	m_textData = textData;
}

bool PublicService::publish()
{
    return false;
}

void PublicService::stop()
{}

void PublicService::publishAsync()
{
	emit published(false);
}

void PublicService::virtual_hook(int, void*)
{
}

}

#include "publicservice.moc"
