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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <qstringlist.h>
#include "domainbrowser.h"
#include "query.h"
#include "servicebrowser.h"

namespace DNSSD
{

const QString ServiceBrowser::AllServices = "_services._dns-sd._udp";

class ServiceBrowserPrivate 
{
public:	
	ServiceBrowserPrivate() : m_running(false) 
	{}
	QValueList<RemoteService::Ptr> m_services;
	QValueList<RemoteService::Ptr> m_duringResolve;
	QString m_type;
	bool m_autoresolve;
	DomainBrowser* m_domains;
	bool m_running;
	bool m_finished;
	QDict<Query> resolvers;
};

ServiceBrowser::ServiceBrowser(const QString& type,DomainBrowser* domains,bool autoResolve)
{
	if (domains) init(type,domains,autoResolve);
		else init(type,new DomainBrowser(this),autoResolve);
}

void ServiceBrowser::init(const QString& type,DomainBrowser* domains,bool autoResolve)
{
	d = new ServiceBrowserPrivate();
	d->resolvers.setAutoDelete(true);
	d->m_type=type;
	d->m_autoresolve=autoResolve;
	d->m_domains = domains;
	connect(d->m_domains,SIGNAL(domainAdded(const QString& )),this,SLOT(addDomain(const QString& )));
	connect(d->m_domains,SIGNAL(domainRemoved(const QString& )),this,
		SLOT(removeDomain(const QString& )));
}
ServiceBrowser::ServiceBrowser(const QString& type,const QString& domain,bool autoResolve)
{
	init(type,new DomainBrowser(domain,false,this),autoResolve);
}
ServiceBrowser::~ ServiceBrowser()
{
	delete d;
}

void ServiceBrowser::serviceResolved(bool success)
{
	QObject* sender_obj = const_cast<QObject*>(sender());
	RemoteService* svr = static_cast<RemoteService*>(sender_obj);
	disconnect(svr,SIGNAL(resolved(bool)),this,SLOT(serviceResolved(bool)));
	QValueList<RemoteService::Ptr>::Iterator it = d->m_duringResolve.begin();
	QValueList<RemoteService::Ptr>::Iterator itEnd = d->m_duringResolve.end();
	while ( it!= itEnd && svr!= (*it)) ++it;
	if (it != itEnd) {
		if (success) {
		  	d->m_services+=(*it);
			emit serviceAdded(svr);
		}
		d->m_duringResolve.remove(it);
		queryFinished();
	}
}

void ServiceBrowser::startBrowse()
{
	if (d->m_running) return;
	d->m_running=true;
	if (d->m_domains->isRunning()) {
		QStringList::const_iterator itEnd  = d->m_domains->domains().end();
		for ( QStringList::const_iterator it = d->m_domains->domains().begin(); it != itEnd; ++it )
			addDomain(*it);
	} else d->m_domains->startBrowse();
}

void ServiceBrowser::gotNewService(RemoteService::Ptr svr)
{
	if (findDuplicate(svr)==(d->m_services.end()))  {
		if (d->m_autoresolve) {
			connect(svr,SIGNAL(resolved(bool )),this,SLOT(serviceResolved(bool )));
			d->m_duringResolve+=svr;
			svr->resolveAsync();
		} else	{
			d->m_services+=svr;
			emit serviceAdded(svr);
		}
	}
}

void ServiceBrowser::gotRemoveService(RemoteService::Ptr svr)
{
	QValueList<RemoteService::Ptr>::Iterator it = findDuplicate(svr);
	if (it!=(d->m_services.end())) {
		emit serviceRemoved(*it);
		d->m_services.remove(it);
	}
}


void ServiceBrowser::removeDomain(const QString& domain)
{
	d->resolvers.remove(domain);
	for ( QValueList<RemoteService::Ptr>::Iterator it = d->m_services.begin(); 
		     it!=d->m_services.end(); ++it)
		if ((*it)->domain() == domain) {
		emit serviceRemoved(*it);
			it = d->m_services.remove(it);
		}
}

void ServiceBrowser::addDomain(const QString& domain)
{
	if (!d->m_running) return;
	if (!(d->resolvers[domain])) {
		Query* b = new Query(d->m_type,domain);
		connect(b,SIGNAL(serviceAdded(DNSSD::RemoteService::Ptr)),this,
			SLOT(gotNewService(DNSSD::RemoteService::Ptr)));
		connect(b,SIGNAL(serviceRemoved(DNSSD::RemoteService::Ptr )),this,
			SLOT(gotRemoveService(DNSSD::RemoteService::Ptr)));
		connect(b,SIGNAL(finished()),this,SLOT(queryFinished()));
		b->startQuery();
		d->resolvers.insert(domain,b);
	}
}

void ServiceBrowser::queryFinished()
{
	if (allFinished()) emit finished();
}

bool ServiceBrowser::allFinished()
{
	if  (d->m_duringResolve.count()) return false;
	bool all = true;
	QDictIterator<Query> it(d->resolvers);
	for ( ; it.current(); ++it) all&=(*it)->isFinished();
	return all;
}

const QValueList<RemoteService::Ptr>& ServiceBrowser::services() const
{
	return d->m_services;
}

void ServiceBrowser::virtual_hook(int, void*)
{}

QValueList<RemoteService::Ptr>::Iterator ServiceBrowser::findDuplicate(RemoteService::Ptr src)
{
	QValueList<RemoteService::Ptr>::Iterator itEnd = d->m_services.end();
	for (QValueList<RemoteService::Ptr>::Iterator it = d->m_services.begin(); it!=itEnd; ++it) 
		if ((src->type()==(*it)->type()) && (src->serviceName()==(*it)->serviceName()) &&
				   (src->domain() == (*it)->domain())) return it;
	return itEnd;
}


}

#include "servicebrowser.moc"
