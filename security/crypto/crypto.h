/**
 * crypto.h
 *
 * Copyright (c) 2000 George Staikos <staikos@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _CRYPTO_H
#define _CRYPTO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kcmodule.h>

#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qvbuttongroup.h>
#include <qlistbox.h>
#include <qlineedit.h>

class KCryptoConfig : public KCModule
{
  Q_OBJECT
public:
  KCryptoConfig(QWidget *parent = 0L, const char *name = 0L);
  virtual ~KCryptoConfig();

  void load();
  void save();
  void defaults();
  
  int buttons();
  QString quickHelp() const;

#ifdef HAVE_SSL
  bool loadCiphers();
#endif
  
public slots:
  void configChanged();
      
private:
  QTabWidget *tabs;
  QWidget *tabSSL, *tabYourSSLCert, *tabOtherSSLCert, *tabSSLCA, *tabSSLCOpts;
  QCheckBox *mUseTLS, *mUseSSLv2, *mUseSSLv3;
  QCheckBox *mWarnOnEnter, *mWarnOnLeave, *mWarnOnUnencrypted, *mWarnOnMixed;
  QListBox *SSLv2Box, *SSLv3Box, *yourSSLBox, *otherSSLBox, *caSSLBox;
  QCheckBox *mWarnSelfSigned, *mWarnExpired, *mWarnRevoked;
  QPushButton *macAdd, *macRemove, *macClear;
  QListBox *macBox;
  QPushButton *otherSSLImport, *otherSSLView, *otherSSLRemove, *otherSSLVerify;
  QPushButton *caSSLImport, *caSSLView, *caSSLRemove, *caSSLVerify;
  QPushButton *yourSSLImport, *yourSSLView, *yourSSLRemove, *yourSSLExport,
              *yourSSLDefault, *yourSSLVerify;
  QRadioButton *yourSSLUseDefault, *yourSSLList, *yourSSLDont;
  QLineEdit *macCert;

  
};

#endif
