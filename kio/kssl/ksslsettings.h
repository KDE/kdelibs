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

#ifndef _KSSLSETTINGS_H
#define _KSSLSETTINGS_H

#include <qstring.h>
#include <qvaluelist.h>
#include <kconfig.h>

class KSSLSettingsPrivate;

class KSSLSettings {
public:
  KSSLSettings(bool readConfig = true);
  ~KSSLSettings();

  bool sslv2() const;
  bool sslv3() const;
  bool tlsv1() const;

  bool warnOnEnter() const;
  bool warnOnUnencrypted() const;
  void setWarnOnUnencrypted(bool x);
  bool warnOnLeave() const;
  bool warnOnMixed() const;
  bool warnOnSelfSigned() const;
  bool warnOnRevoked() const;
  bool warnOnExpired() const;

  bool useEGD() const;
  bool useEFile() const;
  
  void setTLSv1(bool enabled);
  void setSSLv2(bool enabled);
  void setSSLv3(bool enabled);

  bool autoSendX509() const;
  bool promptSendX509() const;

  // Returns the OpenSSL cipher list for selecting the list of ciphers to
  // use in a connection
  QString getCipherList();

  QString& getEGDPath();

  void load();
  void defaults();
  void save();

private:
  KConfig *m_cfg;
  bool m_bUseSSLv2, m_bUseSSLv3, m_bUseTLSv1;
  bool m_bWarnOnEnter, m_bWarnOnUnencrypted, m_bWarnOnLeave, m_bWarnOnMixed;
  bool m_bWarnSelfSigned, m_bWarnRevoked, m_bWarnExpired;

  QValueList<QString> v2ciphers, v2selectedciphers,
                      v3ciphers, v3selectedciphers;
  QValueList<int>     v2bits, v3bits;

  KSSLSettingsPrivate *d;
};


#endif

