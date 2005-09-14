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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include "publicservice.h"
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <netinet/in.h>
#include <sys/socket.h>
#include <qapplication.h>
#include <network/ksocketaddress.h>
#include <kurl.h>
#include <unistd.h>
#include "sdevent.h"
#include "responder.h"
#include "settings.h"

namespace DNSSD
{
static unsigned long publicIP();
#ifdef HAVE_DNSSD
void publish_callback (DNSServiceRef, DNSServiceFlags, DNSServiceErrorType errorCode, const char *name,
		       const char*, const char*, void *context);
#endif
class PublicServicePrivate : public Responder
{
public:
	PublicServicePrivate() : m_published(false)
	{}
	bool m_published;
};

PublicService::PublicService(const QString& name, const QString& type, unsigned int port,
			      const QString& domain)
  		: QObject(), ServiceBase(name, type, QString::null, domain, port)
{
	d = new PublicServicePrivate;
	if (domain.isNull())
		if (Configuration::publishType()==Configuration::EnumPublishType::LAN) m_domain="local.";
		else m_domain=Configuration::publishDomain();
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
#ifdef HAVE_DNSSD
	TXTRecordRef txt;
	TXTRecordCreate(&txt,0,0);
	QMap<QString,QString>::ConstIterator itEnd = m_textData.end();
	for (QMap<QString,QString>::ConstIterator it = m_textData.begin(); it!=itEnd ; ++it) {
		QByteArray value = it.data().utf8();
		if (TXTRecordSetValue(&txt,it.key().utf8(),value.length(),value)!=kDNSServiceErr_NoError) {
			TXTRecordDeallocate(&txt);
			emit published(false);
			return;
		}
	}
	DNSServiceRef ref;
	if (DNSServiceRegister(&ref,0,0,m_serviceName.utf8(),m_type.ascii(),domainToDNS(m_domain),NULL,
	    htons(m_port),TXTRecordGetLength(&txt),TXTRecordGetBytesPtr(&txt),publish_callback,
	    reinterpret_cast<void*>(this)) == kDNSServiceErr_NoError) d->setRef(ref);
	TXTRecordDeallocate(&txt);
#endif
	if (!d->isRunning()) emit published(false);
}

#ifdef HAVE_DNSSD
void publish_callback (DNSServiceRef, DNSServiceFlags, DNSServiceErrorType errorCode, const char *name,
		       const char*, const char*, void *context)
{
	QObject *obj = reinterpret_cast<QObject*>(context);
	if (errorCode != kDNSServiceErr_NoError) {
		ErrorEvent err;
		QApplication::sendEvent(obj, &err);
	} else {
		PublishEvent pev(QString::fromUtf8(name));
		QApplication::sendEvent(obj, &pev);
	}
}
#endif

const KURL PublicService::toInvitation(const QString& host)
{
	KURL url;
	url.setProtocol("invitation");
	if (host.isEmpty()) { // select best address
		unsigned long s_address = publicIP();
		if (!s_address) return KURL();
		KNetwork::KIpAddress addr(s_address);
		url.setHost(addr.toString());
	} else 	url.setHost(host);
	//FIXME: if there is no public interface, select any non-loopback
	url.setPort(m_port);
	url.setPath("/"+m_type+"/"+KURL::encode_string(m_serviceName));
	QString query;
	QMap<QString,QString>::ConstIterator itEnd = m_textData.end();
	for (QMap<QString,QString>::ConstIterator it = m_textData.begin(); it!=itEnd ; ++it)
		url.addQueryItem(it.key(),it.data());;
	return url;
}

void PublicService::customEvent(QCustomEvent* event)
{
	if (event->type()==QEvent::User+SD_ERROR) {
		stop();
		emit published(false);
	}
	if (event->type()==QEvent::User+SD_PUBLISH) {
		d->m_published=true;
		emit published(true);
		m_serviceName = static_cast<PublishEvent*>(event)->m_name;
	}
}

void PublicService::virtual_hook(int, void*)
{
}

static unsigned long publicIP()
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	int sock = socket(AF_INET,SOCK_DGRAM,0);
	if (sock == -1) return 0;
	addr.sin_family = AF_INET;
	addr.sin_port = 1;	// Not important, any port and public address will do
	addr.sin_addr.s_addr = 0x11111111;
	if ((connect(sock,(const struct sockaddr*)&addr,sizeof(addr))) == -1) { close(sock); return 0; }
	if ((getsockname(sock,(struct sockaddr*)&addr, &len)) == -1) { close(sock); return 0; }
	::close(sock);
	return addr.sin_addr.s_addr;
}


}

#include "publicservice.moc"
