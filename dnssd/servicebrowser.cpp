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

#include <QtCore/QStringList>
#include "domainbrowser.h"
#include "query.h"
#include "servicebrowser.h"
#include "servicebrowser_p.h"
#include <config-dnssd.h>
#include <QHash>
#include <dns_sd.h>

namespace DNSSD
{

const QString ServiceBrowser::AllServices = "_services._dns-sd._udp";


ServiceBrowser::ServiceBrowser(const QString& type,bool autoResolve,const QString& domain)
	:d(new ServiceBrowserPrivate(this))
{
	d->m_type=type;
	d->m_autoResolve=autoResolve;
	d->m_domain=domain;
}


ServiceBrowser::State ServiceBrowser::isAvailable()
{
//	DNSServiceRef ref;
//	bool ok (DNSServiceCreateConnection(&ref)==kDNSServiceErr_NoError);
//	if (ok) DNSServiceRefDeallocate(ref);
//	return (ok) ? Working : Stopped;
	return Working;
}
ServiceBrowser::~ ServiceBrowser()
{
	delete d->m_resolver;
	delete d;
}

void ServiceBrowserPrivate::serviceResolved(bool success)
{
	QObject* sender_obj = const_cast<QObject*>(sender());
	RemoteService* svr = static_cast<RemoteService*>(sender_obj);
	disconnect(svr,SIGNAL(resolved(bool)),this,SLOT(serviceResolved(bool)));
	QList<RemoteService::Ptr>::Iterator it = m_duringResolve.begin();
	QList<RemoteService::Ptr>::Iterator itEnd = m_duringResolve.end();
	while ( it!= itEnd && svr!= (*it).data()) ++it;
	if (it != itEnd) {
		if (success) {
		  	m_services+=(*it);
			emit m_parent->serviceAdded(RemoteService::Ptr(svr));
		}
		m_duringResolve.erase(it);
		queryFinished();
	}
}

void ServiceBrowser::startBrowse()
{
	if (d->m_running) return;
	d->m_running=true;
	Query* b = new Query(d->m_type,d->m_domain);
	connect(b,SIGNAL(serviceAdded(DNSSD::RemoteService::Ptr)),this,
			SLOT(gotNewService(DNSSD::RemoteService::Ptr)));
	connect(b,SIGNAL(serviceRemoved(DNSSD::RemoteService::Ptr )),this,
			SLOT(gotRemoveService(DNSSD::RemoteService::Ptr)));
	connect(b,SIGNAL(finished()),this,SLOT(queryFinished()));
	b->startQuery();
	d->m_resolver=b;
}

void ServiceBrowserPrivate::gotNewService(RemoteService::Ptr svr)
{
	if (findDuplicate(svr)==(m_services.end()))  {
		if (m_autoResolve) {
			connect(svr.data(),SIGNAL(resolved(bool )),this,SLOT(serviceResolved(bool )));
			m_duringResolve+=svr;
			svr->resolveAsync();
		} else	{
			m_services+=svr;
			emit m_parent->serviceAdded(svr);
		}
	}
}

void ServiceBrowserPrivate::gotRemoveService(RemoteService::Ptr svr)
{
	QList<RemoteService::Ptr>::Iterator it = findDuplicate(svr);
	if (it!=(m_services.end())) {
		emit m_parent->serviceRemoved(*it);
		m_services.erase(it);
	}
}

void ServiceBrowserPrivate::queryFinished()
{
	if (!m_duringResolve.count() && m_resolver->isFinished()) emit m_parent->finished();
}


QList<RemoteService::Ptr> ServiceBrowser::services() const
{
	return d->m_services;
}

void ServiceBrowser::virtual_hook(int, void*)
{}

QList<RemoteService::Ptr>::Iterator ServiceBrowserPrivate::findDuplicate(RemoteService::Ptr src)
{
	QList<RemoteService::Ptr>::Iterator itEnd = m_services.end();
	for (QList<RemoteService::Ptr>::Iterator it = m_services.begin(); it!=itEnd; ++it)
		if ((src->type()==(*it)->type()) && (src->serviceName()==(*it)->serviceName()) &&
				   (src->domain() == (*it)->domain())) return it;
	return itEnd;
}


}

#include "servicebrowser.moc"
#include "servicebrowser_p.moc"
