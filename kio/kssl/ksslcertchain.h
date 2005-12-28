/* This file is part of the KDE project
 *
 * Copyright (C) 2001-2003 George Staikos <staikos@kde.org>
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

#ifndef _KSSLCERTCHAIN_H
#define _KSSLCERTCHAIN_H

#include <qglobal.h>
#include <q3ptrlist.h>
#include <kdemacros.h>

class QString;
class KSSL;
class KSSLCertChainPrivate;
class QStringList;

#include <ksslcertificate.h>

/**
 * KDE Certificate Chain Representation Class
 *
 * This class provides a representation for an X.509 certificate chain.
 *
 * @author George Staikos <staikos@kde.org>
 * @see KSSL, KSSLCertificate, KSSLPeerInfo
 * @short KDE X.509 Certificate Chain
 */
class KIO_EXPORT KSSLCertChain {
friend class KSSL;
friend class KSSLPeerInfo;

public:
	/**
	 *  Construct a KSSLCertChain object
	 */
	KSSLCertChain();

	/**
	 *  Destroy this KSSLCertChain object
	 */
	~KSSLCertChain();

	/**
	 *  Determine if this represents a valid certificate chain
	 *
	 *  @return true if it is a valid certificate chain
	 */
	bool isValid();

	/**
	 *  Do a deep copy of the certificate chain.
	 *
	 *  @return pointer to a new certificate chain object
	 *
	 *  This is an expensive operation, and you are responsible for deleting
	 *  the returned object yourself.
	 */
	KSSLCertChain *replicate();

	/**
	 *  Set the raw chain from OpenSSL
	 *  @internal
	 */
	void setChain(void *stack_of_x509);

	/**
	 *  Set the certificate chain as a pointer list of KSSL certificates.
	 *
	 *  @param chain the certificate chain
	 *  @see KSSLCertificate
	 */
	void setChain(Q3PtrList<KSSLCertificate>& chain);

	/**
	 *  Set the certificate chain as a list of base64 encoded X.509
	 *  certificates.
	 *
	 *  @param chain the certificate chain
	 *  @deprecated
	 */
	void setChain(QStringList chain) KDE_DEPRECATED;

	/**
	 *  Set the certificate chain as a list of base64 encoded X.509
	 *  certificates.
	 *
	 *  @param chain the certificate chain
	 */
	void setCertChain(const QStringList& chain);

	/**
	 *  Obtain a copy of the certificate chain.
	 *
	 *  @return a deep copy of the certificate chain.
	 *  @see KSSLCertificate
	 */
	Q3PtrList<KSSLCertificate> getChain();

	/**
	 *  Determine the number of entries (depth) of the chain.
	 *
	 *  @return the number of entries in the certificate chain
	 */
	int depth();

	/**
	 *  Read the raw chain in OpenSSL format
	 *  @internal
	 */
	void *rawChain() { return _chain; }

private:
	KSSLCertChainPrivate *d;
	void *_chain;
};


#endif

