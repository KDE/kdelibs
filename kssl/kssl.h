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

#ifndef _KSSL_H
#define _KSSL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SSL
#include <openssl/ssl.h>
#endif

#include <ksslsettings.h>
#include <ksslpeerinfo.h>
#include <ksslconnectioninfo.h>

class KSSL {

public:
  KSSL(bool init = true);

  ~KSSL();

  bool initialize();
  void close();
  bool reInitialize();

  bool reconfig();
  void setAutoReconfig(bool ar);
  bool setSettings(KSSLSettings *settings);

  int connect(int sock);

  int read(void *buf, int len);
  int write(const void *buf, int len);

private:
#ifdef HAVE_SSL
  SSL *m_ssl;
  SSL_CTX *m_ctx;
  SSL_METHOD *m_meth;
#endif
  bool m_bInit;
  bool m_bAutoReconfig;
  KSSLSettings *m_cfg;
//  KSSLConnectionInfo *m_ci;
//  KSSLPeerInfo *m_pi;
};


#endif

