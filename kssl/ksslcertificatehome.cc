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


void KSSLCertificateHome::setDefaultCertificate(QString name, QString host, bool send, bool prompt) {
KSimpleConfig cfg("ksslauthmap", false);

   cfg.setGroup(host);
   cfg.writeEntry("certificate", name);
   cfg.writeEntry("send", send);
   cfg.writeEntry("prompt", prompt);
   cfg.sync();
}


void KSSLCertificateHome::setDefaultCertificate(KSSLPKCS12 *cert, QString host, bool send, bool prompt) {
   if (cert)
      KSSLCertificateHome::setDefaultCertificate(cert->name(), host, send, prompt);
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


KSSLPKCS12* KSSLCertificateHome::getCertificateByName(QString name) {
KSimpleConfig cfg("ksslcertificates", false);
  if (!cfg.hasGroup(name)) return NULL;

  cfg.setGroup(name);

  return KSSLPKCS12::fromString(cfg.readEntry("PKCS12Base64", ""), cfg.readEntry("Password", ""));
}


KSSLPKCS12* KSSLCertificateHome::getCertificateByHost(QString host, QString password, bool *send, bool *prompt) {
   return KSSLCertificateHome::getCertificateByName(KSSLCertificateHome::getDefaultCertificateName(host, send, prompt), password);
}


QString KSSLCertificateHome::getDefaultCertificateName(QString host, bool *send, bool *prompt) {
KSimpleConfig cfg("ksslauthmap", false);

   cfg.setGroup(host);
   if (send) *send = cfg.readBoolEntry("send", false);
   if (prompt) *prompt = cfg.readBoolEntry("prompt", false);
   if (!cfg.readBoolEntry("send", false)) return QString("");

return cfg.readEntry("certificate", "");
}


QString KSSLCertificateHome::getDefaultCertificateName(bool *send, bool *prompt) {
KConfig cfg("cryptodefaults", false);

   cfg.setGroup("Auth");
   if (send) *send = (cfg.readEntry("AuthMethod", "") == "send");
   if (prompt) *prompt = (cfg.readEntry("AuthMethod", "") == "prompt");
return cfg.readEntry("DefaultCert", "");
}


KSSLPKCS12* KSSLCertificateHome::getDefaultCertificate(QString password, bool *send, bool *prompt) {
QString name = KSSLCertificateHome::getDefaultCertificateName(send, prompt);
KSimpleConfig cfg("ksslcertificates", false);

   if (name.isEmpty()) return NULL;

   cfg.setGroup(name);
   return KSSLPKCS12::fromString(cfg.readEntry("PKCS12Base64", ""), password);
}



KSSLPKCS12* KSSLCertificateHome::getDefaultCertificate(bool *send, bool *prompt) {
QString name = KSSLCertificateHome::getDefaultCertificateName();
KSimpleConfig cfg("ksslcertificates", false);

   if (name.isEmpty()) return NULL;

   cfg.setGroup(name);
   if (send) *send = cfg.readBoolEntry("send", false);
   if (prompt) *prompt = cfg.readBoolEntry("prompt", false);
   return KSSLPKCS12::fromString(cfg.readEntry("PKCS12Base64", ""), 
                                 cfg.readEntry("Password", ""));
}


void KSSLCertificateHome::setDefaultCertificate(QString name, bool send, bool prompt) {
KSimpleConfig cfg("ksslauthmap", false);

   cfg.setGroup("<default>");
   cfg.writeEntry("defaultCertificate", name);
   cfg.writeEntry("send", send);
   cfg.writeEntry("prompt", prompt);
}


void KSSLCertificateHome::setDefaultCertificate(KSSLPKCS12 *cert, bool send, bool prompt) {
   if (cert)
   KSSLCertificateHome::setDefaultCertificate(cert->name(), send, prompt);
}





