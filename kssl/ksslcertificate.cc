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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kssldefs.h"
#include "ksslcertificate.h"
#include "ksslutils.h"

#include <kstddirs.h>

// this hack provided by Malte Starostik to avoid glibc/openssl bug
// on some systems
#ifdef HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/x509_vfy.h>
#include <openssl/pem.h>
#undef crypt
#endif

#include <kdebug.h>

class KSSLCertificatePrivate {
public:
  KSSLCertificatePrivate() {
  }

  ~KSSLCertificatePrivate() {
  }

  KSSLCertificate::KSSLValidation m_stateCache;
  bool m_stateCached;
  #ifdef HAVE_SSL
    X509 *m_cert;
  #endif
};

KSSLCertificate::KSSLCertificate() {
  d = new KSSLCertificatePrivate;
  d->m_stateCached = false;
#ifdef HAVE_SSL
  d->m_cert = NULL;
#endif
}


KSSLCertificate::~KSSLCertificate() {
#ifdef HAVE_SSL
  if (d->m_cert)
    X509_free(d->m_cert);
#endif
  delete d;
}


QString KSSLCertificate::getSubject() const {
QString rc = "";

#ifdef HAVE_SSL
  char *t = X509_NAME_oneline(X509_get_subject_name(d->m_cert), 0, 0);
  if (!t) return rc;
  rc = t;
  OPENSSL_free(t);
#endif
return rc;
}


QString KSSLCertificate::getIssuer() const {
QString rc = "";

#ifdef HAVE_SSL
  char *t = X509_NAME_oneline(X509_get_issuer_name(d->m_cert), 0, 0);
  if (!t) return rc;
  rc = t;
  OPENSSL_free(t);
#endif

return rc;
}

void KSSLCertificate::setCert(X509 *c) {
#ifdef HAVE_SSL
  d->m_cert = c;
#endif
  d->m_stateCached = false;
}


// pull in the callback.  It's common across multiple files but we want
// it to be hidden.

#include "ksslcallback.c"


bool KSSLCertificate::isValid() {
  return (validate() == KSSLCertificate::Ok);
}

//
// See apps/verify.c in OpenSSL for the source of most of this logic.
//

// CRL files?  what are they?  do we need to deal with them?

// This is for verifying certificate FILES (.pem), not remote presentations
// of certificates.

KSSLCertificate::KSSLValidation KSSLCertificate::validate() {

#ifdef HAVE_SSL
X509_STORE *certStore;
X509_LOOKUP *certLookup;
X509_STORE_CTX *certStoreCTX;

  if (!d->m_cert) return Unknown;

  if (d->m_stateCached) return d->m_stateCache;

  certStore = X509_STORE_new();
  if (!certStore)
    return Unknown;

  X509_STORE_set_verify_cb_func(certStore, X509Callback);

  KStandardDirs kstd;
  QStringList qsl = kstd.findDirs("data", "kssl");

  if (qsl.isEmpty()) {
    X509_STORE_free(certStore);
    return KSSLCertificate::NoCARoot;
  }

  for (QValueListIterator<QString> j = qsl.begin();
                                   j != qsl.end(); ++j) {
    QString _j = (*j)+"/caroot";
    kdDebug() << "KSSL Certificate Root directory found: " << _j << endl;
    certLookup = X509_STORE_add_lookup(certStore, X509_LOOKUP_hash_dir());
    if (!certLookup) {
      X509_STORE_free(certStore);
      return Unknown;
    }

    if (!X509_LOOKUP_add_dir(certLookup, _j.ascii(), X509_FILETYPE_PEM)) {
      // error accessing directory and loading pems
      X509_STORE_free(certStore);
      return KSSLCertificate::ErrorReadingRoot;
    }
  }

  //
  // This is the checking code
  certStoreCTX = X509_STORE_CTX_new();

  if (!certStoreCTX) {
    X509_STORE_free(certStore);
    return KSSLCertificate::Unknown;
  }

  X509_STORE_CTX_init(certStoreCTX, certStore, d->m_cert, NULL);

  // do all the X509_STORE_CTX_set_flags(); here
  //   +----->  Note that this is for 0.9.6 or better ONLY!

  int rc = X509_verify_cert(certStoreCTX);
  int errcode = certStoreCTX->error;
  X509_STORE_CTX_free(certStoreCTX);
  // end of checking code
  //

  X509_STORE_free(certStore);
  kdDebug() << "======================== RC: " << rc << endl;
  if (rc) {
    d->m_stateCached = true;
    d->m_stateCache = KSSLCertificate::Ok;
  }
  return (rc ? KSSLCertificate::Ok : processError(errcode));
#endif
return NoSSL;
}


KSSLCertificate::KSSLValidation KSSLCertificate::revalidate() {
d->m_stateCached = false;
return validate();
}


KSSLCertificate::KSSLValidation KSSLCertificate::processError(int ec) {
KSSLCertificate::KSSLValidation rc;

rc = KSSLCertificate::Unknown;
#ifdef HAVE_SSL
switch (ec) {
case X509_V_OK:       // OK
  rc = KSSLCertificate::Ok;
break;


case X509_V_ERR_CERT_REJECTED:
  rc = KSSLCertificate::Rejected;
break;


case X509_V_ERR_CERT_UNTRUSTED:
  rc = KSSLCertificate::Untrusted;
break;


case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
case X509_V_ERR_CERT_SIGNATURE_FAILURE:
case X509_V_ERR_CRL_SIGNATURE_FAILURE:
case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
  rc = KSSLCertificate::SignatureFailed;
break;

case X509_V_ERR_INVALID_CA:
case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
  rc = KSSLCertificate::InvalidCA;
break;


case X509_V_ERR_INVALID_PURPOSE:
  rc = KSSLCertificate::InvalidPurpose;
break;


case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
  rc = KSSLCertificate::SelfSigned;
break;


case X509_V_ERR_CERT_REVOKED:
  rc = KSSLCertificate::Revoked;
break;

case X509_V_ERR_PATH_LENGTH_EXCEEDED:
  rc = KSSLCertificate::PathLengthExceeded;
break;

case X509_V_ERR_CERT_NOT_YET_VALID:
case X509_V_ERR_CERT_HAS_EXPIRED:
case X509_V_ERR_CRL_NOT_YET_VALID:
case X509_V_ERR_CRL_HAS_EXPIRED:
case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
 rc = KSSLCertificate::Expired;
break;

case 1:
case X509_V_ERR_APPLICATION_VERIFICATION:
case X509_V_ERR_OUT_OF_MEM:
case X509_V_ERR_UNABLE_TO_GET_CRL:
case X509_V_ERR_CERT_CHAIN_TOO_LONG:
default:
  rc = KSSLCertificate::Unknown;
break;
}

d->m_stateCache = rc;
d->m_stateCached = true;
#endif
return rc;
}


QString KSSLCertificate::getNotBefore() const {
#ifdef HAVE_SSL
return ASN1_UTCTIME_QString(X509_get_notBefore(d->m_cert));
#else
return QString::null;
#endif
}


QString KSSLCertificate::getNotAfter() const {
#ifdef HAVE_SSL
return ASN1_UTCTIME_QString(X509_get_notAfter(d->m_cert));
#else
return QString::null;
#endif
}



