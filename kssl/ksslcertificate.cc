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
 
#include "ksslcertificate.h"

// this hack provided by Malte Starostik to avoid glibc/openssl bug
// on some systems
#ifdef HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#undef crypt
#endif


class KSSLCertificatePrivate {
public:
  KSSLCertificatePrivate() {
  }

  ~KSSLCertificatePrivate() {
  }

  #ifdef HAVE_SSL
    X509 *m_cert;
  #endif
};

KSSLCertificate::KSSLCertificate() {
  d = new KSSLCertificatePrivate;
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


#define OPENSSL_free Free

QString KSSLCertificate::getSubject() const {
QString rc = "";

#ifdef HAVE_SSL
  char *t = X509_NAME_oneline(X509_get_subject_name(d->m_cert), 0, 0);
  if (!t) return rc;
  rc = t;
  Free(t);    // NOTE - _Free_  *NOT*  _free_
#endif
return rc;
}


QString KSSLCertificate::getIssuer() const {
QString rc = "";

#ifdef HAVE_SSL
  char *t = X509_NAME_oneline(X509_get_issuer_name(d->m_cert), 0, 0);
  if (!t) return rc;
  rc = t;
  Free(t);    // NOTE - _Free_  *NOT*  _free_
#endif

return rc;
}

void KSSLCertificate::setCert(X509 *c) {
#ifdef HAVE_SSL
  d->m_cert = c;
#endif
}

