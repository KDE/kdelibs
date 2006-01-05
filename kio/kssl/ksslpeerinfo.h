/* This file is part of the KDE project
 *
 * Copyright (C) 2000-2003 George Staikos <staikos@kde.org>
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

#ifndef _KSSLPEERINFO_H
#define _KSSLPEERINFO_H

class KSSL;

#include <qglobal.h>
#include <qstringlist.h>
#include <ksslcertificate.h>

class KSSLPeerInfoPrivate;
class KInetSocketAddress;

/**
 * KDE SSL Peer Data
 *
 * This class contains data about the peer of an SSL connection.
 *
 * @author George Staikos <staikos@kde.org>
 * @see KSSL
 * @short KDE SSL Peer Data
 */
class KIO_EXPORT KSSLPeerInfo {
friend class KSSL;
public:
	/**
	 *  Destroy this instance
	 */
	~KSSLPeerInfo();

	/**
	 *  Get a reference to the peer's certificate
	 *
	 *  @return a reference to the peer's certificate
	 *  @see KSSLCertificate
	 */
	KSSLCertificate& getPeerCertificate();

	/**
	 *  Determine if the peer's certificate matches the address set with
	 *  setPeerHost().  Note that this is a match in the "https"
	 *  sense, taking into account, for instance,  wildcards.
	 * 
	 *  @return true if it matches
	 *  @see setPeerHost
	 */
	bool certMatchesAddress();

	/**
	 *  Determine if the given "common name" matches the address set with
	 *  setPeerHost().  Note that this is a match in the "https"
	 *  sense, taking into account, for instance,  wildcards.
	 * 
	 *  @return true if it matches
	 *  @see setPeerHost
	 */
	bool cnMatchesAddress(QString cn);

	/**
	 *  Set the host that we are connected to.  This is generally set by
	 *  KSSL, and should be exactly what the user -thinks- he is connected
	 *  to.  (for instance, the host name in the url)
	 *
	 *  @param host the hostname
	 */
	void setPeerHost(const QString &host = QString());

	/**
	 *  Returns the host we are connected to.
	 */
	const QString& peerHost() const;

	/**
	 *  Clear out the host name.
	 */
	void reset();

protected:
	KSSLPeerInfo();

	KSSLCertificate m_cert;

private:
	KSSLPeerInfoPrivate *d;
};


#endif

