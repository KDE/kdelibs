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

// WARNING: THIS CODE IS INCOMPLETE AND MAY CHANGE WITHOUT NOTICE

#ifndef _INCLUDE_KSSLCCACHE_H
#define _INCLUDE_KSSLCCACHE_H

class QString;
class KSSLCertificate;
#include <qcstring.h>
#include <qdatetime.h>


class KSSLCertificateCache {
public:

enum KSSLCertificatePolicy { Unknown, Reject, Accept, Prompt, Ambiguous };
// Unknown: no policy has been set for this record
// Reject: user has requested to not accept data from this site
// Accept: user has requested to always accept data from this site
// Prompt: user wishes to be prompted before accepting this certificate
//         You may need to set a [non-]permanent policy on this record after
//         the user is prompted.
// Ambiguous: The state cannot be uniquely determined.  Hopefully this
//            doesn't happen.

  KSSLCertificateCache();
  ~KSSLCertificateCache();

  void addCertificate(KSSLCertificate& cert, KSSLCertificatePolicy policy, 
                                                     bool permanent = true);

  // WARNING!  This is not a "secure" method.  You need to actually
  //           do a getPolicyByCertificate to be cryptographically sure
  //           that this is an accepted certificate/site pair.
  //           (note that the site (CN) is encoded in the certificate
  //            so you should only accept certificates whose CN matches
  //            the exact FQDN of the site presenting it)
  //           If you're just doing an OpenSSL connection, I believe it
  //           tests this for you, but don't take my word for it.
  KSSLCertificatePolicy getPolicyByCN(QString& cn);

  KSSLCertificatePolicy getPolicyByCertificate(KSSLCertificate& cert);

  bool seenCN(QString& cn);
  bool seenCertificate(KSSLCertificate& cert);

  bool removeByCN(QString& cn);
  bool removeByCertificate(KSSLCertificate& cert);

  bool isPermanent(KSSLCertificate& cert);

  bool modifyByCN(QString& cn,
                  KSSLCertificateCache::KSSLCertificatePolicy policy,
                  bool permanent,
                  QDateTime& expires);

  bool modifyByCertificate(KSSLCertificate& cert,
                           KSSLCertificateCache::KSSLCertificatePolicy policy,
                           bool permanent,
                           QDateTime& expires);

  void reload();

  // You shouldn't need to call this but in some weird circumstances
  // it might be necessary.
  void saveToDisk();

private:
  class KSSLCertificateCachePrivate;
  KSSLCertificateCachePrivate *d;

  void loadDefaultPolicies();
  void clearList();

};


QDataStream& operator<<(QDataStream& s, const KSSLCertificateCache::KSSLCertificatePolicy& p);
QDataStream& operator>>(QDataStream& s, KSSLCertificateCache::KSSLCertificatePolicy& p);

#endif
