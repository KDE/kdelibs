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

#include <signal.h>
#include <errno.h>
#include <qstringlist.h>
#include <qfile.h>
#include "domainbrowser.h"
#include "query.h"
#include "servicebrowser.h"
#include <config.h>
#ifdef HAVE_DNSSD
#include <dns_sd.h>
#endif

#define MDNSD_PID "/var/run/mdnsd.pid"

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
	QStringList m_types;
	DomainBrowser* m_domains;
	int m_flags;
	bool m_running;
	bool m_finished;
	QDict<Query> resolvers;
};

ServiceBrowser::ServiceBrowser(const QString& type,DomainBrowser* domains,bool autoResolve)
{
	if (domains) init(type,domains,autoResolve ? AutoResolve : 0);
		else init(type,new DomainBrowser(this),autoResolve ?  AutoResolve|AutoDelete : AutoDelete);
}
ServiceBrowser::ServiceBrowser(const QStringList& types,DomainBrowser* domains,int flags)
{
	if (domains) init(types,domains,flags);
		else init(types,new DomainBrowser(this),flags|AutoDelete);
}

void ServiceBrowser::init(const QStringList& type,DomainBrowser* domains,int flags)
{
	d = new ServiceBrowserPrivate();
	d->resolvers.setAutoDelete(true);
	d->m_types=type;
	d->m_flags=flags;
	d->m_domains = domains;
	connect(d->m_domains,SIGNAL(domainAdded(const QString& )),this,SLOT(addDomain(const QString& )));
	connect(d->m_domains,SIGNAL(domainRemoved(const QString& )),this,
		SLOT(removeDomain(const QString& )));
}
ServiceBrowser::ServiceBrowser(const QString& type,const QString& domain,bool autoResolve)
{
	init(type,new DomainBrowser(domain,false,this),autoResolve ? AutoResolve|AutoDelete : AutoDelete);
}
ServiceBrowser::ServiceBrowser(const QString& type,const QString& domain,int flags)
{
	init(type,new DomainBrowser(domain,false,this),flags | AutoDelete);
}

const ServiceBrowser::State ServiceBrowser::isAvailable()
{
#ifdef HAVE_DNSSD
	QFile f(MDNSD_PID);
	if (!f.open(IO_ReadOnly)) return Stopped; // no pidfile
	QString line;
	if (f.readLine(line,16)<1) return Stopped;
	unsigned int pid = line.toUInt();
	if (pid==0) return Stopped;           // not a pid
	return (kill(pid,0)==0 || errno==EPERM) ? Working : Stopped; 
	// signal 0 only checks if process is running, mdnsd is probably owned by 'nobody' so we will
	// get EPERM, if mdnsd is not running error will be ESRCH
	
#else
	return Unsupported;
#endif
}
ServiceBrowser::~ ServiceBrowser()
{
	if (d->m_flags & AutoDelete) delete d->m_domains;
	delete d;
}

const DomainBrowser* ServiceBrowser::browsedDomains() const
{
	return d->m_domains;
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
	if (isAvailable()!=Working) return;
	if (d->m_domains->isRunning()) {
		QStringList::const_iterator itEnd  = d->m_domains->domains().end();
		for ( QStringList::const_iterator it = d->m_domains->domains().begin(); it != itEnd; ++it )
			addDomain(*it);
	} else d->m_domains->startBrowse();
}

void ServiceBrowser::gotNewService(RemoteService::Ptr svr)
{
	if (findDuplicate(svr)==(d->m_services.end()))  {
		if (d->m_flags & AutoResolve) {
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
	while (d->resolvers[domain]) d->resolvers.remove(domain);
	QValueList<RemoteService::Ptr>::Iterator it = d->m_services.begin();
	while (it!=d->m_services.end()) 
		// use section to skip possible trailing dot
		if ((*it)->domain().section('.',0) == domain.section('.',0)) {
			emit serviceRemoved(*it);
			it = d->m_services.remove(it);
		} else ++it;
}

void ServiceBrowser::addDomain(const QString& domain)
{
	if (!d->m_running) return;
	if (!(d->resolvers[domain])) {
		QStringList::ConstIterator itEnd = d->m_types.end();
		for (QStringList::ConstIterator it=d->m_types.begin(); it!=itEnd; ++it) {
			Query* b = new Query((*it),domain);
			connect(b,SIGNAL(serviceAdded(DNSSD::RemoteService::Ptr)),this,
				SLOT(gotNewService(DNSSD::RemoteService::Ptr)));
			connect(b,SIGNAL(serviceRemoved(DNSSD::RemoteService::Ptr )),this,
				SLOT(gotRemoveService(DNSSD::RemoteService::Ptr)));
			connect(b,SIGNAL(finished()),this,SLOT(queryFinished()));
			b->startQuery();
			d->resolvers.insert(domain,b);
		}
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
