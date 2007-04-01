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

#include "remoteservice.h"
#include <QtCore/QDataStream>

namespace DNSSD
{

RemoteService::RemoteService(const QString& name,const QString& type,const QString& domain, const QString& subtype)
		: ServiceBase(name, type, domain, subtype), d(0)
{
}


RemoteService::~RemoteService()
{
}

bool RemoteService::resolve()
{
    return false;
}

void RemoteService::resolveAsync()
{
	emit resolved(false);
}

bool RemoteService::isResolved() const
{
	return false;
}


void RemoteService::virtual_hook(int, void*)
{
	// BASE::virtual_hook(int, void*);
}

QDataStream & operator<< (QDataStream & s, const RemoteService & a)
{
	s << (static_cast<ServiceBase>(a));
	qint8 resolved = 0;
	s << resolved;
	return s;
}

QDataStream & operator>> (QDataStream & s, RemoteService & a)
{
	// stop any possible resolve going on
	qint8 resolved;
	operator>>(s,(static_cast<ServiceBase&>(a)));
	s >> resolved;
	return s;
}



}

#include "remoteservice.moc"
