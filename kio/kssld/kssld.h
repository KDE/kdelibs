/*
   This file is part of the KDE libraries

   Copyright (c) 2001 George Staikos <staikos@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/
#ifndef __KSSLD_H__
#define __KSSLD_H__

#include <kded/kdedmodule.h>
#include <ksslcertificate.h>
#include <ksslcertificatecache.h>
#include <qstring.h>
#include <qdatetime.h>


class KSimpleConfig;
class KSSLCNode;
class KOpenSSLProxy;

class KSSLD : public KDEDModule
{
  Q_OBJECT
  K_DCOP

public:
  
  KSSLD(const QCString &name);
  
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

  //
  //  Certificate Home methods
  //

private:

  void cacheClearList();
  void cacheSaveToDisk();
  void cacheLoadDefaultPolicies();

  // for the cache portion:
  KSimpleConfig *cfg;
  QList<KSSLCNode> certList;

  // Our pointer to OpenSSL
  KOpenSSLProxy *kossl;
};


#endif
