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
#endif


KSSLPKCS7::KSSLPKCS7() {
   _pkcs = NULL;
   _cert = NULL;
   kossl = KOSSL::self();
}



KSSLPKCS7::~KSSLPKCS7() {
#ifdef HAVE_SSL
   if (_pkcs) kossl->PKCS7_free(_pkcs);
#endif
   if (_cert) delete _cert;
}


KSSLPKCS7* KSSLPKCS7::fromString(QString base64) {
#ifdef HAVE_SSL
KTempFile ktf;

    if (base64.isEmpty()) return NULL;
    QByteArray qba, qbb = QCString(base64.latin1()).copy();
    KCodecs::base64Decode(qbb, qba);
    ktf.file()->writeBlock(qba);
    ktf.close();
    KSSLPKCS7* rc = loadCertFile(ktf.name());
    ktf.unlink();
    return rc;
#endif
return NULL;
}



KSSLPKCS7* KSSLPKCS7::loadCertFile(QString filename) {
#ifdef HAVE_SSL
QFile qf(filename);
PKCS7 *newpkcs = NULL;

  if (!qf.open(IO_ReadOnly))
    return NULL;

  FILE *fp = fdopen(qf.handle(), "r");
  if (!fp) return NULL;

  newpkcs = KOSSL::self()->d2i_PKCS7_fp(fp, &newpkcs);

  if (!newpkcs) return NULL;

  KSSLPKCS7 *c = new KSSLPKCS7;
  c->setCert(newpkcs);

  return c;
#endif
return NULL;
}


void KSSLPKCS7::setCert(PKCS7 *c) {
#ifdef HAVE_SSL
   _pkcs = c;
   //STACK_OF(PKCS7_SIGNER_INFO) *PKCS7_get_signer_info(PKCS7 *p7);
   //X509 *PKCS7_cert_from_signer_info(PKCS7 *p7, PKCS7_SIGNER_INFO *si);
   // set _chain and _cert here.
#endif
}


KSSLCertificate *KSSLPKCS7::getCertificate() {
   return _cert;
}


KSSLCertChain *KSSLPKCS7::getChain() {
   return _chain;
}


QString KSSLPKCS7::toString() {
QString base64;
#ifdef HAVE_SSL
unsigned char *p;
int len;

   len = kossl->i2d_PKCS7(_pkcs, NULL);
   char *buf = new char[len];
   p = (unsigned char *)buf;
   kossl->i2d_PKCS7(_pkcs, &p);
   QByteArray qba;
   qba.setRawData(buf, len);
   base64 = KCodecs::base64Encode(qba);
   qba.resetRawData(buf, len);
   delete[] buf;
#endif
return base64;
}



bool KSSLPKCS7::toFile(QString filename) {
#ifdef HAVE_SSL
QFile out(filename);

   if (!out.open(IO_WriteOnly)) return false;

   int fd = out.handle();
   FILE *fp = fdopen(fd, "w");

   if (!fp) {
      unlink(filename.latin1());
      return false;
   }

   kossl->i2d_PKCS7_fp(fp, _pkcs);

   fclose(fp);
   return true;
#endif
return false;
}


KSSLCertificate::KSSLValidation KSSLPKCS7::validate() {
#ifdef HAVE_SSL
KSSLCertificate::KSSLValidation xx = _cert->validate();
return xx;
#else
return KSSLCertificate::NoSSL;
#endif
}


KSSLCertificate::KSSLValidation KSSLPKCS7::revalidate() {
   if (_cert)
      return _cert->revalidate();
   return KSSLCertificate::Unknown;
}


bool KSSLPKCS7::isValid() {
return (validate() == KSSLCertificate::Ok);
}


QString KSSLPKCS7::name() {
   if (_cert)
      return _cert->getSubject();
   return QString();
}

 
#ifdef HAVE_SSL
#undef sk_new
#undef sk_push
#undef sk_free
#undef sk_value
#undef sk_num
#undef sk_dup
#endif

