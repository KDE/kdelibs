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

#include <qtimer.h>

#include "kssld.h"
#include <kconfig.h>
#include <ksimpleconfig.h>
#include "ksslcertchain.h"
#include "ksslcertificate.h"
#include <qlist.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <qfile.h>
#include <qsortedlist.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <qdatetime.h>

#include <kmdcodec.h>
#include <kopenssl.h>


extern "C" {
   KDEDModule *create_kssld(const QCString &name) {
	   return new KSSLD(name);
   }
}


KSSLD::KSSLD(const QCString &name) : KDEDModule(name)
{
// ----------------------- FOR THE CACHE ------------------------------------	
  cfg = new KSimpleConfig("ksslpolicies", false);
  if (!KGlobal::dirs()->addResourceType("kssl", "share/apps/kssl")) {
     //    kdDebug(7029) << "Error adding (kssl, share/apps/kssl)" << endl;
  }
  cacheLoadDefaultPolicies();
  certList.setAutoDelete(false);
  kossl = KOSSL::self();

// ----------------------- FOR THE HOME -------------------------------------
}
  

KSSLD::~KSSLD()
{
// ----------------------- FOR THE CACHE ------------------------------------	
  cacheClearList();
  delete cfg;

// ----------------------- FOR THE HOME -------------------------------------
}

  


// A node in the cache
class KSSLCNode {
public:
  KSSLCertificate *cert;
  KSSLCertificateCache::KSSLCertificatePolicy policy;
  bool permanent;
  QDateTime expires;
  KSSLCNode() { cert = NULL; policy = KSSLCertificateCache::Unknown; 
                permanent = true; }
  ~KSSLCNode() { if (cert) delete cert; }
};



void KSSLD::cacheSaveToDisk() {
  KSSLCNode *node;

  for (node = certList.first(); node; node = certList.next()) {
    if (node->permanent || node->expires > QDateTime::currentDateTime()) {
      // First convert to a binary format and then write the kconfig entry
      // write the (CN, policy, cert) to KSimpleConfig
      cfg->setGroup(node->cert->getSubject());
      cfg->writeEntry("Certificate", node->cert->toString());
      cfg->writeEntry("Policy", node->policy);
      cfg->writeEntry("Expires", node->expires);
      cfg->writeEntry("Permanent", node->permanent);
      // Also write the chain
      QStringList qsl;
      QList<KSSLCertificate> cl = node->cert->chain().getChain();
      for (KSSLCertificate *c = cl.first(); c != 0; c = cl.next()) {
         //kdDebug() << "Certificate in chain: " <<  c->toString() << endl;
         qsl << c->toString();
      }
      cl.setAutoDelete(true);
      cfg->writeEntry("Chain", qsl);
    }
  }  

  cfg->sync();

  // insure proper permissions -- contains sensitive data
  QString cfgName(KGlobal::dirs()->findResource("config", "ksslpolicies"));
  if (!cfgName.isEmpty())
    ::chmod(QFile::encodeName(cfgName), 0600);
}


void KSSLD::cacheClearList() {
  KSSLCNode *node;

  for (node = certList.first(); node; node = certList.next()) {
    certList.remove(node);
    delete node;
  }  
}


void KSSLD::cacheLoadDefaultPolicies() {
  QStringList groups = cfg->groupList();

  for (QStringList::Iterator i = groups.begin();
                             i != groups.end();
                             ++i) {
    if ((*i).length() == 0) continue;
    cfg->setGroup(*i);

    // remove it if it has expired
    if (cfg->readDateTimeEntry("Expires") < QDateTime::currentDateTime()) {
       cfg->deleteGroup(*i);
       continue;
    }

    QCString encodedCert = cfg->readEntry("Certificate").local8Bit();
    KSSLCertificate *newCert = KSSLCertificate::fromString(encodedCert);
    if (!newCert) continue;
    KSSLCNode *n = new KSSLCNode;
    n->cert = newCert;
    n->policy = (KSSLCertificateCache::KSSLCertificatePolicy)
                cfg->readNumEntry("Policy");
    n->permanent = cfg->readBoolEntry("Permanent");
    n->expires = cfg->readDateTimeEntry("Expires");
    newCert->chain().setChain(cfg->readListEntry("Chain"));
    certList.append(n); 
  }
}


