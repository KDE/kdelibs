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

#include <sys/types.h>
#include <sys/stat.h>
 
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>


#include "ksslsettings.h"
#include <kglobal.h>
#include <kstddirs.h>

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
 

bool KSSLSettings::sslv2() const {
  return m_bUseSSLv2;
}


bool KSSLSettings::sslv3() const {
  return m_bUseSSLv3;
}


bool KSSLSettings::tlsv1() const {
  return m_bUseTLSv1;
}

 
QString KSSLSettings::getCipherList() {
QString clist = "";
#ifdef HAVE_SSL
  if (!m_bUseTLSv1) {
    QString tcipher;
    bool firstcipher = true;
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
        SSL_CIPHER *sc = (meth->get_cipher)(i);
        if (!sc) break;;
        tcipher.sprintf("cipher_%s", sc->name);
 
        if (m_cfg->readBoolEntry(tcipher)) {
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

// FIXME - sync these up so that we can use them with the control module!!
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

  m_cfg->setGroup("Validation");
  m_bWarnSelfSigned = m_cfg->readBoolEntry("WarnSelfSigned", true);
  m_bWarnExpired = m_cfg->readBoolEntry("WarnExpired", true);
  m_bWarnRevoked = m_cfg->readBoolEntry("WarnRevoked", true);

  #ifdef HAVE_SSL



  #endif
}


void KSSLSettings::defaults() {
  m_bUseTLSv1 = false;
  m_bUseSSLv2 = true;
  m_bUseSSLv3 = true;
  m_bWarnOnEnter = false;
  m_bWarnOnLeave = true;
  m_bWarnOnUnencrypted = false;
  m_bWarnOnMixed = true;
  m_bWarnSelfSigned = true;
  m_bWarnExpired = true;
  m_bWarnRevoked = true;

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

  m_cfg->setGroup("Validation");
  m_cfg->writeEntry("WarnSelfSigned", m_bWarnSelfSigned);
  m_cfg->writeEntry("WarnExpired", m_bWarnExpired);
  m_cfg->writeEntry("WarnRevoked", m_bWarnRevoked);

  // FIXME - ciphers
#if 0
#ifdef HAVE_SSL
  m_cfg->setGroup("SSLv2");
  for (unsigned int i = 0; i < v2ciphers.count(); i++) {
    QString ciphername;
    ciphername.sprintf("cipher_%s", v2ciphers[i].ascii());
    if (v2selectedciphers.contains(v2ciphers[i])) {
      m_cfg->writeEntry(ciphername, true);
    } else m_cfg->writeEntry(ciphername, false);
  }
 
  m_cfg->setGroup("SSLv3");
  for (unsigned int i = 0; i < v3ciphers.count(); i++) {
    QString ciphername;
    ciphername.sprintf("cipher_%s", v3ciphers[i].ascii());
    if (v3selectedciphers.contains(v3ciphers[i])) {
      m_cfg->writeEntry(ciphername, true);
    } else m_cfg->writeEntry(ciphername, false);
  }
#endif 
 
  m_cfg->sync();
 
  // insure proper permissions -- contains sensitive data
  QString cfgName(KGlobal::dirs()->findResource("config", "kcmcryptorc"));
  if (!cfgName.isEmpty())
    ::chmod(cfgName.utf8(), 0600);
#endif
}


bool KSSLSettings::warnOnEnter() const       { return m_bWarnOnEnter; }
bool KSSLSettings::warnOnUnencrypted() const { return m_bWarnOnUnencrypted; }
bool KSSLSettings::warnOnLeave() const       { return m_bWarnOnLeave; }
bool KSSLSettings::warnOnMixed() const       { return m_bWarnOnMixed; }
bool KSSLSettings::warnOnSelfSigned() const  { return m_bWarnSelfSigned; }
bool KSSLSettings::warnOnRevoked() const     { return m_bWarnRevoked; }
bool KSSLSettings::warnOnExpired() const     { return m_bWarnExpired; }

void KSSLSettings::setTLSv1(bool enabled) { m_bUseTLSv1 = enabled; }
void KSSLSettings::setSSLv2(bool enabled) { m_bUseSSLv2 = enabled; }
void KSSLSettings::setSSLv3(bool enabled) { m_bUseSSLv3 = enabled; }



