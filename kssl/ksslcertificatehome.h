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

#ifndef _KSSLCERTIFICATEHOME_H
#define _KSSLCERTIFICATEHOME_H

class KSSLCertificate;
class QString;

class KSSLCertificateHome {
 
public:
  static KSSLCertificate* getCertificateByHost(QString& host);
  static KSSLCertificate* getDefaultCertificate();
  static void setDefaultCertificate(QString& filename, QString& host);
  static void addCertificate(QString& filename, QString& host);
  static void addCertificate(KSSLCertificate* cert, QString& host);
 
private:
 
  class KSSLCertificateHomePrivate;
  KSSLCertificateHomePrivate *d;
 
protected:

};

#endif

