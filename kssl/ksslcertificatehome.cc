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

#include <ksslcertificatehome.h>
#include <ksslcertificate.h>
#include <ksslpkcs12.h>

#include <ksimpleconfig.h>


QStringList KSSLCertificateHome::getCertificateList() {
KSimpleConfig cfg("ksslcertificates", false);
QStringList list = cfg.groupList();

list.remove(list.last());  // get rid of "<default>"
return list;
}


void KSSLCertificateHome::setDefaultCertificate(QString name, QString host) {

}


void KSSLCertificateHome::setDefaultCertificate(KSSLPKCS12 *cert, QString host) {

}


bool KSSLCertificateHome::addCertificate(QString filename, QString password, bool storePass) {
KSSLPKCS12 *pkcs = KSSLPKCS12::loadCertFile(filename, password);

  if (!pkcs) return false;

  KSSLCertificateHome::addCertificate(pkcs, storePass?password:QString(""));
  delete pkcs;

return true;
}


void KSSLCertificateHome::addCertificate(KSSLPKCS12 *cert, QString passToStore) {
   if (!cert) return;

KSimpleConfig cfg("ksslcertificates", false);

   cfg.setGroup(cert->name());
   cfg.writeEntry("PKCS12Base64", cert->toString());
   cfg.writeEntry("Password", passToStore);
   cfg.sync();
}


KSSLPKCS12* KSSLCertificateHome::getCertificateByName(QString name, QString password) {
KSimpleConfig cfg("ksslcertificates", false);
  if (!cfg.hasGroup(name)) return NULL;

  cfg.setGroup(name);

  return KSSLPKCS12::fromString(cfg.readEntry("PKCS12Base64", ""), password);
}


KSSLPKCS12* KSSLCertificateHome::getCertificateByHost(QString host, QString password) {
   return KSSLCertificateHome::getCertificateByName(KSSLCertificateHome::getDefaultCertificateName(host), password);
}


QString KSSLCertificateHome::getDefaultCertificateName(QString host) {

}


QString KSSLCertificateHome::getDefaultCertificateName() {

}


KSSLPKCS12* KSSLCertificateHome::getDefaultCertificate(QString password) {

}



