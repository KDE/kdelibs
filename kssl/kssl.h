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

// this hack provided by Malte Starostik to avoid glibc/openssl bug
// on some systems
#ifdef HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#undef crypt
#endif

#include <ksslsettings.h>
#include <ksslpeerinfo.h>
#include <ksslconnectioninfo.h>

class KSSLPrivate;

class KSSL {

public:
  KSSL(bool init = true);

  ~KSSL();

  static bool doesSSLWork();

  bool initialize();
  void close();
  bool reInitialize();

  bool reconfig();
  void setAutoReconfig(bool ar);
  bool setSettings(KSSLSettings *settings);

  int connect(int sock);

  int read(void *buf, int len);
  int write(const void *buf, int len);

  const KSSLConnectionInfo& connectionInfo() const;
  const KSSLPeerInfo& peerInfo() const;

private:
  static bool m_bSSLWorks;
  bool m_bInit;
  bool m_bAutoReconfig;
  KSSLSettings *m_cfg;
  KSSLConnectionInfo m_ci;
  KSSLPeerInfo m_pi;

  KSSLPrivate *d;

  void setConnectionInfo();
  void setPeerInfo();
};


#endif

