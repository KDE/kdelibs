/*
   This file is part of the KDE libraries

   Copyright (c) 2001-2005 George Staikos <staikos@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/
#ifndef __KSSLD_H__
#define __KSSLD_H__

#include <kdedmodule.h>
#include <ksslcertificate.h>
#include <ksslcertificatecache.h>
#include <qstring.h>
#include <qstringlist.h>

#include <qmap.h>
#include <QVector>

class KSimpleConfig;
class KSSLCNode;
class KOpenSSLProxy;

class KSSLD : public KDEDModule
{
  Q_OBJECT
  K_DCOP

public:
  
  KSSLD(const QByteArray &name);
  
  virtual ~KSSLD();

k_dcop:
  //
  //  Certificate Cache methods
  //
  void cacheAddCertificate(KSSLCertificate cert, 
		           KSSLCertificateCache::KSSLCertificatePolicy policy,
		           bool permanent = true);
  KSSLCertificateCache::KSSLCertificatePolicy cacheGetPolicyByCN(QString cn);

  KSSLCertificateCache::KSSLCertificatePolicy cacheGetPolicyByCertificate(KSSLCertificate cert);

  bool cacheSeenCN(QString cn);
  bool cacheSeenCertificate(KSSLCertificate cert);

  bool cacheRemoveByCN(QString cn);
  bool cacheRemoveBySubject(QString subject);
  bool cacheRemoveByCertificate(KSSLCertificate cert);
	       
  bool cacheIsPermanent(KSSLCertificate cert);

  void cacheReload();

  bool cacheModifyByCN(QString cn,
                       KSSLCertificateCache::KSSLCertificatePolicy policy,
                       bool permanent,
                       QDateTime expires);

  bool cacheModifyByCertificate(KSSLCertificate cert,
                           KSSLCertificateCache::KSSLCertificatePolicy policy,
                                bool permanent,
                                QDateTime expires);

  QStringList cacheGetHostList(KSSLCertificate cert);

  bool cacheAddHost(KSSLCertificate cert, QString host);

  bool cacheRemoveHost(KSSLCertificate cert, QString host);

  /* Certificate Authorities */
  void caVerifyUpdate();
  bool caRegenerate();

  QStringList caList();

  bool caUseForSSL(QString subject);

  bool caUseForEmail(QString subject);
  
  bool caUseForCode(QString subject);

  bool caAdd(QString certificate, bool ssl, bool email, bool code);

  bool caAddFromFile(QString filename, bool ssl, bool email, bool code);

  bool caRemove(QString subject);

  bool caRemoveFromFile(QString filename);

  QString caGetCert(QString subject);

  bool caSetUse(QString subject, bool ssl, bool email, bool code);

  QStringList getKDEKeyByEmail(const QString &email);

  KSSLCertificate getCertByMD5Digest(const QString &key);

  //
  //  Certificate Home methods
  //

  QStringList getHomeCertificateList();

  bool addHomeCertificateFile(QString filename, QString password, bool storePass /*=false*/);

  bool addHomeCertificatePKCS12(QString base64cert, QString passToStore);

  bool deleteHomeCertificateByFile(QString filename, QString password);

  bool deleteHomeCertificateByPKCS12(QString base64cert, QString password);

  bool deleteHomeCertificateByName(QString name);

private:

  void cacheClearList();
  void cacheSaveToDisk();
  void cacheLoadDefaultPolicies();

  // for the cache portion:
  KSimpleConfig *cfg;
  QList<KSSLCNode*> certList;

  // Our pointer to OpenSSL
  KOpenSSLProxy *kossl;

  // 
  void searchAddCert(KSSLCertificate *cert);
  void searchRemoveCert(KSSLCertificate *cert);

  QMap<QString, QVector<KSSLCertificate*> > skEmail;
  QMap<QString, KSSLCertificate *> skMD5Digest;
};


#endif
