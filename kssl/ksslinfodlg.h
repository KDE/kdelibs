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

#ifndef _KSSLINFODLG_H
#define _KSSLINFODLG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
 
#ifdef HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#undef crypt
#endif
 
#include <qwidget.h>
#include <kdialog.h>
#include "ksslx509map.h"
#include <qlabel.h>

 
class KSSLInfoDlg : public KDialog {

  Q_OBJECT 

public:
  KSSLInfoDlg(bool secureConnection, QWidget *parent = 0, const char *name = 0);
  virtual ~KSSLInfoDlg();

  void setup(const QString& peername, const QString& issuer,
             const QString& ip, const QString& url,
             const QString& cipher, const QString& cipherdesc,
             const QString& sslversion, int usedbits, int bits);
 
private:
 
  class KSSLInfoDlgPrivate;
  KSSLInfoDlgPrivate *d;

  void interfaceNoSSL();
  void interfaceSSLBlank();
  void interfaceSSL();

  bool m_secCon;
  KSSLX509Map m_peer, m_issuer;
  QString m_ip, m_url, m_cipher, m_cipherdesc, m_sslversion;
  int m_usedbits, m_bits;
  QLabel *m_lip, *m_lurl, *m_lcipher, *m_lcipherdesc,
         *m_lsslversion, *m_lbits;
  QLabel *m_lpO, *m_lpOU, *m_lpL, *m_lpCN, *m_lpST, *m_lpEMAIL,
         *m_lpC, *m_liO, *m_liOU, *m_liL, *m_liCN, *m_liST, *m_liEMAIL,
         *m_liC;


protected:


private slots:
  void launchConfig();

};

#endif

