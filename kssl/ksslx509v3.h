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
 
#ifndef _KSSLX509V3_H
#define _KSSLX509V3_H

#include <qstring.h>


class KSSLX509V3 {
friend class KSSLCertificate;
friend class KSSLCertificatePrivate;
public:
  ~KSSLX509V3();

  bool certTypeCA();
  bool certTypeSSLCA();
  bool certTypeEmailCA();
  bool certTypeCodeCA();
  bool certTypeSSLClient();
  bool certTypeSSLServer();
  bool certTypeNSSSLServer();
  bool certTypeSMIME();
  bool certTypeSMIMEEncrypt();
  bool certTypeSMIMESign();
  bool certTypeCRLSign();

private:
  class KSSLX509V3Private;
  KSSLX509V3Private *d;

protected:
  KSSLX509V3();
  long flags;    // warning: this is only valid for up to 16 flags + 16 CA.
};

#endif
