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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*

  DESIGN
  ------

  This is the first implementation and I think this cache actually needs
  experimentation to determine which implementation works best.  My current
  options are:

   (1) Store copies of the X509 certificates in a QList using a self
       organizing heuristic as described by Munro and Suwanda.
   (2) Store copies of the X509 certificates in a tree structure, perhaps
       a redblack tree, avl tree, or even just a simple binary tree.
   (3) Store the CN's in a tree or list and use them as a hash to retrieve 
       the X509 certificates.
   (4) Create "nodes" containing the X509 certificate and place them in
       two structures concurrently, one organized by CN, the other by
       X509 serial number.

  This implementation uses (1).  (4) is definitely attractive, but I don't
  think it will be necessary to go so crazy with performance, and perhaps
  end up performing poorly in situations where there are very few entries in
  the cache (which is most likely the case most of the time).  The style of
  heuristic is move-to-front, not swap-forward.  This seems to make more
  sense because the typical user will hit a site at least a few times in a
  row before moving to a new one.
 
  What I worry about most with respect to performance is that cryptographic
  routines are expensive and if we have to perform them on each X509
  certificate until the right one is found, we will perform poorly.

  All in all, this code is actually quite crucial for performance on SSL
  website, especially those with many image files loaded via SSL.  If a
  site loads 15 images, we will have to run through this code 15 times.
  A heuristic for self organization will make each successive lookup faster.
  Sounds good, doesn't it?

  DO NOT ATTEMPT TO GUESS WHICH CERTIFICATES ARE ACCEPTIBLE IN YOUR CODE!!
  ALWAYS USE THE CACHE.  IT MAY CHECK THINGS THAT YOU DON'T THINK OF, AND
  ALSO IF THERE IS A BUG IN THE CHECKING CODE, IF IT IS ALL CONTAINED IN
  THIS LIBRARY, A MINOR FIX WILL FIX ALL APPLICATIONS.
 */


#include "ksslcertificatecache.h"
#include "ksslcertchain.h"
#include "ksslcertificate.h"
#include <qlist.h>
#include <ksimpleconfig.h>

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

#ifdef HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#undef crypt
#endif

#include <kopenssl.h>

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


class KSSLCertificateCache::KSSLCertificateCachePrivate {
  public:
  QList<KSSLCNode> certList;
  KSimpleConfig *cfg;
  KOSSL *kossl;

  KSSLCertificateCachePrivate()  { certList.setAutoDelete(false); 
                                   kossl = KOSSL::self(); }
  ~KSSLCertificateCachePrivate() { }

};



KSSLCertificateCache::KSSLCertificateCache() {
  d = new KSSLCertificateCachePrivate;
  d->cfg = new KSimpleConfig("ksslpolicies", false);
  if (!KGlobal::dirs()->addResourceType("kssl", "share/apps/kssl")) {
     //    kdDebug(7029) << "Error adding (kssl, share/apps/kssl)" << endl;
  }
  loadDefaultPolicies();
}

KSSLCertificateCache::~KSSLCertificateCache() {
  saveToDisk();
  clearList();

  delete d->cfg;
  delete d;
}


void KSSLCertificateCache::saveToDisk() {
  KSSLCNode *node;

  for (node = d->certList.first(); node; node = d->certList.next()) {
    if (node->permanent || node->expires > QDateTime::currentDateTime()) {
      // First convert to a binary format and then write the kconfig entry
      // write the (CN, policy, cert) to KSimpleConfig
      d->cfg->setGroup(node->cert->getSubject());
      d->cfg->writeEntry("Certificate", node->cert->toString());
      d->cfg->writeEntry("Policy", node->policy);
      d->cfg->writeEntry("Expires", node->expires);
      d->cfg->writeEntry("Permanent", node->permanent);
      // Also write the chain
      QStringList qsl;
      QList<KSSLCertificate> cl(node->cert->chain().getChain());
      for (KSSLCertificate *c = cl.first(); c != 0; c = cl.next()) {
         //kdDebug() << "Certificate in chain: " <<  c->toString() << endl;
         qsl << c->toString();
      }
      d->cfg->writeEntry("Chain", qsl);
    }
  }  

  d->cfg->sync();

  // insure proper permissions -- contains sensitive data
  QString cfgName(KGlobal::dirs()->findResource("config", "ksslpolicies"));
  if (!cfgName.isEmpty())
    ::chmod(QFile::encodeName(cfgName), 0600);
}


void KSSLCertificateCache::clearList() {
  KSSLCNode *node;

  for (node = d->certList.first(); node; node = d->certList.next()) {
    d->certList.remove(node);
    delete node;
  }  
}


