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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ksslsettings.h"

// this hack provided by Malte Starostik to avoid glibc/openssl bug
// on some systems
#ifdef HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#undef crypt
#endif


//
// FIXME
// Implementation note: for now, we only read cipher settings from disk,
//                      and do not store them in memory.  This should change.
//

KSSLSettings::KSSLSettings(bool readConfig) {
  m_cfg = new KConfig("cryptodefaults");
  if (readConfig) load();
}


// we don't save settings incase it was a temporary object
KSSLSettings::~KSSLSettings() {
  delete m_cfg;
}
 

bool KSSLSettings::sslv2() {
  return m_bUseSSLv2;
}


bool KSSLSettings::sslv3() {
  return m_bUseSSLv3;
}


bool KSSLSettings::tlsv1() {
  return m_bUseTLSv1;
}

 
QString KSSLSettings::getCipherList() {
QString clist = "";
#ifdef HAVE_SSL
  if (!m_bUseTLSv1) {
    QString tcipher;
    bool firstcipher = true;
    bool v2ciphers_unset = false, v3ciphers_unset = false;
    SSL_METHOD *meth;

    if (m_bUseTLSv1)
      meth = TLSv1_client_method();
    else if (m_bUseSSLv2 && m_bUseSSLv3)
      meth = SSLv23_client_method();
    else if (m_bUseSSLv3)
      meth = SSLv3_client_method();
    else
      meth = SSLv2_client_method();
 
    // The user might have v2 and v3 enabled so we start with an
    // empty buffer and add v2 if needed, then v3 if needed.
    // we assume that the config file will have consecutive entries.
    for (int k = 0; k < 2; k++) {
 
      if (k == 0) {                   // do v2, then v3
        if (!m_bUseSSLv2) continue;
        m_cfg->setGroup("SSLv2");
      } else {
        if (!m_bUseSSLv3) continue;
        m_cfg->setGroup("SSLv3");
      }
 
      for(int i = 0;; i++) {
        tcipher.sprintf("cipher_%d", i);
 
        if (!m_cfg->hasKey(tcipher)) {
          if (i == 0)
            if (k == 0)
              v2ciphers_unset = true;
            else v3ciphers_unset = true;
          break;
        } // if
 
        if (m_cfg->readBoolEntry(tcipher)) {  // add it to the list
          SSL_CIPHER *sc = (meth->get_cipher)(i);
          if (!sc) continue;
 
          if (firstcipher)          // we don't start with a ':'
            firstcipher = false;
          else clist.append(":");
 
          clist.append(sc->name);
        } // if
      } // for  i
    } // for    k
  } // if
#endif
return clist;
}


void KSSLSettings::load() {
  m_cfg->reparseConfiguration();

  m_cfg->setGroup("TLSv1");
  m_bUseTLSv1 = m_cfg->readBoolEntry("Enabled", false);
 
  m_cfg->setGroup("SSLv2");
  m_bUseSSLv2 = m_cfg->readBoolEntry("Enabled", true);
 
  m_cfg->setGroup("SSLv3");
  m_bUseSSLv3 = m_cfg->readBoolEntry("Enabled", true);
 
  m_cfg->setGroup("Warnings");
  m_bWarnOnEnter = m_cfg->readBoolEntry("OnEnter", false);
  m_bWarnOnLeave = m_cfg->readBoolEntry("OnLeave", true);
  m_bWarnOnUnencrypted = m_cfg->readBoolEntry("OnUnencrypted", false);
  m_bWarnOnMixed = m_cfg->readBoolEntry("OnMixed", true);

  // FIXME - ciphers
}


void KSSLSettings::defaults() {
  m_bUseTLSv1 = false;
  m_bUseSSLv2 = true;
  m_bUseSSLv3 = true;
  m_bWarnOnEnter = false;
  m_bWarnOnLeave = true;
  m_bWarnOnUnencrypted = false;
  m_bWarnOnMixed = true;

  // FIXME - ciphers
}


void KSSLSettings::save() {
  m_cfg->setGroup("TLSv1");
  m_cfg->writeEntry("Enabled", m_bUseTLSv1);
 
  m_cfg->setGroup("SSLv2");
  m_cfg->writeEntry("Enabled", m_bUseSSLv2);
 
  m_cfg->setGroup("SSLv3");
  m_cfg->writeEntry("Enabled", m_bUseSSLv3);
 
  m_cfg->setGroup("Warnings");
  m_cfg->writeEntry("OnEnter", m_bWarnOnEnter);
  m_cfg->writeEntry("OnLeave", m_bWarnOnLeave);
  m_cfg->writeEntry("OnUnencrypted", m_bWarnOnUnencrypted);
  m_cfg->writeEntry("OnMixed", m_bWarnOnMixed);

  // FIXME - ciphers
}


