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
#include <unistd.h>
#include <qstring.h>

#include "kssldefs.h"
#include "ksslcertificate.h"
#include "ksslcertchain.h"
#include "ksslutils.h"

#include <kstddirs.h>
#include <kmdcodec.h>
#include <klocale.h>
#include <qdatetime.h>
#include <ktempfile.h>

#include <sys/types.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

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

#include <kopenssl.h>
#include <qcstring.h>
#include <kdebug.h>



class KSSLCertificatePrivate {
public:
  KSSLCertificatePrivate() {
     kossl = KOSSL::self();
  }

  ~KSSLCertificatePrivate() {
  }

  KSSLCertificate::KSSLValidation m_stateCache;
  bool m_stateCached;
  #ifdef HAVE_SSL
    X509 *m_cert;
  #endif
  KOSSL *kossl;
  KSSLCertChain _chain;
};

KSSLCertificate::KSSLCertificate() {
  d = new KSSLCertificatePrivate;
  d->m_stateCached = false;
  KGlobal::dirs()->addResourceType("kssl", "share/apps/kssl");
#ifdef HAVE_SSL
  d->m_cert = NULL;
#endif
}


KSSLCertificate::~KSSLCertificate() {
#ifdef HAVE_SSL
  if (d->m_cert)
    d->kossl->X509_free(d->m_cert);
#endif
  delete d;
}


KSSLCertChain& KSSLCertificate::chain() {
  return d->_chain;
}


KSSLCertificate *KSSLCertificate::fromString(QCString cert) {
KSSLCertificate *n = NULL;
#ifdef HAVE_SSL
    if (cert.length() == 0) return NULL;
    QByteArray qba, qbb = cert.copy();
    KCodecs::base64Decode(qbb, qba);
    unsigned char *qbap = reinterpret_cast<unsigned char *>(qba.data());
    X509 *x5c = KOSSL::self()->d2i_X509(NULL, &qbap, qba.size());
    if (!x5c) {
       return NULL;
    }
 
    n = new KSSLCertificate;
    n->setCert(x5c);
#endif
    return n;
}



QString KSSLCertificate::getSubject() const {
QString rc = "";

#ifdef HAVE_SSL
  char *t = d->kossl->X509_NAME_oneline(d->kossl->X509_get_subject_name(d->m_cert), 0, 0);
  if (!t) return rc;
  rc = t;
  d->kossl->OPENSSL_free(t);
#endif
return rc;
}


QString KSSLCertificate::getIssuer() const {
QString rc = "";

#ifdef HAVE_SSL
  char *t = d->kossl->X509_NAME_oneline(d->kossl->X509_get_issuer_name(d->m_cert), 0, 0);
  if (!t) return rc;
  rc = t;
  d->kossl->OPENSSL_free(t);
#endif

return rc;
}

void KSSLCertificate::setChain(void *c) {
#ifdef HAVE_SSL
  d->_chain.setChain(c);
#endif
  d->m_stateCached = false;
  d->m_stateCache = KSSLCertificate::Unknown;
}

void KSSLCertificate::setCert(X509 *c) {
#ifdef HAVE_SSL
  d->m_cert = c;
#endif
  d->m_stateCached = false;
  d->m_stateCache = KSSLCertificate::Unknown;
}