void KSSLD::cacheAddCertificate(KSSLCertificate& cert, 
         KSSLCertificateCache::KSSLCertificatePolicy policy, bool permanent) {
  KSSLCNode *node;

  for (node = certList.first(); node; node = certList.next()) {
    if (cert == *(node->cert)) {
      node->policy = policy;
      node->permanent = permanent;
      if (!permanent) {
        node->expires = QDateTime::currentDateTime();
// FIXME: make this configurable
        node->expires = node->expires.addSecs(3600);
      }
      cacheSaveToDisk();
      return;
    }
  }

  KSSLCNode *n = new KSSLCNode;
  n->cert = cert.replicate();
  n->policy = policy;
  n->permanent = permanent;
  certList.prepend(n); 
  if (!permanent) {
    n->expires = QDateTime::currentDateTime();
    n->expires = n->expires.addSecs(3600);
  }
  cacheSaveToDisk();
}


KSSLCertificateCache::KSSLCertificatePolicy KSSLD::cacheGetPolicyByCN(QString cn) {
  KSSLCNode *node;

  for (node = certList.first(); node; node = certList.next()) {
    if (node->cert->getSubject() == cn) {
      if (!node->permanent && node->expires < QDateTime::currentDateTime()) {
        certList.remove(node);
        cfg->deleteGroup(node->cert->getSubject());
        delete node;
        continue;
      }
      certList.remove(node);
      certList.prepend(node);
      return node->policy;
    }
  }
  return KSSLCertificateCache::Unknown;
}


KSSLCertificateCache::KSSLCertificatePolicy KSSLD::cacheGetPolicyByCertificate(KSSLCertificate& cert) {
  KSSLCNode *node;

  for (node = certList.first(); node; node = certList.next()) {
    if (cert == *(node->cert)) {  
      if (!node->permanent && node->expires < QDateTime::currentDateTime()) {
        certList.remove(node);
        cfg->deleteGroup(node->cert->getSubject());
        delete node;
        continue;
      }
      certList.remove(node);
      certList.prepend(node);
      return node->policy;
    }
  }
  return KSSLCertificateCache::Unknown;
}


bool KSSLD::cacheSeenCN(QString cn) {
  KSSLCNode *node;

  for (node = certList.first(); node; node = certList.next()) {
    if (node->cert->getSubject() == cn) {
      if (!node->permanent && node->expires < QDateTime::currentDateTime()) {
        certList.remove(node);
        cfg->deleteGroup(node->cert->getSubject());
        delete node;
        continue;
      }
      certList.remove(node);
      certList.prepend(node);
      return true;
    }
  }
  return false;
}


bool KSSLD::cacheSeenCertificate(KSSLCertificate& cert) {
  KSSLCNode *node;

  for (node = certList.first(); node; node = certList.next()) {
    if (cert == *(node->cert)) {
      if (!node->permanent && node->expires < QDateTime::currentDateTime()) {
        certList.remove(node);
        cfg->deleteGroup(node->cert->getSubject());
        delete node;
        continue;
      }
      certList.remove(node);
      certList.prepend(node);
      return true;
    }
  }
  return false;
}


bool KSSLD::cacheIsPermanent(KSSLCertificate& cert) {
  KSSLCNode *node;

  for (node = certList.first(); node; node = certList.next()) {
    if (cert == *(node->cert)) {
      if (!node->permanent && node->expires < QDateTime::currentDateTime()) {
        certList.remove(node);
        cfg->deleteGroup(node->cert->getSubject());
        delete node;
        continue;
      }
      certList.remove(node);
      certList.prepend(node);
      return node->permanent;
    }
  }
  return false;
}


bool KSSLD::cacheRemoveByCN(QString cn) {
  KSSLCNode *node;
  bool gotOne = false;

  for (node = certList.first(); node; node = certList.next()) {
    if (node->cert->getSubject() == cn) {
      certList.remove(node);
      cfg->deleteGroup(node->cert->getSubject());
      delete node;
      cacheSaveToDisk();
      gotOne = true;
    }
  }
  return gotOne;
}


bool KSSLD::cacheRemoveByCertificate(KSSLCertificate& cert) {
  KSSLCNode *node;

  for (node = certList.first(); node; node = certList.next()) {
    if (cert == *(node->cert)) {
      certList.remove(node);
      cfg->deleteGroup(node->cert->getSubject());
      delete node;
      cacheSaveToDisk();
      return true;
    }
  }
  return false;
}



#include "kssld.moc"

