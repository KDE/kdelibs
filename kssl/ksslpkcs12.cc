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

#include <qstring.h>
#include <qfile.h>
#include <ksslall.h>
#include <kdebug.h>
#include <ktempfile.h>
#include <kmdcodec.h>

#include <assert.h>


KSSLPKCS12::KSSLPKCS12() {
   _pkcs = NULL;
   _pkey = NULL;
   _cert = NULL;
   _caStack = NULL;
   kossl = KOSSL::self();
}



KSSLPKCS12::~KSSLPKCS12() {
#ifdef HAVE_SSL
   if (_pkcs) kossl->PKCS12_free(_pkcs);
   // FIXME: cleanup the stack and the private key!
   if (_pkey) kossl->EVP_PKEY_free(_pkey);
   //if (_caStack) 
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

  if (!newpkcs) return NULL;

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

  int rc = kossl->PKCS12_parse(_pkcs, pass.latin1(), &_pkey, &x, NULL);

  if (rc == 1) {
     // kdDebug() << "PKCS12_parse success" << endl;
     if (x) {
        _cert = new KSSLCertificate;
        _cert->setCert(x);
        return true;
     }
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
KTempFile ktf;

   kossl->i2d_PKCS12_fp(ktf.fstream(), _pkcs);
   ktf.close();
   QFile qf(ktf.name());
   qf.open(IO_ReadOnly);
   char *buf = new char[qf.size()];
   qf.readBlock(buf, qf.size());
   QByteArray qba;
   qba.setRawData(buf, qf.size());
   base64 = KCodecs::base64Encode(qba);
   qba.resetRawData(buf, qf.size());
   delete[] buf;
   qf.close();
   ktf.unlink();

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


// FIXME: check the private key and do the caching of the private key results
KSSLCertificate::KSSLValidation KSSLPKCS12::validate() {
   return _cert->validate();
}


KSSLCertificate::KSSLValidation KSSLPKCS12::revalidate() {
   return _cert->revalidate();
}


bool KSSLPKCS12::isValid() {
return (validate() == KSSLCertificate::Ok);
}


QString KSSLPKCS12::name() {
   return _cert->getSubject();
}


