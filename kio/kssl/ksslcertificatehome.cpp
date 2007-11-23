/* This file is part of the KDE project
 *
 * Copyright (C) 2000-2005 George Staikos <staikos@kde.org>
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

#include <ksslcertificatehome.h>
#include <ksslcertificate.h>
#include <ksslpkcs12.h>

#include <kconfiggroup.h>
#include <kconfig.h>

QStringList KSSLCertificateHome::getCertificateList()
{
    KConfig cfg("ksslcertificates", KConfig::SimpleConfig);
    return cfg.groupList();
}


void KSSLCertificateHome::setDefaultCertificate(const QString & name, const QString &host, bool send, bool prompt)
{
    KConfig file("ksslauthmap", KConfig::SimpleConfig);
    KConfigGroup cfg(&file, QString::fromLatin1(QUrl::toAce(host)));

    cfg.writeEntry("certificate", name);
    cfg.writeEntry("send", send);
    cfg.writeEntry("prompt", prompt);
    cfg.sync();
}


void KSSLCertificateHome::setDefaultCertificate(KSSLPKCS12 *cert, const QString &host, bool send, bool prompt) {
    if (cert)
        KSSLCertificateHome::setDefaultCertificate(cert->name(), host, send, prompt);
}


bool KSSLCertificateHome::addCertificate(const QString &filename, const QString &password, bool storePass) {
    KSSLPKCS12 *pkcs = KSSLPKCS12::loadCertFile(filename, password);

    if (!pkcs) return false;

    KSSLCertificateHome::addCertificate(pkcs, storePass?password:QString(""));
    delete pkcs;

    return true;
}


bool KSSLCertificateHome::addCertificate(KSSLPKCS12 *cert, const QString &passToStore) {
   if (!cert) return false;

   KConfig file("ksslcertificates", KConfig::SimpleConfig);
   KConfigGroup cfg = file.group(cert->name().toLatin1());

   cfg.writeEntry("PKCS12Base64", cert->toString());
   cfg.writeEntry("Password", passToStore);
   cfg.sync();
   return true;
}

bool KSSLCertificateHome::deleteCertificate(const QString &filename, const QString &password) {
KSSLPKCS12 *pkcs = KSSLPKCS12::loadCertFile(filename, password);

   if (!pkcs) return false;

   bool ok = deleteCertificate(pkcs);
   delete pkcs;

   return ok;
}

bool KSSLCertificateHome::deleteCertificate(KSSLPKCS12 *cert) {
   if (!cert) return false;

   return deleteCertificateByName(cert->name());
}

bool KSSLCertificateHome::deleteCertificateByName(const QString &name) {
    if (name.isEmpty()) return false;

    KConfig cfg("ksslcertificates", KConfig::SimpleConfig);

    cfg.deleteGroup(name);
    cfg.sync();

    return true;
}

KSSLPKCS12* KSSLCertificateHome::getCertificateByName(const QString &name, const QString &password)
{
    KConfig cfg("ksslcertificates", KConfig::SimpleConfig);
    if (!cfg.hasGroup(name)) return NULL;

    KConfigGroup cg(&cfg, name);

    return KSSLPKCS12::fromString(cg.readEntry("PKCS12Base64", ""), password);
}


KSSLPKCS12* KSSLCertificateHome::getCertificateByName(const QString &name)
{
    KConfig cfg("ksslcertificates", KConfig::SimpleConfig);
    if (!cfg.hasGroup(name)) return NULL;

    KConfigGroup cg(&cfg, name);

    return KSSLPKCS12::fromString(cg.readEntry("PKCS12Base64", ""), cg.readEntry("Password", ""));
}


bool KSSLCertificateHome::hasCertificateByName(const QString &name) {
  KConfig cfg("ksslcertificates", KConfig::SimpleConfig);
  if (!cfg.hasGroup(name)) return false;
  return true;
}

KSSLPKCS12* KSSLCertificateHome::getCertificateByHost(const QString &host,
                                                      const QString &password, KSSLAuthAction *aa)
{
    return KSSLCertificateHome::getCertificateByName(KSSLCertificateHome::getDefaultCertificateName(host, aa), password);
}


QString KSSLCertificateHome::getDefaultCertificateName(const QString &host, KSSLAuthAction *aa)
{
    KConfig file("ksslauthmap", KConfig::SimpleConfig);
    KConfigGroup cfg = file.group(QString::fromLatin1(QUrl::toAce(host)));

    if (!cfg.exists()) {
        if (aa) *aa = AuthNone;
        return QString();
    } else {
        if (aa) {
            bool tmp = cfg.readEntry("send", false);
            *aa = AuthSend;
            if (!tmp) {
                tmp = cfg.readEntry("prompt", false);
                *aa = AuthPrompt;
                if (!tmp) {
                    *aa = AuthDont;
                }
            }
        }
        return cfg.readEntry("certificate", "");
    }
}


QString KSSLCertificateHome::getDefaultCertificateName(KSSLAuthAction *aa)
{
   KConfig _cfg("cryptodefaults", KConfig::NoGlobals);
   KConfigGroup cfg(&_cfg, "Auth");
   if (aa) {
      QString am = cfg.readEntry("AuthMethod", "");
      if (am == "send")
         *aa = AuthSend;
      else if (am == "prompt")
         *aa = AuthPrompt;
      else
         *aa = AuthDont;
   }

   return cfg.readEntry("DefaultCert", "");
}


KSSLPKCS12* KSSLCertificateHome::getDefaultCertificate(const QString &password, KSSLAuthAction *aa) {
    QString name = KSSLCertificateHome::getDefaultCertificateName(aa);
    KConfig cfg("ksslcertificates", KConfig::SimpleConfig);

    if (name.isEmpty()) return NULL;

    KConfigGroup cg(&cfg, name);
    return KSSLPKCS12::fromString(cg.readEntry("PKCS12Base64", ""), password);
}



KSSLPKCS12* KSSLCertificateHome::getDefaultCertificate(KSSLAuthAction *aa) {
   QString name = KSSLCertificateHome::getDefaultCertificateName(aa);
   KConfig cfg("ksslcertificates", KConfig::SimpleConfig);

   if (name.isEmpty()) return NULL;

   KConfigGroup cg(&cfg, name);

   return KSSLPKCS12::fromString(cg.readEntry("PKCS12Base64", ""),
                                 cg.readEntry("Password", ""));
}


void KSSLCertificateHome::setDefaultCertificate(const QString &name, bool send, bool prompt)
{
   KConfig cfg("ksslauthmap", KConfig::SimpleConfig);
   KConfigGroup cg(&cfg, "<default>");
   cg.writeEntry("defaultCertificate", name);
   cg.writeEntry("send", send);
   cg.writeEntry("prompt", prompt);
}


void KSSLCertificateHome::setDefaultCertificate(KSSLPKCS12 *cert, bool send, bool prompt) {
   if (cert)
       KSSLCertificateHome::setDefaultCertificate(cert->name(), send, prompt);
}

