/* This file is part of the KDE project
 *
 * Copyright (C) 2000 George Staikos <staikos@kde.org>
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

#ifndef _KSSLCERTIFICATE_H
#define _KSSLCERTIFICATE_H

// UPDATE: I like the structure of this class less and less every time I look
//         at it.  I think it needs to change.
//

//
//  The biggest reason for making everything protected here is so that
//  the class can have all it's methods available even if openssl is not
//  available.  Also, to create a new certificate you should use the
//  KSSLCertificateFactory, and to manage the user's database of certificates,
//  you should go through the KSSLCertificateHome.
//
//  There should be no reason to touch the X509 stuff directly.
//

#include <qstring.h>

class KSSL;
class KSSLCertificatePrivate;
 
typedef struct x509_st X509;

class KSSLCertificate {
friend class KSSL;
friend class KSSLCertificateHome;
friend class KSSLCertificateFactory;
friend class KSSLPeerInfo;

public:
  ~KSSLCertificate();

  enum KSSLValidation { Unknown, Ok, NoCARoot, InvalidPurpose,
                        PathLengthExceeded, InvalidCA, Expired,
                        SelfSigned, ErrorReadingRoot, NoSSL,
                        Revoked, Untrusted, SignatureFailed,
                        Rejected };

  QString getSubject() const;
  QString getIssuer() const;
  QString getNotBefore() const;
  QString getNotAfter() const;
  // getSerialNumber() const;
  // getSignatureType() const;
  // get public key ??
  bool isValid();
  KSSLValidation validate();
  KSSLValidation revalidate();
  

private:
  KSSLCertificatePrivate *d;


protected:
  KSSLCertificate();

  void setCert(X509 *c);
  KSSLValidation processError(int ec);
};


#endif

