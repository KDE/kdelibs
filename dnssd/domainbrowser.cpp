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

#include <qstringlist.h>
#include "domainbrowser.h"
#include "settings.h"
#include "remoteservice.h"
#include "query.h"
#include <kapplication.h>

namespace DNSSD
{

class DomainBrowserPrivate 
{
public:
	Q3Dict<Query> resolvers;
	QStringList m_domains;
	bool m_recursive;
	bool m_running;
};		
	
DomainBrowser::DomainBrowser(QObject *parent) : QObject(parent)
{
	d = new DomainBrowserPrivate;
	d->m_running = false;
	d->resolvers.setAutoDelete(true);
	d->m_recursive = Configuration::recursive();
	d->m_domains = Configuration::domainList();
	if (Configuration::browseLocal()) d->m_domains+="local.";
	connect(KApplication::kApplication(),SIGNAL(kipcMessage(int,int)),this,
	        SLOT(domainListChanged(int,int)));
}

DomainBrowser::DomainBrowser(const QStringList& domains, bool recursive, QObject *parent) : QObject(parent)
{
	d = new DomainBrowserPrivate;
	d->m_recursive = recursive;
	d->m_running = false;
	d->resolvers.setAutoDelete(true);
	d->m_domains=domains;
}


DomainBrowser::~DomainBrowser()
{
	delete d;
}


void DomainBrowser::startBrowse()
{
	if (d->m_running) return;
	d->m_running=true;
	QStringList::const_iterator itEnd = d->m_domains.end();
	for (QStringList::const_iterator it=d->m_domains.begin(); it!=itEnd; ++it ) {
		emit domainAdded(*it);
		if (d->m_recursive) {
			Query* b = new Query("b._dns-sd._udp",(*it));
			connect(b,SIGNAL(serviceAdded(DNSSD::RemoteService::Ptr)),this,
				SLOT(gotNewDomain(DNSSD::RemoteService::Ptr)));
			connect(b,SIGNAL(serviceRemoved(DNSSD::RemoteService::Ptr )),this,
				SLOT(gotRemoveDomain(DNSSD::RemoteService::Ptr)));
			b->startQuery();
			d->resolvers.insert((*it),b);
		}
	}
}

void DomainBrowser::gotNewDomain(DNSSD::RemoteService::Ptr srv)
{
	QString domain = srv->serviceName()+"."+srv->domain();
	if (d->m_domains.contains(domain)) return;
	d->m_domains.append(domain);
	emit domainAdded(domain);
	if (d->m_recursive && !d->resolvers[domain]) {
		Query* b = new Query("b._dns-sd._udp",domain);
		connect(b,SIGNAL(serviceAdded(DNSSD::RemoteService::Ptr)),this,
			SLOT(gotNewDomain(DNSSD::RemoteService::Ptr)));
		connect(b,SIGNAL(serviceRemoved(DNSSD::RemoteService::Ptr )),this,
			SLOT(gotRemoveDomain(DNSSD::RemoteService::Ptr)));
		b->startQuery();
		d->resolvers.insert(domain,b);
	}
}

void DomainBrowser::gotRemoveDomain(DNSSD::RemoteService::Ptr srv)
{
	QString domain = srv->serviceName()+"."+srv->domain();
	d->m_domains.remove(domain);
	emit domainRemoved(domain);
	d->resolvers.remove(domain);
}

void DomainBrowser::domainListChanged(int message,int)
{
	if (message!=KIPCDomainsChanged) return;
	bool was_running = d->m_running;
	d->m_running = false;
	// remove all domains and resolvers
	d->resolvers.clear();
	if (was_running) {
		QStringList::const_iterator itEnd = d->m_domains.end();
		for (QStringList::const_iterator it=d->m_domains.begin(); it!=itEnd; ++it )
			emit domainRemoved(*it);
	}
	d->m_domains.clear();
	// now reread configuration and add domains
	Configuration::self()->readConfig();
	d->m_recursive = Configuration::recursive();
	d->m_domains = Configuration::domainList();
	if (Configuration::browseLocal()) d->m_domains+="local.";
	// this will emit domainAdded() for every domain if necessary
	if (was_running) startBrowse();
}

const QStringList& DomainBrowser::domains() const
{
	return d->m_domains;
}

bool DomainBrowser::isRunning() const
{
	return d->m_running;
}

void DomainBrowser::virtual_hook(int, void*)
{}

}
#include "domainbrowser.moc"
