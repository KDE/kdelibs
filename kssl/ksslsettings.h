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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#undef crypt
#endif


#include <qstring.h>

#include <kconfig.h>

class KSSLSettings {
public:
  KSSLSettings(bool readConfig = true);
  ~KSSLSettings();

  bool sslv2();
  bool sslv3();
  bool tlsv1();

  QString getCipherList();

  void load();
  void defaults();
  void save();

private:
  KConfig *m_cfg;
  bool m_bUseSSLv2, m_bUseSSLv3, m_bUseTLSv1;
  bool m_bWarnOnEnter, m_bWarnOnUnencrypted, m_bWarnOnLeave, m_bWarnOnMixed;

  class KSSLSettingsPrivate;
  KSSLSettingsPrivate *d;
};


#endif

