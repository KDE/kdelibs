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

#include "query.h" 
#include "responder.h"
#include "remoteservice.h"
#include "sdevent.h"
#include <qapplication.h>
#include <qtimer.h>

#define TIMEOUT_WAN 2000
#define TIMEOUT_LAN 200

namespace DNSSD
{
#ifdef HAVE_DNSSD  
void query_callback (DNSServiceRef, DNSServiceFlags flags, uint32_t, DNSServiceErrorType errorCode,
		     const char *serviceName, const char *regtype, const char *replyDomain, void *context);
#endif
class QueryPrivate : public Responder
{
public:
	QueryPrivate(const QString& type, const QString& domain) : Responder(), m_finished(false),
	m_domain(domain), m_type(type)
	{};
	bool m_finished;
	QString m_domain;
	QTimer timeout;
	QString m_type;
};

Query::Query(const QString& type, const QString& domain)
{
	d = new QueryPrivate(type,domain);
	connect(&d->timeout,SIGNAL(timeout()),this,SLOT(timeout()));
}


Query::~Query()
{
	delete d;
}

bool Query::isRunning() const
{
	return d->isRunning();
}

bool Query::isFinished() const
{
	return d->m_finished;
}

const QString& Query::domain() const
{
	return d->m_domain;
}

void Query::startQuery()
{
	if (d->isRunning()) return;
	d->m_finished = false;
#ifdef HAVE_DNSSD
	DNSServiceRef ref;
	if (DNSServiceBrowse(&ref,0,0, d->m_type.ascii(), 
	    domainToDNS(d->m_domain),query_callback,reinterpret_cast<void*>(this))
		   == kDNSServiceErr_NoError) d->setRef(ref);
#endif
	if (!d->isRunning()) emit finished();
		else d->timeout.start(domainIsLocal(d->m_domain) ? TIMEOUT_LAN : TIMEOUT_WAN,true);
}
void Query::virtual_hook(int, void*)
{
}

void Query::customEvent(QCustomEvent* event)
{
	if (event->type()==QEvent::User+SD_ERROR) {
		d->stop();
		d->m_finished=false;
		emit finished();
	}
	if (event->type()==QEvent::User+SD_ADDREMOVE) {
		AddRemoveEvent *aev = static_cast<AddRemoveEvent*>(event);
		// m_type has useless trailing dot
		RemoteService*  svr = new RemoteService(aev->m_name+"."+
		    	aev->m_type.left(aev->m_type.length()-1)+"."+aev->m_domain);
		if (aev->m_op==AddRemoveEvent::Add) emit serviceAdded(svr);
			else emit serviceRemoved(svr);
		d->m_finished = aev->m_last;
		if (d->m_finished) emit finished();
	}
}

void Query::timeout()
{
	d->m_finished=true;
	emit finished();
}
#ifdef HAVE_DNSSD
void query_callback (DNSServiceRef, DNSServiceFlags flags, uint32_t, DNSServiceErrorType errorCode,
		     const char *serviceName, const char *regtype, const char *replyDomain,
		     void *context)
{
	QObject *obj = reinterpret_cast<QObject*>(context);
	if (errorCode != kDNSServiceErr_NoError) {
		ErrorEvent err;
		QApplication::sendEvent(obj, &err);
	} else {
		AddRemoveEvent arev((flags & kDNSServiceFlagsAdd) ? AddRemoveEvent::Add :
			AddRemoveEvent::Remove, QString::fromUtf8(serviceName), regtype, 
			DNSToDomain(replyDomain), !(flags & kDNSServiceFlagsMoreComing));
		QApplication::sendEvent(obj, &arev);
	}
}
#endif
}
#include "query.moc"
