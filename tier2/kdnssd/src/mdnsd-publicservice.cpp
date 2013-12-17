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
#include <QtCore/QStringList>
#include <netinet/in.h>
#include "publicservice.h"
#include "servicebase_p.h"
#include "mdnsd-sdevent.h"
#include "mdnsd-responder.h"

#define K_D PublicServicePrivate* d=static_cast<PublicServicePrivate*>(this->d)

namespace KDNSSD
{
void publish_callback (DNSServiceRef, DNSServiceFlags, DNSServiceErrorType errorCode, const char *name,
		       const char*, const char*, void *context);
class PublicServicePrivate : public Responder, public ServiceBasePrivate
{
public:
	PublicServicePrivate(PublicService* parent, const QString& name, const QString& type, unsigned int port,
			      const QString& domain) : Responder(), ServiceBasePrivate(name, type, domain, QString(), port),
	    m_published(false), m_parent(parent)
	{}
	bool m_published;
	PublicService* m_parent;
	QStringList m_subtypes;
	virtual void customEvent(QEvent* event);
};

PublicService::PublicService(const QString& name, const QString& type, unsigned int port,
			      const QString& domain, const QStringList& subtypes)
  		: QObject(), ServiceBase(new PublicServicePrivate(this, name, type, port, domain))
{
	K_D;
	if (domain.isNull())  d->m_domain="local.";
	d->m_subtypes=subtypes;
}


PublicService::~PublicService()
{
	stop();
}

void PublicService::setServiceName(const QString& serviceName)
{
	K_D;
	d->m_serviceName = serviceName;
	if (d->isRunning()) {
		stop();
		publishAsync();
	}
}

void PublicService::setDomain(const QString& domain)
{
	K_D;
	d->m_domain = domain;
	if (d->isRunning()) {
	stop();
	publishAsync();
	}
}

QStringList PublicService::subtypes() const
{
	K_D;
	return d->m_subtypes;
}

void PublicService::setType(const QString& type)
{
	K_D;
	d->m_type = type;
	if (d->isRunning()) {
		stop();
		publishAsync();
	}
}

void PublicService::setSubTypes(const QStringList& subtypes)
{
	K_D;
	d->m_subtypes = subtypes;
	if (d->isRunning()) {
		stop();
		publishAsync();
	}
}

void PublicService::setPort(unsigned short port)
{
	K_D;
	d->m_port = port;
	if (d->isRunning()) {
		stop();
		publishAsync();
	}
}

bool PublicService::isPublished() const
{
	K_D;
	return d->m_published;
}

void PublicService::setTextData(const QMap<QString,QByteArray>& textData)
{
	K_D;
	d->m_textData = textData;
	if (d->isRunning()) {
		stop();
		publishAsync();
	}
}

bool PublicService::publish()
{
	K_D;
	publishAsync();
	while (d->isRunning() && !d->m_published) d->process();
	return d->m_published;
}

void PublicService::stop()
{
	K_D;
	d->stop();
	d->m_published = false;
}

void PublicService::publishAsync()
{
	K_D;
	if (d->isRunning()) stop();
	TXTRecordRef txt;
	TXTRecordCreate(&txt,0,0);
	QMap<QString,QByteArray>::ConstIterator itEnd = d->m_textData.cend();
	for (QMap<QString,QByteArray>::ConstIterator it = d->m_textData.cbegin(); it!=itEnd ; ++it) {
		if (TXTRecordSetValue(&txt,it.key().toUtf8(),it.value().length(),it.value())!=kDNSServiceErr_NoError) {
			TXTRecordDeallocate(&txt);
			emit published(false);
			return;
		}
	}
	DNSServiceRef ref;
	QString fullType=d->m_type;
	Q_FOREACH(const QString &subtype, d->m_subtypes) fullType+=','+subtype;
	if (DNSServiceRegister(&ref,0,0,d->m_serviceName.toUtf8(),fullType.toLatin1().constData(),domainToDNS(d->m_domain),NULL,
	    htons(d->m_port),TXTRecordGetLength(&txt),TXTRecordGetBytesPtr(&txt),publish_callback,
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
		m_serviceName = static_cast<PublishEvent*>(event)->m_name;
	}
}

void PublicService::virtual_hook(int, void*)
{
}

}

#include "moc_publicservice.cpp"
