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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kopenssl.h>

#include <qstring.h>
#include <qfile.h>
#include <ksslall.h>
#include <kdebug.h>
#include <ktempfile.h>
#include <kmdcodec.h>

#include <assert.h>

#ifdef HAVE_SSL
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
#ifdef HAVE_SSL
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
   if (_cert) delete _cert;
}


KSSLPKCS12* KSSLPKCS12::fromString(QString base64, QString password) {
#ifdef HAVE_SSL
KTempFile ktf;

    if (base64.isEmpty()) return NULL;
    QByteArray qba, qbb = QCString(base64.latin1()).copy();
    KCodecs::base64Decode(qbb, qba);
    ktf.file()->writeBlock(qba);
    ktf.close();
    KSSLPKCS12* rc = loadCertFile(ktf.name(), password);
    ktf.unlink();
    return rc;
#endif
return NULL;
}



KSSLPKCS12* KSSLPKCS12::loadCertFile(QString filename, QString password) {
#ifdef HAVE_SSL
QFile qf(filename);
PKCS12 *newpkcs = NULL;

  if (!qf.open(IO_ReadOnly))
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
#ifdef HAVE_SSL
   _pkcs = c;
#endif
}


bool KSSLPKCS12::changePassword(QString pold, QString pnew) {
#ifdef HAVE_SSL
   // OpenSSL makes me cast away the const here.  argh
   return (0 == kossl->PKCS12_newpass(_pkcs, 
                                      (char *)pold.latin1(), 
                                      (char *)pnew.latin1()));
#endif
return false;
}


bool KSSLPKCS12::parse(QString pass) {
#ifdef HAVE_SSL
X509 *x = NULL;

  assert(_pkcs);   // if you're calling this before pkcs gets set, it's a BUG!

   if (_cert) delete _cert;
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

  int rc = kossl->PKCS12_parse(_pkcs, pass.latin1(), &_pkey, &x, &_caStack);

  if (rc == 1) {
     // kdDebug(7029) << "PKCS12_parse success" << endl;
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


QString KSSLPKCS12::toString() {
QString base64;
#ifdef HAVE_SSL
unsigned char *p;
int len;

   len = kossl->i2d_PKCS12(_pkcs, NULL);
   char *buf = new char[len];
   p = (unsigned char *)buf;
   kossl->i2d_PKCS12(_pkcs, &p);
   QByteArray qba;
   qba.setRawData(buf, len);
   base64 = KCodecs::base64Encode(qba);
   qba.resetRawData(buf, len);
   delete[] buf;
#endif
return base64;
}



bool KSSLPKCS12::toFile(QString filename) {
#ifdef HAVE_SSL
QFile out(filename);

   if (!out.open(IO_WriteOnly)) return false;

   int fd = out.handle();
   FILE *fp = fdopen(fd, "w");

   if (!fp) {
      unlink(filename.latin1());
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
#ifdef HAVE_SSL
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


QString KSSLPKCS12::name() {
   return _cert->getSubject();
}

 
#ifdef HAVE_SSL
#undef sk_new
#undef sk_push
#undef sk_free
#undef sk_value
#undef sk_num
#undef sk_pop
#undef sk_dup
#endif