X509 *KSSLCertificate::getCert() {
#ifdef HAVE_SSL
  return d->m_cert;
#endif
return 0;
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

// CRL files?  we don't do that yet

// This is for verifying certificate FILES (.pem), not remote presentations
// of certificates.

KSSLCertificate::KSSLValidation KSSLCertificate::validate() {

#ifdef HAVE_SSL
  X509_STORE *certStore;
  X509_LOOKUP *certLookup;
  X509_STORE_CTX *certStoreCTX;
  int rc = 0;

  if (!d->m_cert) return KSSLCertificate::Unknown;

  if (d->m_stateCached) {
    // kdDebug(7029) << "KSSL returning a cached value" << d->m_stateCached << endl;
    return d->m_stateCache;
  }

  QStringList qsl = KGlobal::dirs()->resourceDirs("kssl");

  if (qsl.isEmpty()) {
    return KSSLCertificate::NoCARoot;
  }

  KSSLCertificate::KSSLValidation ksslv = Unknown;

  for (QValueListIterator<QString> j = qsl.begin();
                                   j != qsl.end(); ++j) {
    struct stat sb;
    QString _j = (*j)+"caroot/ca-bundle.crt";
    if (-1 == stat(_j.ascii(), &sb)) continue;
    //kdDebug(7029) << "KSSL Certificate Root directory found: " << _j << endl;

    certStore = d->kossl->X509_STORE_new();
    if (!certStore)
      return KSSLCertificate::Unknown;

    X509_STORE_set_verify_cb_func(certStore, X509Callback);

    certLookup = d->kossl->X509_STORE_add_lookup(certStore, d->kossl->X509_LOOKUP_file());
    if (!certLookup) {
      // kdDebug(7029) << "KSSL error adding lookup file" << endl;
      ksslv = KSSLCertificate::Unknown;
      d->kossl->X509_STORE_free(certStore);
      continue;
    }

    //kdDebug(7029) << "KSSL about to load file" << endl;
    if (!d->kossl->X509_LOOKUP_load_file(certLookup, _j.ascii(), X509_FILETYPE_PEM)) {
      // error accessing directory and loading pems
      kdDebug(7029) << "KSSL couldn't read CA root: " << _j << endl;
      ksslv = KSSLCertificate::ErrorReadingRoot;
      d->kossl->X509_STORE_free(certStore);
      continue;
    }

    //
    // This is the checking code
    certStoreCTX = d->kossl->X509_STORE_CTX_new();

    // this is a bad error - could mean no free memory.  This may be the
    // wrong thing to do here
    if (!certStoreCTX) {
      kdDebug(7029) << "KSSL couldn't create an X509 store context." << endl;
      d->kossl->X509_STORE_free(certStore);
      continue;
    }

    //kdDebug(7029) << "KSSL Initializing the certificate store context" << endl;
    d->kossl->X509_STORE_CTX_init(certStoreCTX, certStore, d->m_cert, NULL);
    if (d->_chain.isValid())
      d->kossl->X509_STORE_CTX_set_chain(certStoreCTX, (STACK_OF(X509)*)d->_chain.rawChain());

    // FIXME: do all the X509_STORE_CTX_set_flags(); here
    //   +----->  Note that this is for 0.9.6 or better ONLY!

    d->kossl->X509_STORE_CTX_set_purpose(certStoreCTX, X509_PURPOSE_SSL_SERVER);

    //kdDebug(7029) << "KSSL verifying.............." << endl;
    certStoreCTX->error = X509_V_OK;
    rc = d->kossl->X509_verify_cert(certStoreCTX);
    int errcode = certStoreCTX->error;
    //kdDebug(7029) << "KSSL freeing" << endl;
    d->kossl->X509_STORE_CTX_free(certStoreCTX);
    d->kossl->X509_STORE_free(certStore);
    // end of checking code
    //

    ksslv = processError(errcode);
    //kdDebug(7029) << "KSSL Validation procedure RC: " << rc << endl;
    //kdDebug(7029) << "KSSL Validation procedure errcode: " << errcode << endl;
    //kdDebug(7029) << "KSSL Validation procedure RESULTS: " << ksslv << endl;

    if (ksslv != NoCARoot && ksslv != InvalidCA) {
      d->m_stateCached = true;
      d->m_stateCache = ksslv;
      break;
    }
  }

  return (d->m_stateCache);

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
 kdDebug(7029) << "KSSL apparently this is expired.  Not after: "
           << getNotAfter() << endl;
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


QDateTime KSSLCertificate::getQDTNotBefore() const {
#ifdef HAVE_SSL
return ASN1_UTCTIME_QDateTime(X509_get_notBefore(d->m_cert), NULL);
#else
return QDateTime::currentDateTime();
#endif
}


QDateTime KSSLCertificate::getQDTNotAfter() const {
#ifdef HAVE_SSL
return ASN1_UTCTIME_QDateTime(X509_get_notAfter(d->m_cert), NULL);
#else
return QDateTime::currentDateTime();
#endif
}


int operator==(KSSLCertificate &x, KSSLCertificate &y) {
#ifndef HAVE_SSL
  return 1;
#else
  if (!KOSSL::self()->X509_cmp(x.getCert(), y.getCert())) return 1;
  return 0;
#endif
}


KSSLCertificate::KSSLCertificate(const KSSLCertificate& x) {
  d = new KSSLCertificatePrivate;
  d->m_stateCached = false;
  KGlobal::dirs()->addResourceType("kssl", "share/apps/kssl");
#ifdef HAVE_SSL
  d->m_cert = NULL;
  setCert(KOSSL::self()->X509_dup(const_cast<KSSLCertificate&>(x).getCert()));
  KSSLCertChain *c = x.d->_chain.replicate();
  setChain(c->rawChain());
  delete c;
#endif
}


KSSLCertificate *KSSLCertificate::replicate() {
  // The new certificate doesn't have the cached value.  It's probably
  // better this way.  We can't anticipate every reason for doing this.
  KSSLCertificate *newOne = new KSSLCertificate();
  #ifdef HAVE_SSL
  newOne->setCert(d->kossl->X509_dup(getCert()));
  KSSLCertChain *c = d->_chain.replicate();
  newOne->setChain(c->rawChain());
  delete c;
  #endif
  return newOne;
}


QString KSSLCertificate::toString() {
return KCodecs::base64Encode(toDer());
}


QString KSSLCertificate::verifyText(KSSLValidation x) {
  switch (x) {
  case KSSLCertificate::Ok:
     return i18n("The certificate is valid.");
  case KSSLCertificate::PathLengthExceeded:
  case KSSLCertificate::ErrorReadingRoot:
  case KSSLCertificate::NoCARoot:
     return i18n("Certificate signing authority root files could not be found so the certificate is not verified.");
  case KSSLCertificate::InvalidCA:
     return i18n("Certificate signing authority is unknown or invalid.");
  case KSSLCertificate::SelfSigned:
     return i18n("Certificate is self-signed and thus may not be trustworthy.");
  case KSSLCertificate::Expired:
     return i18n("Certificate has expired.");
  case KSSLCertificate::Revoked:
     return i18n("Certificate has been revoked.");
  case KSSLCertificate::NoSSL:
     return i18n("SSL support was not found.");
  case KSSLCertificate::Untrusted:
  case KSSLCertificate::SignatureFailed:
  case KSSLCertificate::Rejected:
  case KSSLCertificate::InvalidPurpose:
  break;
  }

return i18n("The certificate is invalid.");
}


QByteArray KSSLCertificate::toDer() {
QByteArray qba;
#ifdef HAVE_SSL
      unsigned int certlen = d->kossl->i2d_X509(getCert(), NULL);
      // These should technically be unsigned char * but it doesn't matter
      // for our purposes
      char *cert = new char[certlen];
      char *p = cert;
      // FIXME: return code!
      d->kossl->i2d_X509(getCert(), (unsigned char **)&p);
 
      // encode it into a QString
      qba.duplicate(cert, certlen);
      delete[] cert;
#endif
return qba;
}



QByteArray KSSLCertificate::toPem() {
QByteArray qba;
QString thecert = toString();
const char *header = "-----BEGIN CERTIFICATE-----\n";
const char *footer = "-----END CERTIFICATE-----\n";

   // We just do base64 on the ASN1
   //  64 character lines  (unpadded)
   unsigned int xx = thecert.length()-1;
   for (unsigned int i = 0; i < xx/64; i++) {
      thecert.insert(64*(i+1)+i, '\n');
   }
   thecert.prepend(header);
   if (thecert[thecert.length()-1] != '\n')
      thecert += "\n";
   thecert.append(footer);

   qba.duplicate(thecert.local8Bit(), thecert.length());
return qba;
}


#define NETSCAPE_CERT_HDR     "certificate"

// what a piece of crap this is
QByteArray KSSLCertificate::toNetscape() {
QByteArray qba;
#ifdef HAVE_SSL
      ASN1_HEADER ah;
      ASN1_OCTET_STRING os;
      KTempFile ktf;
 
      os.data=(unsigned char *)NETSCAPE_CERT_HDR;
      os.length=strlen(NETSCAPE_CERT_HDR);
      ah.header= &os;
      ah.data=(char *)getCert();
      ah.meth=d->kossl->X509_asn1_meth();
 
      d->kossl->ASN1_i2d_fp(ktf.fstream(),(unsigned char *)&ah);

      ktf.close();
      QFile qf(ktf.name());
      qf.open(IO_ReadOnly);
      char *buf = new char[qf.size()];
      qf.readBlock(buf, qf.size());
      qba.duplicate(buf, qf.size());
      qf.close();
      delete[] buf;

      ktf.unlink();

#endif
return qba;
}



QString KSSLCertificate::toText() {
QString text;
#ifdef HAVE_SSL
KTempFile ktf;

   d->kossl->X509_print(ktf.fstream(), getCert());
   ktf.close();
   QFile qf(ktf.name());
   qf.open(IO_ReadOnly);
   char *buf = new char[qf.size()+1];
   qf.readBlock(buf, qf.size());
   buf[qf.size()] = 0;
   text = buf;
   delete[] buf;
   qf.close();
   ktf.unlink();
#endif
return text;
}



bool KSSLCertificate::setCert(QString& cert) {
#ifdef HAVE_SSL
    QByteArray qba, qbb = cert.local8Bit().copy();
    KCodecs::base64Decode(qbb, qba);
    unsigned char *qbap = reinterpret_cast<unsigned char *>(qba.data());
    X509 *x5c = KOSSL::self()->d2i_X509(NULL, &qbap, qba.size());
    if (x5c) {
       setCert(x5c);
       return true;
    }
#endif
return false;
}


QDataStream& operator<<(QDataStream& s, const KSSLCertificate& r) {
QStringList qsl;
QList<KSSLCertificate> cl = const_cast<KSSLCertificate&>(r).chain().getChain();

      for (KSSLCertificate *c = cl.first(); c != 0; c = cl.next()) {
         //kdDebug() << "Certificate in chain: " <<  c->toString() << endl;
         qsl << c->toString();
      }

      cl.setAutoDelete(true);

s << const_cast<KSSLCertificate&>(r).toString() << qsl;

return s;
}


QDataStream& operator>>(QDataStream& s, KSSLCertificate& r) {
QStringList qsl;
QString cert;

s >> cert >> qsl;

       if (r.setCert(cert) && !qsl.isEmpty())
          r.chain().setChain(qsl);

return s;
}




