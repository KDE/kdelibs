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

#include "avahi-remoteservice_p.h"
#include <netinet/in.h>
#include <QtCore/QEventLoop>
#include <QtCore/QCoreApplication>
#include <QDebug>
#include "remoteservice.h"
#include "avahi_server_interface.h"
#include "avahi_serviceresolver_interface.h"
namespace KDNSSD
{

RemoteService::RemoteService(const QString& name,const QString& type,const QString& domain)
		: ServiceBase(new RemoteServicePrivate(this, name,type,domain))
{
}


RemoteService::~RemoteService()
{
}

bool RemoteService::resolve()
{
	K_D;
	resolveAsync();
	while (d->m_running && !d->m_resolved) QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	return d->m_resolved;
}

void RemoteService::resolveAsync()
{
	K_D;
	if (d->m_running) return;
	d->m_resolved = false;
        registerTypes();
	//qDebug() << this << ":Starting resolve of : " << d->m_serviceName << " " << d->m_type << " " << d->m_domain << "\n";
	org::freedesktop::Avahi::Server s("org.freedesktop.Avahi","/",QDBusConnection::systemBus());
	//FIXME: don't use LOOKUP_NO_ADDRESS if NSS unavailable 
	QDBusReply<QDBusObjectPath> rep=s.ServiceResolverNew(-1, -1, d->m_serviceName, d->m_type, 
	    domainToDNS(d->m_domain), -1, 8 /*AVAHI_LOOKUP_NO_ADDRESS*/);
	if (!rep.isValid()) {
	    emit resolved(false);
	    return;
	}
	
	org::freedesktop::Avahi::ServiceResolver *b=new org::freedesktop::Avahi::ServiceResolver("org.freedesktop.Avahi",rep.value().path(),
	    QDBusConnection::systemBus());
	connect(b,SIGNAL(Found(int,int,const QString &,const QString &,const QString &,const QString &, int, const QString &,ushort,
	     const QList<QByteArray>&, uint)),d, SLOT(gotFound(int,int,const QString &,const QString &,const QString &,const QString &,
	     int, const QString &,ushort , const QList<QByteArray>&, uint)));
	connect(b,SIGNAL(Failure(QString)),d, SLOT(gotError()));
	d->m_running=true;
}

bool RemoteService::isResolved() const
{
	K_D;
	return d->m_resolved;
}

void RemoteServicePrivate::gotError()
{
	m_resolved=false;
	stop();
		
	emit m_parent->resolved(false);
}

void RemoteServicePrivate::gotFound(int, int, const QString &name, const QString &, const QString &domain, const QString &host, int, const QString &, ushort port, const QList<QByteArray> &txt, uint)
{
	m_serviceName = name;
	m_hostName = host;
	m_port = port;
	m_domain=DNSToDomain(domain);
	Q_FOREACH(const QByteArray &x, txt) {
		int pos=x.indexOf("=");
		if (pos==-1) m_textData[x]=QByteArray();
		else m_textData[x.mid(0,pos)]=x.mid(pos+1,x.size()-pos);
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


}

#include "moc_remoteservice.cpp"
#include "moc_avahi-remoteservice_p.cpp"