void KSSLCertificateCache::loadDefaultPolicies() {
  QStringList groups = d->cfg->groupList();

  for (QStringList::Iterator i = groups.begin();
                             i != groups.end();
                             ++i) {
    if ((*i).length() == 0) continue;
    d->cfg->setGroup(*i);

    // remove it if it has expired
    if (d->cfg->readDateTimeEntry("Expires") < QDateTime::currentDateTime()) {
       d->cfg->deleteGroup(*i);
       continue;
    }

    QCString encodedCert = d->cfg->readEntry("Certificate").local8Bit();
    KSSLCertificate *newCert = KSSLCertificate::fromString(encodedCert);
    if (!newCert) continue;
    KSSLCNode *n = new KSSLCNode;
    n->cert = newCert;
    n->policy = (KSSLCertificateCache::KSSLCertificatePolicy)
                d->cfg->readNumEntry("Policy");
    n->permanent = d->cfg->readBoolEntry("Permanent");
    n->expires = d->cfg->readDateTimeEntry("Expires");
    newCert->chain().setChain(d->cfg->readListEntry("Chain"));
    d->certList.append(n); 
  }
}


void KSSLCertificateCache::addCertificate(KSSLCertificate& cert, 
                       KSSLCertificatePolicy policy, bool permanent) {
  KSSLCNode *node;

  for (node = d->certList.first(); node; node = d->certList.next()) {
    if (cert == *(node->cert)) {
      node->policy = policy;
      node->permanent = permanent;
      if (!permanent) {
        node->expires = QDateTime::currentDateTime();
// FIXME: make this configurable
        node->expires = node->expires.addSecs(3600);
      }
      saveToDisk();
      return;
    }
  }

  KSSLCNode *n = new KSSLCNode;
  n->cert = cert.replicate();
  n->policy = policy;
  n->permanent = permanent;
  d->certList.prepend(n); 
  if (!permanent) {
    n->expires = QDateTime::currentDateTime();
    n->expires = n->expires.addSecs(3600);
  }
  saveToDisk();
}


KSSLCertificateCache::KSSLCertificatePolicy KSSLCertificateCache::getPolicyByCN(QString& cn) {
  KSSLCNode *node;

  for (node = d->certList.first(); node; node = d->certList.next()) {
    if (node->cert->getSubject() == cn) {
      if (!node->permanent && node->expires < QDateTime::currentDateTime()) {
        d->certList.remove(node);
        d->cfg->deleteGroup(node->cert->getSubject());
        delete node;
        continue;
      }
      d->certList.remove(node);
      d->certList.prepend(node);
      return node->policy;
    }
  }
  return Unknown;
}


KSSLCertificateCache::KSSLCertificatePolicy KSSLCertificateCache::getPolicyByCertificate(KSSLCertificate& cert) {
  KSSLCNode *node;

  for (node = d->certList.first(); node; node = d->certList.next()) {
    if (cert == *(node->cert)) {  
      if (!node->permanent && node->expires < QDateTime::currentDateTime()) {
        d->certList.remove(node);
        d->cfg->deleteGroup(node->cert->getSubject());
        delete node;
        continue;
      }
      d->certList.remove(node);
      d->certList.prepend(node);
      return node->policy;
    }
  }
  return Unknown;
}


bool KSSLCertificateCache::seenCN(QString& cn) {
  KSSLCNode *node;

  for (node = d->certList.first(); node; node = d->certList.next()) {
    if (node->cert->getSubject() == cn) {
      if (!node->permanent && node->expires < QDateTime::currentDateTime()) {
        d->certList.remove(node);
        d->cfg->deleteGroup(node->cert->getSubject());
        delete node;
        continue;
      }
      d->certList.remove(node);
      d->certList.prepend(node);
      return true;
    }
  }
  return false;
}


bool KSSLCertificateCache::seenCertificate(KSSLCertificate& cert) {
  KSSLCNode *node;

  for (node = d->certList.first(); node; node = d->certList.next()) {
    if (cert == *(node->cert)) {
      if (!node->permanent && node->expires < QDateTime::currentDateTime()) {
        d->certList.remove(node);
        d->cfg->deleteGroup(node->cert->getSubject());
        delete node;
        continue;
      }
      d->certList.remove(node);
      d->certList.prepend(node);
      return true;
    }
  }
  return false;
}


bool KSSLCertificateCache::isPermanent(KSSLCertificate& cert) {
  KSSLCNode *node;

  for (node = d->certList.first(); node; node = d->certList.next()) {
    if (cert == *(node->cert)) {
      if (!node->permanent && node->expires < QDateTime::currentDateTime()) {
        d->certList.remove(node);
        d->cfg->deleteGroup(node->cert->getSubject());
        delete node;
        continue;
      }
      d->certList.remove(node);
      d->certList.prepend(node);
      return node->permanent;
    }
  }
  return false;
}


bool KSSLCertificateCache::removeByCN(QString& cn) {
  KSSLCNode *node;
  bool gotOne = false;

  for (node = d->certList.first(); node; node = d->certList.next()) {
    if (node->cert->getSubject() == cn) {
      d->certList.remove(node);
      d->cfg->deleteGroup(node->cert->getSubject());
      delete node;
      saveToDisk();
      gotOne = true;
    }
  }
  return gotOne;
}


bool KSSLCertificateCache::removeByCertificate(KSSLCertificate& cert) {
  KSSLCNode *node;

  for (node = d->certList.first(); node; node = d->certList.next()) {
    if (cert == *(node->cert)) {
      d->certList.remove(node);
      d->cfg->deleteGroup(node->cert->getSubject());
      delete node;
      saveToDisk();
      return true;
    }
  }
  return false;
}


