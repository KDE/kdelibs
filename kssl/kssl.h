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

#include <ksslsettings.h>
#include <ksslpeerinfo.h>
#include <ksslconnectioninfo.h>

class KSSLPrivate;
class KSSLCertificate;
class KSSLPKCS12;

class KSSL {

public:
  KSSL(bool init = true);

  ~KSSL();

  static bool doesSSLWork();


  bool initialize();

  /**
   * This is used for apps which do STARTTLS or something similar.
   * It creates a TLS method regardless of the user's settings.
   */
  bool TLSInit();

  void close();
  bool reInitialize();

  bool reconfig();
  void setAutoReconfig(bool ar);

  /**
   * This will reseed the PRNG with the EGD if the EGD is configured and
   * enabled.  You don't need to call this yourself normally.
   */
  int seedWithEGD();

  /**
   * Set a new KSSLSettings instance as the settings.
   * This deletes the current instance of KSSLSettings.
   */
  bool setSettings(KSSLSettings *settings);

  /**
   * @return the current settings instance
   * One is built by the constructor, so this will never return 0L.
   */
  KSSLSettings * settings() { return m_cfg; }

  /**
   * @return true if the certificate was properly set to the session.
   * Use this to set the certificate to send to the server.
   * Do NOT delete the KSSLPKCS12 object until you are done with the session.
   * It is not defined when KSSL will be done with this.
   */
  bool setClientCertificate(KSSLPKCS12 *pkcs);

  /**
   * Set the status of the connection with respect to proxies.
   * realIP is the IP address of the host you're connecting to
   * realPort is the port of the host you're connecting to
   * proxy is the IP or hostname of the proxy server
   */
  // DEPRECATED
  void setProxyUse(bool active, QString realIP = QString::null, int realPort = 0, QString proxy = QString::null);

  void setProxy(bool active, QString realIP = QString::null);

  int connect(int sock);

  int read(void *buf, int len);
  int write(const void *buf, int len);

  bool pending();

  KSSLConnectionInfo& connectionInfo();
  KSSLPeerInfo& peerInfo();

private:
  static bool m_bSSLWorks;
  bool m_bInit;
  bool m_bAutoReconfig;
  KSSLSettings *m_cfg;
  KSSLConnectionInfo m_ci;
  KSSLPeerInfo m_pi;

  KSSLPrivate *d;

  void setConnectionInfo();
  void setPeerInfo(int sock);
  bool setVerificationLogic();
};


#endif

