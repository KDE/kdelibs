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
class KSSLPKCS12;
#include <qstring.h>
#include <qstringlist.h>


class KSSLCertificateHome {
 
public:

  /*
   *  These methods might dynamically allocate an object for you.  Be sure to
   *  delete them when you are done.
   */
  static KSSLPKCS12* getCertificateByHost(QString host, QString password, bool* send = NULL, bool *prompt = NULL);
  static KSSLPKCS12* getCertificateByName(QString name, QString password);
  static KSSLPKCS12* getCertificateByName(QString name);
  static QString getDefaultCertificateName(QString host, bool *send = NULL, bool *prompt = NULL);
  static QString getDefaultCertificateName(bool *send = NULL, bool *prompt = NULL);
  static KSSLPKCS12* getDefaultCertificate(QString password, bool *send = NULL, bool *prompt = NULL);
  static KSSLPKCS12* getDefaultCertificate(bool *send = NULL, bool *prompt = NULL);


  /*
   *   These set the default certificate for hosts without a policy.
   */
  static void setDefaultCertificate(QString name);
  static void setDefaultCertificate(KSSLPKCS12 *cert);


  /*
   *   These set the default certificate for a host.
   */
  static void setDefaultCertificate(QString name, QString host);
  static void setDefaultCertificate(KSSLPKCS12 *cert, QString host);

  /*
   *   These add a certificate to the repository.
   *   Returns: true on success, false error
   */
  static bool addCertificate(QString filename, QString password, bool storePass = false);
  static void addCertificate(KSSLPKCS12 *cert, QString passToStore = "");
 
  /*
   *   Returns the list of certificates available
   */
  static QStringList getCertificateList();


private:
 
  class KSSLCertificateHomePrivate;
  KSSLCertificateHomePrivate *d;
 
protected:

};

#endif

