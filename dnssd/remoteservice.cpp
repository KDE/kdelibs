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

#include <qeventloop.h>
#include <qapplication.h>
#include <netinet/in.h>
#include "remoteservice.h"
#include "responder.h"
#include "sdevent.h"
#include <kdebug.h>
#include <config.h>

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
	DNSServiceRef ref;
	kdDebug() << this << ":Starting resolve of : " << m_serviceName << " " << m_type << " " << m_domain << "\n";
#ifdef HAVE_DNSSD
	if (DNSServiceResolve(&ref,0,0,m_serviceName.utf8(), m_type, 
		m_domain,resolve_callback,reinterpret_cast<void*>(this))
		== kDNSServiceErr_NoError) d->setRef(ref);
#endif
	kdDebug() << "REF is " << ref << ", running: " << d->isRunning() << "\n";
	if (!d->isRunning()) emit resolved(false);
}

bool RemoteService::isResolved() const
{
	return d->m_resolved;
}

void RemoteService::customEvent(QCustomEvent* event)
{
	if (event->type() == SD_ERROR) {
		d->stop();
		d->m_resolved=false;
		emit resolved(false);
	}
	if (event->type() == SD_RESOLVE) {
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
	ResolveEvent rev(QString::fromUtf8(hosttarget),ntohs(port),map);
	QApplication::sendEvent(obj, &rev);
}
#endif


};

#include "remoteservice.moc"
