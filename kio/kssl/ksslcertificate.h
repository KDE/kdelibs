/* This file is part of the KDE project
 *
 * Copyright (C) 2000,2001 George Staikos <staikos@kde.org>
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


// Note: this object is VERY HEAVY TO COPY.  Please try to use reference
//       or ptr whenever possible

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

#include <qcstring.h>

class QString;
class QCString;
class KSSL;
class KSSLCertificatePrivate;
class QDateTime;
class KSSLCertChain;
class KSSLX509V3;

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SSL
typedef struct x509_st X509;
#else
class X509;
#endif

class KSSLCertificate {
friend class KSSL;
friend class KSSLCertificateHome;
friend class KSSLCertificateFactory;
friend class KSSLCertificateCache;
friend class KSSLCertChain;
friend class KSSLPeerInfo;
friend class KSSLPKCS12;
friend class KSSLD;

public:
  ~KSSLCertificate();
  static KSSLCertificate *fromString(QCString cert);
  // This one duplicates the X509 object for itself.
  static KSSLCertificate *fromX509(X509 *x5);

  // We need a static method to generate one of these, given a filename that
  // points to a PEM encoded certificate file.  It will return NULL on failure.
  // (that's why I don't want to use a constructor)

  enum KSSLValidation { Unknown, Ok, NoCARoot, InvalidPurpose,
                        PathLengthExceeded, InvalidCA, Expired,
                        SelfSigned, ErrorReadingRoot, NoSSL,
                        Revoked, Untrusted, SignatureFailed,
                        Rejected, PrivateKeyFailed };

  enum KSSLPurpose { None=0, SSLServer=1, SSLClient=2, 
		     SMIMESign=4, SMIMEEncrypt=8, Any=0xffff };

  QString toString();

  QString getSubject() const;

  QString getIssuer() const;

  QString getNotBefore() const;
  QString getNotAfter() const;

  QDateTime getQDTNotBefore() const;
  QDateTime getQDTNotAfter() const;

  QByteArray toDer();
  QByteArray toPem();
  QByteArray toNetscape();
  QString    toText();

  QString getSerialNumber() const;
  QString getKeyType() const;
  QString getPublicKeyText() const;
  QString getMD5DigestText() const;
  QString getSignatureText() const;

  bool isValid();
  KSSLValidation validate();
  KSSLValidation validate(KSSLPurpose p);
  KSSLValidation revalidate();
  KSSLValidation revalidate(KSSLPurpose p);
  KSSLCertChain& chain();

  static QString verifyText(KSSLValidation x);

  KSSLCertificate *replicate();
  KSSLCertificate(const KSSLCertificate& x); // copy constructor

         friend int operator==(KSSLCertificate &x, KSSLCertificate &y);
  inline friend int operator!=(KSSLCertificate &x, KSSLCertificate &y) 
                                                       { return !(x == y); }
  bool setCert(QString& cert);

  KSSLX509V3& x509V3Extensions();

  bool isSigner();

private:
  KSSLCertificatePrivate *d;
  int purposeToOpenSSL(KSSLPurpose p) const; 


protected:
  KSSLCertificate();

  void setCert(X509 *c);
  void setChain(void *c);
  X509 *getCert();
  KSSLValidation processError(int ec);
};


QDataStream& operator<<(QDataStream& s, const KSSLCertificate& r);
QDataStream& operator>>(QDataStream& s, KSSLCertificate& r);

#endif

