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

#include <config.h>

#include <qeventloop.h>
#include <qapplication.h>
#include <kurl.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <netinet/in.h>
#include "remoteservice.h"
#include "responder.h"
#include "sdevent.h"
#include <kdebug.h>

namespace DNSSD
{
#ifdef HAVE_DNSSD
void resolve_callback    (    DNSServiceRef,
				DNSServiceFlags,
				uint32_t,
				DNSServiceErrorType                 errorCode,
				const char*,
				const char                          *hosttarget,
				uint16_t                            port,
				uint16_t                            txtLen,
				const char                          *txtRecord,
				void                                *context
			 );

#endif
class RemoteServicePrivate : public Responder
{
public:
	RemoteServicePrivate() : Responder(), m_resolved(false)
	{};
	bool m_resolved;
};

RemoteService::RemoteService(const QString& label)
{
	decode(label);
	d =  new RemoteServicePrivate();
}
RemoteService::RemoteService(const QString& name,const QString& type,const QString& domain)
		: ServiceBase(name, type, domain)
{
	d = new RemoteServicePrivate();
}

RemoteService::RemoteService(const KURL& url)
{
	d = new RemoteServicePrivate();
	if (!url.isValid()) return;
	if (url.protocol()!="invitation") return;
	if (!url.hasPath()) return;
	m_hostName = url.host();
	m_port = url.port();
	m_type = url.path().section('/',1,1);
	m_serviceName = url.path().section('/',2);
	m_textData = url.queryItems();
	d->m_resolved=true;
}

RemoteService::~RemoteService()
{
	delete d;
}

bool RemoteService::resolve()
{
	resolveAsync();
	while (d->isRunning() && !d->m_resolved) d->process();
	d->stop();
	return d->m_resolved;
}

void RemoteService::resolveAsync()
{
	if (d->isRunning()) return;
	d->m_resolved = false;
	kdDebug() << this << ":Starting resolve of : " << m_serviceName << " " << m_type << " " << m_domain << "\n";
#ifdef HAVE_DNSSD
	DNSServiceRef ref;
	if (DNSServiceResolve(&ref,0,0,m_serviceName.utf8(), m_type.ascii(), 
		domainToDNS(m_domain),resolve_callback,reinterpret_cast<void*>(this))
		== kDNSServiceErr_NoError) d->setRef(ref);
#endif
	if (!d->isRunning()) emit resolved(false);
}

bool RemoteService::isResolved() const
{
	return d->m_resolved;
}

void RemoteService::customEvent(QCustomEvent* event)
{
	if (event->type() == QEvent::User+SD_ERROR) {
		d->stop();
		d->m_resolved=false;
		emit resolved(false);
	}
	if (event->type() == QEvent::User+SD_RESOLVE) {
		ResolveEvent* rev = static_cast<ResolveEvent*>(event);
		m_hostName = rev->m_hostname;
		m_port = rev->m_port;
		m_textData = rev->m_txtdata;
		d->m_resolved = true;
		emit resolved(true);
	}
}

void RemoteService::virtual_hook(int, void*)
{
	// BASE::virtual_hook(int, void*);
}

QDataStream & operator<< (QDataStream & s, const RemoteService & a)
{
	s << (static_cast<ServiceBase>(a));
	Q_INT8 resolved = a.d->m_resolved ? 1:0;
	s << resolved;
	return s;
}

QDataStream & operator>> (QDataStream & s, RemoteService & a)
{
	// stop any possible resolve going on
	a.d->stop();
	Q_INT8 resolved;
	operator>>(s,(static_cast<ServiceBase&>(a)));
	s >> resolved;
	a.d->m_resolved = (resolved == 1);	
	return s;
}


#ifdef HAVE_DNSSD
void resolve_callback    (    DNSServiceRef,
			      DNSServiceFlags,
			      uint32_t,
			      DNSServiceErrorType                 errorCode,
			      const char*,
			      const char                          *hosttarget,
			      uint16_t                            port,
			      uint16_t                            txtLen,
			      const char                          *txtRecord,
			      void                                *context
			 )
{
	QObject *obj = reinterpret_cast<QObject*>(context);
	if (errorCode != kDNSServiceErr_NoError) {
		ErrorEvent err;
		QApplication::sendEvent(obj, &err);	
		return;
	}
	char key[256];
	const char *value;
	int index=0;
	unsigned char valueLen;
	kdDebug() << "Resolve callback\n";
	QMap<QString,QString> map;
	while (TXTRecordGetItemAtIndex(txtLen,txtRecord,index++,256,key,&valueLen,
		reinterpret_cast<const void **>(&value)) == kDNSServiceErr_NoError) 
		if (value) map[QString::fromUtf8(key)]=QString::fromUtf8(value,valueLen);
			else map[QString::fromUtf8(key)]=QString::null;
	ResolveEvent rev(DNSToDomain(hosttarget),ntohs(port),map);
	QApplication::sendEvent(obj, &rev);
}
#endif


}

#include "remoteservice.moc"
