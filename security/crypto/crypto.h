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
#include <qlistview.h>
#include <qlineedit.h>
// #include <qvaluelist.h>

class KCryptoConfig;

class CipherItem : public QCheckListItem
{
public:
    CipherItem( QListView *view, const QString& cipher, int bits, int maxBits,
		KCryptoConfig *module );
    ~CipherItem() {}

    void setCipher( const QString& cipher ) { m_cipher = cipher; }
    const QString& cipher() const { return m_cipher; }
    void setBits( int bits ) { m_bits = bits; }
    int bits() const { return m_bits; }

    QString configName() const;

protected:
    virtual void stateChange( bool );

private:
    int m_bits;
    QString m_cipher;
    KCryptoConfig *m_module; // just to call configChanged()
};

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

  void slotGeneratePersonal();
  void slotUseEGD();
  void slotChooseEGD();
  void slotChooseOSSL();
  void slotCWcompatible();
  void slotCWus();
  void slotCWexp();
  void slotCWall();

  void slotTestOSSL();

private:
  QTabWidget *tabs;
  QWidget *tabSSL, *tabOSSL;

  QWidget *tabYourSSLCert, *tabOtherSSLCert, *tabSSLCA, *tabSSLCOpts;

  QListView *SSLv2Box, *SSLv3Box;
  QCheckBox *mUseTLS, *mUseSSLv2, *mUseSSLv3;
  QCheckBox *mWarnOnEnter, *mWarnOnLeave;

  /* EGD stuff */
  QPushButton *mChooseEGD;
  QLabel    *mEGDLabel;
  QLineEdit *mEGDPath;
  QCheckBox *mUseEGD;

  /* CipherWizards */
  QPushButton *mCWall, *mCWus, *mCWexp, *mCWcompatible;

  QCheckBox *mWarnOnUnencrypted, *mWarnOnMixed;
  QListBox *yourSSLBox, *otherSSLBox, *caSSLBox;
  QCheckBox *mWarnSelfSigned, *mWarnExpired, *mWarnRevoked;
  QPushButton *macAdd, *macRemove, *macClear;
  QListBox *macBox;
  QPushButton *otherSSLImport, *otherSSLView, *otherSSLRemove, *otherSSLVerify;
  QPushButton *caSSLImport, *caSSLView, *caSSLRemove, *caSSLVerify;
  QPushButton *yourSSLImport, *yourSSLView, *yourSSLRemove, *yourSSLExport,
              *yourSSLDefault, *yourSSLVerify;
  QRadioButton *yourSSLUseDefault, *yourSSLList, *yourSSLDont;
  QLineEdit *macCert;

  /* OpenSSL tab */
  QLabel    *oInfo;
  QLineEdit *oPath;
  QPushButton *oFind;
  QPushButton *oTest;


  KConfig *config;
};

#endif
