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
 
#ifndef _KSSLX509V3_H
#define _KSSLX509V3_H

#include <qstring.h>

#include <kdelibs_export.h>

// ### KDE4: fix constness of methods
/**
 * KDE X509v3 Flag Class
 *
 * This class is used to track X509v3 flags for 
 *
 * @author George Staikos <staikos@kde.org>
 * @see KExtendedSocket, TCPSlaveBase
 * @short KDE SSL Class
 */
class KIO_EXPORT KSSLX509V3 {
friend class KSSLCertificate;
friend class KSSLCertificatePrivate;
public:
	/**
	 *  Destroy this object
	 */
	~KSSLX509V3();

	/**
	 *  Determine if this certificate can be used by a certificate authority
	 *  @return true if it can be used by a certificate authority
	 */
	bool certTypeCA() const;

	/**
	 *  Determine if this certificate can be used by an SSL signer
	 *  @return true if it can be used by an SSL signer
	 */
	bool certTypeSSLCA() const;

	/**
	 *  Determine if this certificate can be used by an S/MIME signer
	 *  @return true if it can be used by an S/MIME signer
	 */
	bool certTypeEmailCA() const;

	/**
	 *  Determine if this certificate can be used by a code certificate signer
	 *  @return true if it can be used by a code certificate signer
	 */
	bool certTypeCodeCA() const;

	/**
	 *  Determine if this certificate can be used by an SSL client
	 *  @return true if it can be used by an SSL client
	 */
	bool certTypeSSLClient() const;

	/**
	 *  Determine if this certificate can be used by an SSL server
	 *  @return true if it can be used by an SSL server
	 */
	bool certTypeSSLServer() const;

	/**
	 *  Determine if this certificate can be used by a Netscape SSL server
	 *  @return true if it can be used by a Netscape SSL server
	 */
	bool certTypeNSSSLServer() const;

	/**
	 *  Determine if this certificate can be used for S/MIME
	 *  @return true if it can be used for S/MIME
	 */
	bool certTypeSMIME() const;

	/**
	 *  Determine if this certificate can be used for S/MIME encryption
	 *  @return true if it can be used for S/MIME encryption
	 */
	bool certTypeSMIMEEncrypt() const;

	/**
	 *  Determine if this certificate can be used for S/MIME signing
	 *  @return true if it can be used for S/MIME signing
	 */
	bool certTypeSMIMESign() const;

	/**
	 *  Determine if this certificate can be used for revocation signing
	 *  @return true if it can be used for revocation signing
	 */
	bool certTypeCRLSign() const;

private:
	class KSSLX509V3Private;
	KSSLX509V3Private *d;

protected:
	KSSLX509V3();
	long flags;  // warning: this is only valid for up to 16 flags + 16 CA.
};

#endif
