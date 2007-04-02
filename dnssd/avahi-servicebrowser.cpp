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
#include "servicebrowser.h"
#include "avahi_servicebrowser_interface.h"
#include "avahi_server_interface.h"
#include "avahi-servicebrowser_p.h"
#include <QHash>

Q_DECLARE_METATYPE(QList<QByteArray>);

namespace DNSSD
{

//FIXME: implement servicetypebrowser
const QString ServiceBrowser::AllServices = "_services._dns-sd._udp";


ServiceBrowser::ServiceBrowser(const QString& type,bool autoResolve,const QString& domain, const QString& subtype)
	:d(new ServiceBrowserPrivate(this))
{
	d->m_type=type;
	d->m_subtype=subtype;
	d->m_autoResolve=autoResolve;
	d->m_domain=domain;
}

ServiceBrowser::State ServiceBrowser::isAvailable()
{
	org::freedesktop::Avahi::Server s("org.freedesktop.Avahi","/",QDBusConnection::systemBus());
	QDBusReply<int> rep= s.GetState();
	return (rep.isValid() && rep.value()==1) ? Working:Stopped;
}
ServiceBrowser::~ ServiceBrowser()
{
}


void ServiceBrowser::startBrowse()
{
	if (d->m_running) return;
	org::freedesktop::Avahi::Server s("org.freedesktop.Avahi","/",QDBusConnection::systemBus());
	QString fullType=d->m_type;
	if (!d->m_subtype.isEmpty()) fullType=d->m_subtype+"._sub."+d->m_type;
	QDBusReply<QDBusObjectPath> rep=s.ServiceBrowserNew(-1, -1, fullType, d->m_domain,0);
	
	if (!rep.isValid()) return;
	d->m_running=true;
	d->m_browserFinished=true;
	org::freedesktop::Avahi::ServiceBrowser *b=new org::freedesktop::Avahi::ServiceBrowser("org.freedesktop.Avahi",rep.value().path(),
	    QDBusConnection::systemBus());
	connect(b,SIGNAL(ItemNew(int,int,const QString&,const QString&,const QString&,uint)),d, 
	    SLOT(gotNewService(int,int,const QString&,const QString&,const QString&, uint)));
	connect(b,SIGNAL(ItemRemove(int,int,const QString&,const QString&,const QString&,uint)),d, 
	    SLOT(gotRemoveService(int,int,const QString&,const QString&,const QString&, uint)));
	connect(b,SIGNAL(allForNow()),d,SLOT(browseFinished()));
	d->m_browser=b;
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


void ServiceBrowserPrivate::gotNewService(int,int,const QString& name, const QString& type, const QString& domain, uint)
{
	RemoteService::Ptr svr(new RemoteService(name, type,domain));
	if (m_autoResolve) {
		connect(svr.data(),SIGNAL(resolved(bool )),this,SLOT(serviceResolved(bool )));
		m_duringResolve+=svr;
		svr->resolveAsync();
	} else	{
		m_services+=svr;
		emit m_parent->serviceAdded(svr);
	}
}

void ServiceBrowserPrivate::gotRemoveService(int,int,const QString& name, const QString& type, const QString& domain, uint)
{
	RemoteService::Ptr svr(new RemoteService(name, type,domain));
	emit m_parent->serviceRemoved(svr);
	m_services.removeAll(svr);
}
void ServiceBrowserPrivate::browserFinished()
{
    m_browserFinished=true;
    queryFinished();
}

void ServiceBrowserPrivate::queryFinished()
{
	if (!m_duringResolve.count() && m_browserFinished) emit m_parent->finished();
}


QList<RemoteService::Ptr> ServiceBrowser::services() const
{
	return d->m_services;
}

void ServiceBrowser::virtual_hook(int, void*)
{}


}

#include "servicebrowser.moc"
#include "avahi-servicebrowser_p.moc"
