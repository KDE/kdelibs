/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
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

// As of yet, this class is being defined.  if you use it, let it be known
// that BC will break on you until this message is removed.

#ifndef _KSSLPKCS7_H
#define _KSSLPKCS7_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/pkcs7.h>
#undef crypt
#else
class PKCS7;
class X509;
#endif

//#include <kopenssl.h>
#include <ksslcertificate.h>
#include <ksslcertchain.h>

#ifndef STACK_OF
#define STACK_OF(x) void
#endif

class KSSL;
class KSSLPKCS7Private;
class KOpenSSLProxy;


class KSSLPKCS7 {
friend class KSSL;

public:
  virtual ~KSSLPKCS7();

  /*
   *   The name of this certificate.  This can be used to refer to the
   *   certificate instead of passing the object itself.
   */
  QString name();

  /*
   *   Create a KSSLPKCS7 object from a Base64 in a QString.  Returns NULL
   *   on failure.
   */
  static KSSLPKCS7* fromString(QString base64);

  /*
   *   Create a KSSLPKCS7 object by reading a PKCS#7 file.  Returns NULL
   *   on failure.
   */
  static KSSLPKCS7* loadCertFile(QString filename);

  /*
   *   Convert to a Base64 string.
   */
  QString toString();

  /*
   *   Raw set the PKCS7 object.
   */
  void setCert(PKCS7 *c);

  /*
   *   Get the bottom level X.509 certificate
   */
  KSSLCertificate *getCertificate();


  /*
   *   Get the certificate chain
   */
  KSSLCertChain *getChain();


  /*
   *   Write the PKCS#7 to a file in raw mode
   */
  bool toFile(QString filename);


  /*
   *   Check the chain to make sure it's valid.
   */
  KSSLCertificate::KSSLValidation validate();
  KSSLCertificate::KSSLValidation validate(KSSLCertificate::KSSLPurpose p);


  /*
   *   Check the chain to make sure it's valid.
   *   Ignore any cached validation result.
   */
  KSSLCertificate::KSSLValidation revalidate();
  KSSLCertificate::KSSLValidation revalidate(KSSLCertificate::KSSLPurpose p);


  /*
   *   Return true if the chain is valid.
   */
  bool isValid();



protected:
  KSSLPKCS7();

private:
  KSSLPKCS7Private *d;
  PKCS7 *_pkcs;
  KOpenSSLProxy *kossl;
  KSSLCertificate *_cert;
  KSSLCertChain *_chain;
};


#endif

