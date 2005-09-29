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

#ifndef DNSSDRESPONDER_H
#define DNSSDRESPONDER_H

#include <qobject.h>
#include <qsocketnotifier.h>
#include <qsignal.h>
#include <config.h>
#ifdef HAVE_DNSSD
#include <dns_sd.h>
#else
#define DNSServiceRef void*
#endif

namespace DNSSD
{

/**
This class should not be used directly.
 
@author Jakub Stachowski
@short Internal class wrapping dns_sd.h interface
 */
class Responder : public QObject
{
	Q_OBJECT

public:
	Responder(DNSServiceRef ref=0,QObject *parent = 0, const char *name = 0);

	~Responder();

	/**
	Returns true if it is possible to use mDNS service publishing and discovery. 
	It needs mDNSResponder running.
	 */
	bool isRunning() const;
	void setRef(DNSServiceRef ref);
	void stop();
public slots:
	void process();
protected:
	DNSServiceRef m_ref;
	bool m_running;
	QSocketNotifier *m_socket;
};

/* Utils functions */

bool domainIsLocal(const QString& domain);
// Encodes domain name using utf8() or IDN 
QCString domainToDNS(const QString &domain);
QString DNSToDomain(const char* domain);

}

#endif
