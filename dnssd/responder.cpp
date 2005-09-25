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

#include "responder.h"
#include <qapplication.h>
#include <kidna.h>

// dns_sd.h API should care about proper encoding of non-latin1 characters
// but for now it does not
#define IDN_BROKEN_IN_MDNSRESPONDER

namespace DNSSD
{

Responder::Responder(DNSServiceRef ref,QObject *parent)
		: QObject(parent), m_ref(0), m_socket(0)
{
	setRef(ref);
}

void Responder::setRef(DNSServiceRef ref)
{
	if (m_socket || m_ref) stop();
	m_running = false;
	m_ref = ref;
	if (m_ref == 0 ) return;
#ifdef HAVE_DNSSD
	int fd = DNSServiceRefSockFD(ref);
	if (fd == -1) return;
	m_socket = new QSocketNotifier(fd,QSocketNotifier::Read,this);
	connect(m_socket,SIGNAL(activated(int)),this,SLOT(process()));
	m_running = true;
#endif
}
Responder::~Responder()
{
	stop();
}

void Responder::stop()
{
	if (m_socket) delete m_socket;
	m_socket = 0;
#ifdef HAVE_DNSSD
	if (m_ref) DNSServiceRefDeallocate(m_ref);
#endif
	m_ref = 0;
	m_running = false;
}


void Responder::process()
{
#ifdef HAVE_DNSSD
	if ( DNSServiceProcessResult(m_ref) != kDNSServiceErr_NoError) stop();
#endif
}

bool Responder::isRunning() const
{
	return m_running;
}

bool domainIsLocal(const QString& domain)
{
	return domain.section('.',-1,-1).toLower()=="local";
}

QByteArray domainToDNS(const QString &domain)
{
#ifdef IDN_BROKEN_IN_MDNSRESPONDER
	if (domainIsLocal(domain)) return domain.toUtf8();
		else return KIDNA::toAsciiCString(domain);
#else
	return domain.toUtf8();
#endif
}

QString DNSToDomain(const char* domain)
{
#ifdef IDN_BROKEN_IN_MDNSRESPONDER
	if (domainIsLocal(domain)) return QString::fromUtf8(domain);
		else return KIDNA::toUnicode(domain);
#else
	return QString::fromUtf8(domain);
#endif
}


}
#include "responder.moc"
