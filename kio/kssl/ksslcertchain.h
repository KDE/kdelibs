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

#ifndef _KSSLCERTCHAIN_H
#define _KSSLCERTCHAIN_H

#include <qglobal.h>
#if QT_VERSION < 300
#include <qlist.h>
#else
#include <qptrlist.h>
#endif

class QString;
class QCString;
class KSSL;
class KSSLCertChainPrivate;
class QStringList;

#include <ksslcertificate.h>

class KSSLCertChain {
friend class KSSL;
friend class KSSLPeerInfo;

public:
  KSSLCertChain();
  ~KSSLCertChain();

  bool isValid();

  KSSLCertChain *replicate();
  void setChain(void *stack_of_x509);
#if QT_VERSION < 300
  void setChain(QList<KSSLCertificate>& chain);
#else
  void setChain(QPtrList<KSSLCertificate>& chain);
#endif
  void setChain(QStringList chain);
#if QT_VERSION < 300
  QList<KSSLCertificate> getChain();
#else
  QPtrList<KSSLCertificate> getChain();
#endif
  int depth();
  void *rawChain() { return _chain; }


private:
  KSSLCertChainPrivate *d;
  void *_chain;
};


#endif

