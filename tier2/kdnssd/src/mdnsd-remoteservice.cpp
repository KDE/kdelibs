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
#include <QtCore/QEventLoop>
#include <QtCore/QCoreApplication>
#include <QDebug>
#include "remoteservice.h"
#include "servicebase_p.h"
#include "mdnsd-responder.h"
#include "mdnsd-sdevent.h"

namespace KDNSSD
{
void resolve_callback    (    DNSServiceRef,
				DNSServiceFlags,
				uint32_t,
				DNSServiceErrorType                 errorCode,
				const char*,
				const char                          *hosttarget,
				uint16_t                            port,
				uint16_t                            txtLen,
				const unsigned char                 *txtRecord,
				void                                *context
			 );

#define K_D RemoteServicePrivate* d=static_cast<RemoteServicePrivate*>(this->d)

class RemoteServicePrivate : public Responder, public ServiceBasePrivate
{
public:
	RemoteServicePrivate(RemoteService* parent, const QString& name,const QString& type,const QString& domain) : 
	    Responder(), ServiceBasePrivate(name, type, domain, QString(), 0), 	m_resolved(false), m_parent(parent)
	{}
	bool m_resolved;
	RemoteService* m_parent;
	virtual void customEvent(QEvent* event);
};

RemoteService::RemoteService(const QString& name,const QString& type,const QString& domain)
		: ServiceBase(new RemoteServicePrivate(this, name, type, domain))
{
}


RemoteService::~RemoteService()
{}

bool RemoteService::resolve()
{
	K_D;
	resolveAsync();
	while (d->isRunning() && !d->m_resolved) d->process();
	d->stop();
	return d->m_resolved;
}

void RemoteService::resolveAsync()
{
	K_D;
	if (d->isRunning()) return;
	d->m_resolved = false;
	//qDebug() << this << ":Starting resolve of : " << d->m_serviceName << " " << d->m_type << " " << d->m_domain << "\n";
	DNSServiceRef ref;
	if (DNSServiceResolve(&ref,0,0,d->m_serviceName.toUtf8(), d->m_type.toLatin1().constData(), 
 		domainToDNS(d->m_domain),(DNSServiceResolveReply)resolve_callback,reinterpret_cast<void*>(d))
		== kDNSServiceErr_NoError) d->setRef(ref);
	if (!d->isRunning()) emit resolved(false);
}

bool RemoteService::isResolved() const
{
	K_D;
	return d->m_resolved;
}

void RemoteServicePrivate::customEvent(QEvent* event)
{
	if (event->type() == QEvent::User+SD_ERROR) {
		stop();
		m_resolved=false;
		emit m_parent->resolved(false);
	}
	if (event->type() == QEvent::User+SD_RESOLVE) {
		ResolveEvent* rev = static_cast<ResolveEvent*>(event);
		m_hostName = rev->m_hostname;
		m_port = rev->m_port;
		m_textData = rev->m_txtdata;
		m_resolved = true;
		emit m_parent->resolved(true);
	}
}

void RemoteService::virtual_hook(int, void*)
{
	// BASE::virtual_hook(int, void*);
}


void resolve_callback    (    DNSServiceRef,
			      DNSServiceFlags,
			      uint32_t,
			      DNSServiceErrorType                 errorCode,
			      const char*,
			      const char                          *hosttarget,
			      uint16_t                            port,
			      uint16_t                            txtLen,
			      const unsigned char                 *txtRecord,
			      void                                *context
			 )
{
	QObject *obj = reinterpret_cast<QObject*>(context);
	if (errorCode != kDNSServiceErr_NoError) {
		ErrorEvent err;
		QCoreApplication::sendEvent(obj, &err);	
		return;
	}
	char key[256];
	int index=0;
	unsigned char valueLen;
	//qDebug() << "Resolve callback\n";
	QMap<QString,QByteArray> map;
        const void *voidValue = 0;
	while (TXTRecordGetItemAtIndex(txtLen,txtRecord,index++,256,key,&valueLen,
		&voidValue) == kDNSServiceErr_NoError)  
        {
		if (voidValue) map[QString::fromUtf8(key)]=QByteArray((const char*)voidValue,valueLen);
			else map[QString::fromUtf8(key)].clear();
        }
	ResolveEvent rev(DNSToDomain(hosttarget),ntohs(port),map);
	QCoreApplication::sendEvent(obj, &rev);
}


}

#include "moc_remoteservice.cpp"
