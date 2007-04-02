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

#include <netinet/in.h>
#include <qeventloop.h>
#include <qapplication.h>
#include <kdebug.h>
#include "remoteservice.h"
#include "avahi_server_interface.h"
#include "avahi_serviceresolver_interface.h"
#include "avahi-remoteservice_p.h"

Q_DECLARE_METATYPE(QList<QByteArray>);

namespace DNSSD
{

RemoteService::RemoteService(const QString& name,const QString& type,const QString& domain)
		: ServiceBase(name, type, domain), d(new RemoteServicePrivate(this))
{
}


RemoteService::~RemoteService()
{
	delete d;
}

bool RemoteService::resolve()
{
	resolveAsync();
	while (d->m_running && !d->m_resolved) QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	return d->m_resolved;
}

void RemoteService::resolveAsync()
{
	if (d->m_running) return;
	d->m_resolved = false;
        registerTypes();
	kDebug() << this << ":Starting resolve of : " << m_serviceName << " " << m_type << " " << m_domain << "\n";
	org::freedesktop::Avahi::Server s("org.freedesktop.Avahi","/",QDBusConnection::systemBus());
	//FIXME: don't use LOOKUP_NO_ADDRESS if NSS unavailable 
	QDBusReply<QDBusObjectPath> rep=s.ServiceResolverNew(-1, -1, m_serviceName, m_type, domainToDNS(m_domain), -1, 8 /*AVAHI_LOOKUP_NO_ADDRESS*/);
	if (!rep.isValid()) {
	    emit resolved(false);
	    return;
	}
	
	org::freedesktop::Avahi::ServiceResolver *b=new org::freedesktop::Avahi::ServiceResolver("org.freedesktop.Avahi",rep.value().path(),
	    QDBusConnection::systemBus());
	connect(b,SIGNAL(Found(int,int,const QString &,const QString &,const QString &,const QString &, int, const QString &,ushort,
	     const QList<QByteArray>&, uint)),d, SLOT(gotFound(int,int,const QString &,const QString &,const QString &,const QString &,
	     int, const QString &,ushort , const QList<QByteArray>&, uint)));
	connect(b,SIGNAL(Failure(const QString&)),d, SLOT(gotError()));
	d->m_running=true;
}

bool RemoteService::isResolved() const
{
	return d->m_resolved;
}

void RemoteServicePrivate::gotError()
{
	m_resolved=false;
	stop();
		
	emit m_parent->resolved(false);
}

void RemoteServicePrivate::gotFound(int, int, const QString &name, const QString &type, const QString &domain, const QString &host, int, const QString &, ushort port, const QList<QByteArray> &txt, uint)
{
	m_parent->m_serviceName = name;
	m_parent->m_hostName = host;
	m_parent->m_port = port;
	m_parent->m_domain=DNSToDomain(domain);
	Q_FOREACH(QByteArray x, txt) {
		int pos=x.indexOf("=");
		if (pos==-1) m_parent->m_textData[x]=QByteArray();
		else m_parent->m_textData[x.mid(0,pos)]=x.mid(pos+1,x.size()-pos);
	}
	m_resolved = true;
	emit m_parent->resolved(true);
}

void RemoteServicePrivate::stop()
{
    if (m_resolver) m_resolver->Free();
    delete m_resolver;
    m_resolver=0;
    m_running=false;
}

void RemoteService::virtual_hook(int, void*)
{
	// BASE::virtual_hook(int, void*);
}

QDataStream & operator<< (QDataStream & s, const RemoteService & a)
{
	s << (static_cast<ServiceBase>(a));
	qint8 resolved = a.d->m_resolved ? 1:0;
	s << resolved;
	return s;
}

QDataStream & operator>> (QDataStream & s, RemoteService & a)
{
	// stop any possible resolve going on
	a.d->stop();
	qint8 resolved;
	operator>>(s,(static_cast<ServiceBase&>(a)));
	s >> resolved;
	a.d->m_resolved = (resolved == 1);	
	return s;
}

}

#include "remoteservice.moc"
#include "avahi-remoteservice_p.moc"
