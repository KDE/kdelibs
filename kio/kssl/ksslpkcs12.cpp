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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#include <config.h>
#include <ksslconfig.h>

#include <kopenssl.h>

#include <QtCore/QString>
#include <QtCore/QFile>

#include <ksslall.h>
#include <kdebug.h>
#include <qtemporaryfile.h>

#include <assert.h>

#ifdef KSSL_HAVE_SSL
#define sk_new kossl->sk_new
#define sk_push kossl->sk_push
#define sk_free kossl->sk_free
#define sk_value kossl->sk_value
#define sk_num kossl->sk_num
#define sk_dup kossl->sk_dup
#define sk_pop kossl->sk_pop
#endif


KSSLPKCS12::KSSLPKCS12() {
   _pkcs = NULL;
   _pkey = NULL;
   _cert = NULL;
   _caStack = NULL;
   kossl = KOSSL::self();
}



KSSLPKCS12::~KSSLPKCS12() {
#ifdef KSSL_HAVE_SSL
   if (_pkey) kossl->EVP_PKEY_free(_pkey);
   if (_caStack) {
      for (;;) {
         X509* x5 = sk_X509_pop(_caStack);
         if (!x5) break;
         kossl->X509_free(x5);
      }
      sk_X509_free(_caStack);
   }
   if (_pkcs) kossl->PKCS12_free(_pkcs);
#endif
   delete _cert;
}


KSSLPKCS12* KSSLPKCS12::fromString(const QString &base64, const QString &password) {
#ifdef KSSL_HAVE_SSL
    QTemporaryFile ktf;
    ktf.open();

    if (base64.isEmpty()) return NULL;
    QByteArray qba = QByteArray::fromBase64(base64.toLatin1());
    ktf.write(qba);
    ktf.flush();
    KSSLPKCS12* rc = loadCertFile(ktf.fileName(), password);
    return rc;
#endif
return NULL;
}



KSSLPKCS12* KSSLPKCS12::loadCertFile(const QString &filename, const QString &password) {
#ifdef KSSL_HAVE_SSL
QFile qf(filename);
PKCS12 *newpkcs = NULL;

  if (!qf.open(QIODevice::ReadOnly))
    return NULL;

  FILE *fp = fdopen(qf.handle(), "r");
  if (!fp) return NULL;

  newpkcs = KOSSL::self()->d2i_PKCS12_fp(fp, &newpkcs);

  fclose(fp);
  if (!newpkcs) {
	KOSSL::self()->ERR_clear_error();
	return NULL;
  }

  KSSLPKCS12 *c = new KSSLPKCS12;
  c->setCert(newpkcs);

  // Now we parse it to see if we can decrypt it and interpret it
  if (!c->parse(password)) {
        delete c;  c = NULL;
  }

  return c;
#endif
return NULL;
}


void KSSLPKCS12::setCert(PKCS12 *c) {
#ifdef KSSL_HAVE_SSL
   _pkcs = c;
#endif
}


bool KSSLPKCS12::changePassword(const QString &pold, const QString &pnew) {
#ifdef KSSL_HAVE_SSL
   // OpenSSL makes me cast away the const here.  argh
   return (0 == kossl->PKCS12_newpass(_pkcs,
                           pold.isNull() ? (char *)"" : (char *)pold.toLatin1().constData(),
                           pnew.isNull() ? (char *)"" : (char *)pnew.toLatin1().constData()));
#endif
return false;
}


bool KSSLPKCS12::parse(const QString &pass) {
#ifdef KSSL_HAVE_SSL
X509 *x = NULL;

  assert(_pkcs);   // if you're calling this before pkcs gets set, it's a BUG!

   delete _cert;
   if (_pkey) kossl->EVP_PKEY_free(_pkey);
   if (_caStack) {
      for (;;) {
         X509* x5 = sk_X509_pop(_caStack);
         if (!x5) break;
         kossl->X509_free(x5);
      }
      sk_X509_free(_caStack);
   }
   _pkey = NULL;
   _caStack = NULL;
   _cert = NULL;

  int rc = kossl->PKCS12_parse(_pkcs, pass.toLatin1(), &_pkey, &x, &_caStack);

  if (rc == 1) {
     // kDebug(7029) << "PKCS12_parse success";
     if (x) {
        _cert = new KSSLCertificate;
        _cert->setCert(x);
        if (_caStack) {
           _cert->setChain(_caStack);
        }
        return true;
     }
  } else {
    _caStack = NULL;
    _pkey = NULL;
    kossl->ERR_clear_error();
  }
#endif
return false;
}


EVP_PKEY *KSSLPKCS12::getPrivateKey() {
   return _pkey;
}


KSSLCertificate *KSSLPKCS12::getCertificate() {
   return _cert;
}


QString KSSLPKCS12::toString()
{
   QString base64;
#ifdef KSSL_HAVE_SSL
   unsigned char *p;
   int len;

   len = kossl->i2d_PKCS12(_pkcs, NULL);
   if (len > 0) {
     char *buf = new char[len];
     p = (unsigned char *)buf;
     kossl->i2d_PKCS12(_pkcs, &p);
     base64 = QByteArray::fromRawData(buf, len).toBase64();
     delete[] buf;
   }
#endif
   return base64;
}



bool KSSLPKCS12::toFile(const QString &filename) {
#ifdef KSSL_HAVE_SSL
QFile out(filename);

   if (!out.open(QIODevice::WriteOnly)) return false;

   int fd = out.handle();
   FILE *fp = fdopen(fd, "w");

   if (!fp) {
      unlink(filename.toLatin1());
      return false;
   }

   kossl->i2d_PKCS12_fp(fp, _pkcs);

   fclose(fp);
   return true;
#endif
return false;
}


KSSLCertificate::KSSLValidation KSSLPKCS12::validate() {
	return validate(KSSLCertificate::SSLServer);
}


KSSLCertificate::KSSLValidation KSSLPKCS12::validate(KSSLCertificate::KSSLPurpose p) {
#ifdef KSSL_HAVE_SSL
KSSLCertificate::KSSLValidation xx = _cert->validate(p);
   if (1 != kossl->X509_check_private_key(_cert->getCert(), _pkey)) {
      xx = KSSLCertificate::PrivateKeyFailed;
   }

return xx;
#else
return KSSLCertificate::NoSSL;
#endif
}


KSSLCertificate::KSSLValidation KSSLPKCS12::revalidate() {
   return revalidate(KSSLCertificate::SSLServer);
}


KSSLCertificate::KSSLValidation KSSLPKCS12::revalidate(KSSLCertificate::KSSLPurpose p) {
   return _cert->revalidate(p);
}


bool KSSLPKCS12::isValid() {
return isValid(KSSLCertificate::SSLServer);
}


bool KSSLPKCS12::isValid(KSSLCertificate::KSSLPurpose p) {
return (validate(p) == KSSLCertificate::Ok);
}


QString KSSLPKCS12::name() const {
   return _cert->getSubject();
}


#ifdef KSSL_HAVE_SSL
#undef sk_new
#undef sk_push
#undef sk_free
#undef sk_value
#undef sk_num
#undef sk_pop
#undef sk_dup
#endif

