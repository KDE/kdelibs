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

#ifndef _KSSLPKCS12_H
#define _KSSLPKCS12_H

#include "ksslconfig.h"

#ifdef KSSL_HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/pkcs12.h>
#undef crypt
#else
class PKCS12;
class EVP_PKEY;
class X509;
#endif

//#include <kopenssl.h>
#include <ksslcertificate.h>
#include <ksslcertchain.h>

#ifndef STACK_OF
#define STACK_OF(x) void
#endif

class KSSL;
class KSSLPKCS12Private;
class KOpenSSLProxy;


class KSSLPKCS12 {
friend class KSSL;

public:
  virtual ~KSSLPKCS12();

  /*
   *   The name of this certificate.  This can be used to refer to the
   *   certificate instead of passing the object itself.
   */
  QString name();

  /*
   *   Create a KSSLPKCS12 object from a Base64 in a QString.  Returns NULL
   *   on failure.
   */
  static KSSLPKCS12* fromString(QString base64, QString password = "");

  /*
   *   Create a KSSLPKCS12 object by reading a PKCS#12 file.  Returns NULL
   *   on failure.
   */
  static KSSLPKCS12* loadCertFile(QString filename, QString password = "");

  /*
   *   Convert to a Base64 string.
   */
  QString toString();

  /*
   *   Raw set the PKCS12 object.
   */
  void setCert(PKCS12 *c);

  /*
   *   Change the password of the PKCS#12 in memory.  Returns true on success.
   */
  bool changePassword(QString pold, QString pnew);
 

  /*
   *   Get the private key
   */
  EVP_PKEY *getPrivateKey();


  /*
   *   Get the X.509 certificate
   */
  KSSLCertificate *getCertificate();


  /*
   *   Write the PKCS#12 to a file in raw mode
   */
  bool toFile(QString filename);


  /*
   *   Check the X.509 and private key to make sure they're valid.
   */
  KSSLCertificate::KSSLValidation validate();
  KSSLCertificate::KSSLValidation validate(KSSLCertificate::KSSLPurpose p);


  /*
   *   Check the X.509 and private key to make sure they're valid.
   *   Ignore any cached validation result.
   */
  KSSLCertificate::KSSLValidation revalidate();
  KSSLCertificate::KSSLValidation revalidate(KSSLCertificate::KSSLPurpose p);


  /*
   *   Return true if the X.509 and private key are valid.
   */
  bool isValid();
  bool isValid(KSSLCertificate::KSSLPurpose p);



protected:
  KSSLPKCS12();

  /*
   *   Parse the PKCS#12
   */
  bool parse(QString pass);

private:
  KSSLPKCS12Private *d;
  PKCS12 *_pkcs;
  KOpenSSLProxy *kossl;
  EVP_PKEY *_pkey;
  KSSLCertificate *_cert;
  STACK_OF(X509) *_caStack;
};


#endif

