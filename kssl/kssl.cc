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


#include "kssl.h"

#include <kdebug.h>

KSSL::KSSL(bool init) {
  m_bInit = false;
  m_bAutoReconfig = true;
  m_cfg = new KSSLSettings();
  if (init) initialize();
}


KSSL::~KSSL() {
  close();
  delete m_cfg;
}


bool KSSL::initialize() {
#ifdef HAVE_SSL
  if (m_bInit) return false;

  if (m_bAutoReconfig)
    m_cfg->load();
  // TLS1 goes first - it excludes SSL2/3
  // FIXME: we should be able to force SSL off entirely.
  //        This logic here makes v2 a "default" if no other SSL
  //        version is turned on.  IMHO this is the safest one to
  //        use as the default anyways, so I'm not changing it yet.
  if (m_cfg->tlsv1())
    m_meth = TLSv1_client_method();
  else if (m_cfg->sslv2() && m_cfg->sslv3())
    m_meth = SSLv23_client_method();
  else if (m_cfg->sslv3())
    m_meth = SSLv3_client_method();
  else
    m_meth = SSLv2_client_method();
 
  SSLeay_add_ssl_algorithms();
  m_ctx=SSL_CTX_new(m_meth);
  if (m_ctx == NULL) {
    return false;
  }

  // set cipher list
  QString clist = m_cfg->getCipherList();
  if (!clist.isEmpty()) 
    SSL_CTX_set_cipher_list(m_ctx, clist.ascii());

  m_bInit = true;
return true;
#else
return false;
#endif
}




void KSSL::close() {
#ifdef HAVE_SSL
  if (!m_bInit) return;
  SSL_shutdown(m_ssl);
  SSL_free(m_ssl);
  SSL_CTX_free(m_ctx);
  m_bInit = false;
#endif
}


bool KSSL::reInitialize() {
  close();
  return initialize();
}


int KSSL::connect(int sock) {
#ifdef HAVE_SSL
int rc;
  if (!m_bInit) return -1;
  m_ssl = SSL_new(m_ctx);
  if (!m_ssl) return -1;
  SSL_set_fd(m_ssl, sock);
  rc = SSL_connect(m_ssl);
  if (rc != -1) {
    setConnectionInfo();
    setPeerInfo();
  }
  return rc;
#else
  return -1;
#endif
}


int KSSL::read(void *buf, int len) {
#ifdef HAVE_SSL
  if (!m_bInit) return -1;
  return SSL_read(m_ssl, (char *)buf, len);
#else
return -1;
#endif
}


int KSSL::write(const void *buf, int len) {
#ifdef HAVE_SSL
  if (!m_bInit) return -1;
  return SSL_write(m_ssl, (const char *)buf, len);
#else
return -1;
#endif
}


bool KSSL::reconfig() {
  return reInitialize();
}


void KSSL::setAutoReconfig(bool ar) {
  m_bAutoReconfig = ar;
}


bool KSSL::setSettings(KSSLSettings *settings) {
  delete m_cfg;
  m_cfg = settings;
  return reconfig();
}


#ifdef HAVE_SSL
bool KSSL::m_bSSLWorks = true;
#else
bool KSSL::m_bSSLWorks = false;
#endif

bool KSSL::doesSSLWork() {
  return m_bSSLWorks;
}


void KSSL::setConnectionInfo() {
#ifdef HAVE_SSL
SSL_CIPHER *sc;
char buf[1024];

  sc = SSL_get_current_cipher(m_ssl);
  // set the number of bits, bits used
  m_ci.m_iCipherUsedBits = SSL_CIPHER_get_bits(sc, &(m_ci.m_iCipherBits));
  // set the cipher version
  m_ci.m_cipherVersion = SSL_CIPHER_get_version(sc);
  // set the cipher name
  m_ci.m_cipherName = SSL_CIPHER_get_name(sc);
  // set the cipher description
  m_ci.m_cipherDescription = SSL_CIPHER_description(sc, buf, 1023);  

#endif
}


void KSSL::setPeerInfo() {
#ifdef HAVE_SSL
  m_pi.m_cert.m_cert = SSL_get_peer_certificate(m_ssl);
#endif
}


const KSSLConnectionInfo& KSSL::connectionInfo() const {
  return m_ci;
}


const KSSLPeerInfo& KSSL::peerInfo() const {
  return m_pi;
}


