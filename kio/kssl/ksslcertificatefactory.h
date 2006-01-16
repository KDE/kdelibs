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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */ 

// WARNING: THIS CODE IS INCOMPLETE AND MAY CHANGE WITHOUT NOTICE

#ifndef _KSSLCERTIFICATEFACTORY_H
#define _KSSLCERTIFICATEFACTORY_H

#include <kdelibs_export.h>

class KSSLCertificate;

typedef enum {KEYTYPE_UNKNOWN, KEYTYPE_RSA, KEYTYPE_DSA} KSSLKeyType;

class KIO_EXPORT KSSLCertificateFactory {
 
public:

  static KSSLCertificate* generateSelfSigned(KSSLKeyType keytype);
  //  static KSSLCertificate* generateSigned();
  //  static bool generateRequest();

private:
 
  // add this if you ever add a constructor to this class
  //class KSSLCertificateFactoryPrivate;
  //KSSLCertificateFactoryPrivate* const d;
 
protected:

};

#endif

