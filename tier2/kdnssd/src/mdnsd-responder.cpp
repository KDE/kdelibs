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

#include "mdnsd-responder.h"
#include "servicebase.h"
#include <QUrl>
#include <QCoreApplication>

namespace KDNSSD
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
	int fd = DNSServiceRefSockFD(ref);
	if (fd == -1) return;
	m_socket = new QSocketNotifier(fd,QSocketNotifier::Read,this);
	connect(m_socket,SIGNAL(activated(int)),this,SLOT(process()));
	m_running = true;
}
Responder::~Responder()
{
	stop();
}

void Responder::stop()
{
	delete m_socket;
	m_socket = 0;
	if (m_ref) DNSServiceRefDeallocate(m_ref);
	m_ref = 0;
	m_running = false;
}


void Responder::process()
{
	if ( DNSServiceProcessResult(m_ref) != kDNSServiceErr_NoError) stop();
}

bool Responder::isRunning() const
{
	return m_running;
}

QByteArray domainToDNS(const QString &domain)
{
	if (domainIsLocal(domain)) return domain.toUtf8();
		else return QUrl::toAce(domain);
}

QString DNSToDomain(const char* domain)
{
	if (domainIsLocal(domain)) return QString::fromUtf8(domain);
		else return QUrl::fromAce(domain);
}

}
