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
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <QtCore/QMap>
#include <QtCore/QVector>

class KConfig;
class KSSLCNode;
class KOpenSSLProxy;

class KSSLD : public KDEDModule
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.kde.KSSLD")

public:

  KSSLD(QObject* parent, const QList<QVariant>&);

  virtual ~KSSLD();

  //
  //  Certificate Cache methods
  //
  void cacheAddCertificate(KSSLCertificate cert,
		           KSSLCertificateCache::KSSLCertificatePolicy policy,
		           bool permanent = true);
  KSSLCertificateCache::KSSLCertificatePolicy cacheGetPolicyByCertificate(KSSLCertificate cert);
  bool cacheSeenCertificate(KSSLCertificate cert);
  bool cacheRemoveByCertificate(KSSLCertificate cert);
  bool cacheIsPermanent(KSSLCertificate cert);
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

public Q_SLOTS:
  Q_SCRIPTABLE void cacheAddCertificate(QByteArray certData, int policy, bool permanent);
  Q_SCRIPTABLE int cacheGetPolicyByCN(QString cn);
  Q_SCRIPTABLE int cacheGetPolicyByCertificate(QByteArray certData);
  Q_SCRIPTABLE bool cacheSeenCN(QString cn);
  Q_SCRIPTABLE bool cacheSeenCertificate(QByteArray certData);

  Q_SCRIPTABLE bool cacheRemoveByCN(QString cn);
  Q_SCRIPTABLE bool cacheRemoveBySubject(QString subject);
  Q_SCRIPTABLE bool cacheRemoveByCertificate(QByteArray cert);

  Q_SCRIPTABLE bool cacheIsPermanent(QByteArray cert);

  Q_SCRIPTABLE void cacheReload();

  Q_SCRIPTABLE bool cacheModifyByCN(QString cn,
                                    int policy,
                                    bool permanent,
                                    qlonglong expires);

  Q_SCRIPTABLE bool cacheModifyByCertificate(QByteArray certData,
                                             int policy,
                                             bool permanent,
                                             qlonglong expires);

  Q_SCRIPTABLE QStringList cacheGetHostList(QByteArray certData);

  Q_SCRIPTABLE bool cacheAddHost(QByteArray cert, QString host);

  Q_SCRIPTABLE bool cacheRemoveHost(QByteArray cert, QString host);

  /* Certificate Authorities */
  Q_SCRIPTABLE void caVerifyUpdate();
  Q_SCRIPTABLE bool caRegenerate();

  Q_SCRIPTABLE QStringList caList();

  Q_SCRIPTABLE bool caUseForSSL(QString subject);

  Q_SCRIPTABLE bool caUseForEmail(QString subject);

  Q_SCRIPTABLE bool caUseForCode(QString subject);

  Q_SCRIPTABLE bool caAdd(QString certificate, bool ssl, bool email, bool code);

  Q_SCRIPTABLE bool caAddFromFile(QString filename, bool ssl, bool email, bool code);

  Q_SCRIPTABLE bool caRemove(QString subject);

  Q_SCRIPTABLE bool caRemoveFromFile(QString filename);

  Q_SCRIPTABLE QString caGetCert(QString subject);

  Q_SCRIPTABLE bool caSetUse(QString subject, bool ssl, bool email, bool code);

  Q_SCRIPTABLE QStringList getKDEKeyByEmail(const QString &email);

  Q_SCRIPTABLE KSSLCertificate getCertByMD5Digest(const QString &key);

  //
  //  Certificate Home methods
  //

  Q_SCRIPTABLE QStringList getHomeCertificateList();

  Q_SCRIPTABLE bool addHomeCertificateFile(QString filename, QString password, bool storePass /*=false*/);

  Q_SCRIPTABLE bool addHomeCertificatePKCS12(QString base64cert, QString passToStore);

  Q_SCRIPTABLE bool deleteHomeCertificateByFile(QString filename, QString password);

  Q_SCRIPTABLE bool deleteHomeCertificateByPKCS12(QString base64cert, QString password);

  Q_SCRIPTABLE bool deleteHomeCertificateByName(QString name);

private:

  void cacheClearList();
  void cacheSaveToDisk();
  void cacheLoadDefaultPolicies();

  // for the cache portion:
  KConfig *cfg;
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
