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

#include <config.h>

#ifdef HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/pkcs12.h>
#undef crypt
#else
class PKCS12;
class EVP_PKEY;
class X509;
#endif

#include <kopenssl.h>

#ifndef STACK_OF
#define STACK_OF(x) void
#endif

class KSSLCertificate;
class KSSLPKCS12Private;


class KSSLPKCS12 {
public:
  virtual ~KSSLPKCS12();

  /*
   *   Create a KSSLPKCS12 object by reading a PKCS#12 file.  Returns NULL
   *   on failure.
   */
  static KSSLPKCS12* loadCertFile(QString filename, QString password = "");

  /*
   *   Raw set the PKCS12 object.
   */
  void setCert(PKCS12 *c);

  /*
   *   Change the password of the PKCS#12 in memory.  Returns true on success.
   */
  bool changePassword(QString pold, QString pnew);
 

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

