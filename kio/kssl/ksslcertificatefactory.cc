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

#include <ksslcertificatefactory.h>
#include <ksslcertificate.h>
#include <stdlib.h>

//#include <kopenssl.h>

KSSLCertificate*
KSSLCertificateFactory::generateSelfSigned(KSSLKeyType /*keytype*/) {
#if 0
  //#ifdef KSSL_HAVE_SSL
  X509_NAME *x509name = X509_NAME_new();
  X509      *x509;
  ASN1_UTCTIME *beforeafter;
  KSSLCertificate *newcert;
  int rc;

  // FIXME: generate the private key
  if (keytype == KEYTYPE_UNKNOWN || (key=EVP_PKEY_new()) == NULL) {
    X509_NAME_free(x509name);
    return NULL;
  }

  switch(keytype) {
  case KEYTYPE_RSA:
    if (!EVP_PKEY_assign_RSA(key, RSA_generate_key(newkey,0x10001,
						   req_cb,bio_err))) {
      
    } 
    break;
  case KEYTYPE_DSA:
    if (!DSA_generate_key(dsa_params)) goto end;
    if (!EVP_PKEY_assign_DSA(pkey,dsa_params)) goto end;
    dsa_params=NULL; 
    if (pkey->type == EVP_PKEY_DSA)
      digest=EVP_dss1();
    break;
  }

  // FIXME: dn doesn't exist
  // FIXME: allow the notAfter value to be parameterized
  // FIXME: allow a password to lock the key with

  // Fill in the certificate
  X509_NAME_add_entry_by_NID(x509name, OBJ_txt2nid("CN"), 0x1001,
                             (unsigned char *) dn, -1, -1, 0);

  x509 = X509_new();
  rc = X509_set_issuer_name(x509, x509name);
  if (rc != 0) {
    X509_free(x509);
    X509_NAME_free(x509name);
    return NULL;
  }
  rc = X509_set_subject_name(x509, x509name);
  if (rc != 0) {
    X509_free(x509);
    X509_NAME_free(x509name);
    return NULL;
  }
  ASN1_INTEGER_set(X509_get_serialNumber(*x509), 0);

  X509_NAME_free(x509name);

  // Make it a 1 year certificate
  beforeafter = ASN1_UTCTIME_new();
  if (!X509_gmtime_adj(beforeafter, -60*60*24)) {     // yesterday
    X509_free(x509);
    return NULL;
  }
  if (!X509_set_notBefore(x509, beforeafter)) {
    X509_free(x509);
    return NULL;
  }
  if (!X509_gmtime_adj(beforeafter, 60*60*24*364)) {  // a year from yesterday
    X509_free(x509);
    return NULL;
  }
  if (!X509_set_notAfter(x509, beforeafter)) {
    X509_free(x509);
    return NULL;
  }
  ASN1_UTCTIME_free(beforeafter);

  if (!X509_set_pubkey(x509, key)) {
    X509_free(x509);
    return NULL;
  }

  rc = X509_sign(x509, key, EVP_sha1());
  if (rc != 0) {
    X509_free(x509);
    return NULL;
  }

  newCert = new KSSLCertificate;
  newCert->setCert(x509);
  return newCert;  
#else
  return NULL;
#endif
}

