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

#include <QtCore/QCoreApplication>
#include <netinet/in.h>
#include "publicservice.h"
#include "mdnsd-sdevent.h"
#include "mdnsd-responder.h"
#include "settings.h"

namespace DNSSD
{
void publish_callback (DNSServiceRef, DNSServiceFlags, DNSServiceErrorType errorCode, const char *name,
		       const char*, const char*, void *context);
class PublicServicePrivate : public Responder
{
public:
	PublicServicePrivate(PublicService* parent) : m_published(false), m_parent(parent)
	{}
	bool m_published;
	PublicService* m_parent;
	virtual void customEvent(QEvent* event);
};

PublicService::PublicService(const QString& name, const QString& type, unsigned int port,
			      const QString& domain)
  		: QObject(), ServiceBase(name, type, QString(), domain, port),d(new PublicServicePrivate( this))
{
	if (domain.isNull())  m_domain="local.";
}


PublicService::~PublicService()
{
	stop();
	delete d;
}

void PublicService::setServiceName(const QString& serviceName)
{
	m_serviceName = serviceName;
	if (d->isRunning()) {
		stop();
		publishAsync();
	}
}

void PublicService::setDomain(const QString& domain)
{
	m_domain = domain;
	if (d->isRunning()) {
	stop();
	publishAsync();
	}
}


void PublicService::setType(const QString& type)
{
	m_type = type;
	if (d->isRunning()) {
		stop();
		publishAsync();
	}
}

void PublicService::setPort(unsigned short port)
{
	m_port = port;
	if (d->isRunning()) {
		stop();
		publishAsync();
	}
}

bool PublicService::isPublished() const
{
	return d->m_published;
}

void PublicService::setTextData(const QMap<QString,QString>& textData)
{
	m_textData = textData;
	if (d->isRunning()) {
		stop();
		publishAsync();
	}
}

bool PublicService::publish()
{
	publishAsync();
	while (d->isRunning() && !d->m_published) d->process();
	return d->m_published;
}

void PublicService::stop()
{
	d->stop();
	d->m_published = false;
}

void PublicService::publishAsync()
{
	if (d->isRunning()) stop();
	TXTRecordRef txt;
	TXTRecordCreate(&txt,0,0);
	QMap<QString,QString>::ConstIterator itEnd = m_textData.end();
	for (QMap<QString,QString>::ConstIterator it = m_textData.begin(); it!=itEnd ; ++it) {
		QByteArray value = it.value().toUtf8();
		if (TXTRecordSetValue(&txt,it.key().toUtf8(),value.length(),value)!=kDNSServiceErr_NoError) {
			TXTRecordDeallocate(&txt);
			emit published(false);
			return;
		}
	}
	DNSServiceRef ref;
	if (DNSServiceRegister(&ref,0,0,m_serviceName.toUtf8(),m_type.toAscii().constData(),domainToDNS(m_domain),NULL,
	    htons(m_port),TXTRecordGetLength(&txt),TXTRecordGetBytesPtr(&txt),publish_callback,
	    reinterpret_cast<void*>(d)) == kDNSServiceErr_NoError) d->setRef(ref);
	TXTRecordDeallocate(&txt);
	if (!d->isRunning()) emit published(false);
}

void publish_callback (DNSServiceRef, DNSServiceFlags, DNSServiceErrorType errorCode, const char *name,
		       const char*, const char*, void *context)
{
	QObject *obj = reinterpret_cast<QObject*>(context);
	if (errorCode != kDNSServiceErr_NoError) {
		ErrorEvent err;
		QCoreApplication::sendEvent(obj, &err);
	} else {
		PublishEvent pev(QString::fromUtf8(name));
		QCoreApplication::sendEvent(obj, &pev);
	}
}

void PublicServicePrivate::customEvent(QEvent* event)
{
	if (event->type()==QEvent::User+SD_ERROR) {
		m_parent->stop();
		emit m_parent->published(false);
	}
	if (event->type()==QEvent::User+SD_PUBLISH) {
		m_published=true;
		emit m_parent->published(true);
		m_parent->m_serviceName = static_cast<PublishEvent*>(event)->m_name;
	}
}

void PublicService::virtual_hook(int, void*)
{
}

}

#include "publicservice.moc"
