/**
 * crypto.cpp
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

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>

#include <qlayout.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qhbuttongroup.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qfileinfo.h>
#include <qcheckbox.h>

#include <kfiledialog.h>
#include <klineedit.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kdialog.h>
#include <kmessagebox.h>

#include <qframe.h>

#include "crypto.h"

#ifdef HAVE_SSL
#include <openssl/err.h>
#include <openssl/ssl.h>
#endif

KCryptoConfig::KCryptoConfig(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
QGridLayout *grid;
QBoxLayout *top = new QVBoxLayout(this);
QString whatstr;

  ///////////////////////////////////////////////////////////////////////////
  // Create the GUI here - there are currently a total of 4 tabs.
  // The first is SSL and cipher related
  // The second is user's SSL certificate related
  // The second is other SSL certificate related
  // The fourth is SSL certificate authority related
  ///////////////////////////////////////////////////////////////////////////

  tabs = new QTabWidget(this);
  top->addWidget(tabs);

  ///////////////////////////////////////////////////////////////////////////
  // FIRST TAB
  ///////////////////////////////////////////////////////////////////////////
  tabSSL = new QFrame(this);
  grid = new QGridLayout(tabSSL, 9, 2);
  mUseTLS = new QCheckBox(i18n("Use &TLS instead of SSLv2/v3"), tabSSL);
  connect(mUseTLS, SIGNAL(clicked()), SLOT(configChanged()));
  grid->addWidget(mUseTLS, 0, 0);
  whatstr = i18n("TLS is an older protocol and should only be used if"
                " SSL v2 or v3 protocols are unavailable or unusable."
                "  Enabling TLS disables SSLv2 and SSLv3.");
  QWhatsThis::add(mUseTLS, whatstr);

  mUseSSLv2 = new QCheckBox(i18n("Enable SSLv&2"), tabSSL);
  connect(mUseSSLv2, SIGNAL(clicked()), SLOT(configChanged()));
  grid->addWidget(mUseSSLv2, 1, 0);
  whatstr = i18n("SSL v2 is the second revision of the SSL protocol."
                " It is most common to enable v2 and v3.");
  QWhatsThis::add(mUseSSLv2, whatstr);

  mUseSSLv3 = new QCheckBox(i18n("Enable SSLv&3"), tabSSL);
  connect(mUseSSLv3, SIGNAL(clicked()), SLOT(configChanged()));
  grid->addWidget(mUseSSLv3, 1, 1);
  whatstr = i18n("SSL v3 is the second revision of the SSL protocol."
                " It is most common to enable v2 and v3.");
  QWhatsThis::add(mUseSSLv3, whatstr);

#ifdef HAVE_SSL
  QLabel *cipherlabel = new QLabel(i18n("SSLv2 Ciphers To Use:"), tabSSL);
  grid->addWidget(cipherlabel, 2, 0);
  
  SSLv2Box = new QListBox(tabSSL, "v2ciphers");
  connect(SSLv2Box, SIGNAL(selectionChanged()), SLOT(configChanged()));
  whatstr = i18n("Select the ciphers you wish to enable when using the"
                " SSL v2 protocol.  The actual protocol used will be"
                " negotiated with the server at connection time.");
  QWhatsThis::add(SSLv2Box, whatstr);
  SSLv2Box->setSelectionMode(QListBox::Multi);
  SSLv2Box->setColumnMode(QListBox::FixedNumber);

  grid->addMultiCellWidget(SSLv2Box, 3, 5, 0, 0);
  SSLv2Box->resize(SSLv2Box->sizeHint());

#else
  QLabel *nossllabel = new QLabel(i18n("SSL ciphers cannot be configured"
                               " because this module was not linked"
                               " with OpenSSL."), tabSSL); 
  grid->addMultiCellWidget(nossllabel, 3, 3, 0, 1);
#endif

#ifdef HAVE_SSL
          cipherlabel = new QLabel(i18n("SSLv3 Ciphers To Use:"), tabSSL);
  grid->addWidget(cipherlabel, 2, 1);

  SSLv3Box = new QListBox(tabSSL, "v3ciphers");
  connect(SSLv3Box, SIGNAL(selectionChanged()), SLOT(configChanged()));
  whatstr = i18n("Select the ciphers you wish to enable when using the"
                " SSL v3 protocol.  The actual protocol used will be"
                " negotiated with the server at connection time.");
  QWhatsThis::add(SSLv3Box, whatstr);
  SSLv3Box->setSelectionMode(QListBox::Multi);
  SSLv3Box->setColumnMode(QListBox::FixedNumber);
  grid->addMultiCellWidget(SSLv3Box, 3, 5, 1, 1);

  loadCiphers();
#endif

  mWarnOnEnter = new QCheckBox(i18n("Warn on &entering SSL mode"), tabSSL);
  connect(mWarnOnEnter, SIGNAL(clicked()), SLOT(configChanged()));
  grid->addWidget(mWarnOnEnter, 7, 0);
  whatstr = i18n("If selected, you will be notified when entering an SSL"
                " enabled site");
  QWhatsThis::add(mWarnOnEnter, whatstr);

  mWarnOnLeave = new QCheckBox(i18n("Warn on &leaving SSL mode"), tabSSL);
  connect(mWarnOnLeave, SIGNAL(clicked()), SLOT(configChanged()));
  grid->addWidget(mWarnOnLeave, 7, 1);
  whatstr = i18n("If selected, you will be notified when leaving an SSL"
                " based site.");
  QWhatsThis::add(mWarnOnLeave, whatstr);

  mWarnOnUnencrypted = new QCheckBox(i18n("Warn on sending &unencrypted data"), tabSSL);
  connect(mWarnOnUnencrypted, SIGNAL(clicked()), SLOT(configChanged()));
  grid->addWidget(mWarnOnUnencrypted, 8, 0);
  whatstr = i18n("If selected, you will be notified before sending"
                " unencrypted data via a web browser.");
  QWhatsThis::add(mWarnOnUnencrypted, whatstr);

  mWarnOnMixed = new QCheckBox(i18n("Warn on &mixed SSL/non-SSL pages"), tabSSL);
  connect(mWarnOnMixed, SIGNAL(clicked()), SLOT(configChanged()));
  grid->addWidget(mWarnOnMixed, 8, 1);
  whatstr = i18n("If selected, you will be notified if you view a page"
                " that has both encrypted and non-encrypted parts.");
  QWhatsThis::add(mWarnOnMixed, whatstr);

  ///////////////////////////////////////////////////////////////////////////
  // SECOND TAB
  ///////////////////////////////////////////////////////////////////////////
  tabYourSSLCert = new QFrame(this);

#ifdef HAVE_SSL
  grid = new QGridLayout(tabYourSSLCert, 10, 2);

  yourSSLBox = new QListBox(tabYourSSLCert);
  whatstr = i18n("This list box shows which certificates of yours KDE"
                " knows about.  You can easily manage them from here.");
  QWhatsThis::add(yourSSLBox, whatstr);
  yourSSLBox->setSelectionMode(QListBox::Single);
  yourSSLBox->setColumnMode(QListBox::FixedNumber);
  grid->addMultiCellWidget(yourSSLBox, 0, 7, 0, 0);

  yourSSLImport = new QPushButton(i18n("&Import..."), tabYourSSLCert);
  //connect(yourSSLImport, SIGNAL(), SLOT());
  grid->addWidget(yourSSLImport, 0, 1);

  yourSSLView = new QPushButton(i18n("&View/Edit..."), tabYourSSLCert);
  //connect(yourSSLAdd, SIGNAL(), SLOT());
  grid->addWidget(yourSSLView, 1, 1);

  yourSSLRemove = new QPushButton(i18n("&Remove..."), tabYourSSLCert);
  //connect(yourSSLAdd, SIGNAL(), SLOT());
  grid->addWidget(yourSSLRemove, 2, 1);

  yourSSLExport = new QPushButton(i18n("&Export..."), tabYourSSLCert);
  //connect(yourSSLAdd, SIGNAL(), SLOT());
  grid->addWidget(yourSSLExport, 3, 1);

  yourSSLDefault = new QPushButton(i18n("&Set Default..."), tabYourSSLCert);
  //connect(yourSSLAdd, SIGNAL(), SLOT());
  grid->addWidget(yourSSLDefault, 4, 1);

  yourSSLVerify = new QPushButton(i18n("Verif&y..."), tabYourSSLCert);
  //connect(yourSSLAdd, SIGNAL(), SLOT());
  grid->addWidget(yourSSLVerify, 5, 1);

  QHButtonGroup *ocbg = new QHButtonGroup(i18n("On SSL Connection..."), tabYourSSLCert);
  yourSSLUseDefault = new QRadioButton(i18n("&Use default certificate"), ocbg);
  yourSSLList = new QRadioButton(i18n("&List upon connection"), ocbg);
  yourSSLDont = new QRadioButton(i18n("&Do not use certificates"), ocbg);
  grid->addMultiCellWidget(ocbg, 9, 9, 0, 1);
#else
  nossllabel = new QLabel(i18n("SSL certificates cannot be managed"
                               " because this module was not linked"
                               " with OpenSSL."), tabYourSSLCert);
  grid->addMultiCellWidget(nossllabel, 3, 3, 0, 1);
#endif


  ///////////////////////////////////////////////////////////////////////////
  // THIRD TAB
  ///////////////////////////////////////////////////////////////////////////
  tabOtherSSLCert = new QFrame(this);

#ifdef HAVE_SSL
  grid = new QGridLayout(tabOtherSSLCert, 8, 2);

  otherSSLBox = new QListBox(tabOtherSSLCert);
  whatstr = i18n("This list box shows which site and person certificates KDE"
                " knows about.  You can easily manage them from here.");
  QWhatsThis::add(otherSSLBox, whatstr);
  otherSSLBox->setSelectionMode(QListBox::Single);
  otherSSLBox->setColumnMode(QListBox::FixedNumber);
  grid->addMultiCellWidget(otherSSLBox, 0, 7, 0, 0);

  otherSSLImport = new QPushButton(i18n("&Import..."), tabOtherSSLCert);
  //connect(otherSSLImport, SIGNAL(), SLOT());
  grid->addWidget(otherSSLImport, 0, 1);

  otherSSLView = new QPushButton(i18n("&View/Edit..."), tabOtherSSLCert);
  //connect(otherSSLView, SIGNAL(), SLOT());
  grid->addWidget(otherSSLView, 1, 1);

  otherSSLRemove = new QPushButton(i18n("&Remove..."), tabOtherSSLCert);
  //connect(otherSSLRemove, SIGNAL(), SLOT());
  grid->addWidget(otherSSLRemove, 2, 1);

  otherSSLVerify = new QPushButton(i18n("Verif&y..."), tabOtherSSLCert);
  //connect(otherSSLVerify, SIGNAL(), SLOT());
  grid->addWidget(otherSSLVerify, 3, 1);

#else
  nossllabel = new QLabel(i18n("SSL certificates cannot be managed"
                               " because this module was not linked"
                               " with OpenSSL."), tabOtherSSLCert);
  grid->addMultiCellWidget(nossllabel, 1, 1, 0, 1);
#endif


  ///////////////////////////////////////////////////////////////////////////
  // FOURTH TAB
  ///////////////////////////////////////////////////////////////////////////
  tabSSLCA = new QFrame(this);

#ifdef HAVE_SSL
  grid = new QGridLayout(tabSSLCA, 8, 2);

  caSSLBox = new QListBox(tabSSLCA);
  whatstr = i18n("This list box shows which certificate authorities KDE"
                " knows about.  You can easily manage them from here.");
  QWhatsThis::add(caSSLBox, whatstr);
  caSSLBox->setSelectionMode(QListBox::Single);
  caSSLBox->setColumnMode(QListBox::FixedNumber);
  grid->addMultiCellWidget(caSSLBox, 0, 7, 0, 0);

  caSSLImport = new QPushButton(i18n("&Import..."), tabSSLCA);
  //connect(caSSLImport, SIGNAL(), SLOT());
  grid->addWidget(caSSLImport, 0, 1);

  caSSLView = new QPushButton(i18n("&View/Edit..."), tabSSLCA);
  //connect(caSSLView, SIGNAL(), SLOT());
  grid->addWidget(caSSLView, 1, 1);

  caSSLRemove = new QPushButton(i18n("&Remove..."), tabSSLCA);
  //connect(caSSLRemove, SIGNAL(), SLOT());
  grid->addWidget(caSSLRemove, 2, 1);

  caSSLVerify = new QPushButton(i18n("Verif&y..."), tabSSLCA);
  //connect(caSSLVerify, SIGNAL(), SLOT());
  grid->addWidget(caSSLVerify, 3, 1);

#else
  nossllabel = new QLabel(i18n("SSL certificates cannot be managed"
                               " because this module was not linked"
                               " with OpenSSL."), tabSSLCA);
  grid->addMultiCellWidget(nossllabel, 1, 1, 0, 1);
#endif


  ///////////////////////////////////////////////////////////////////////////
  // Add the tabs and startup
  ///////////////////////////////////////////////////////////////////////////
  tabs->addTab(tabSSL, i18n("SSL"));
  tabs->addTab(tabYourSSLCert, i18n("Your SSL Certificates"));
  tabs->addTab(tabOtherSSLCert, i18n("Other SSL Certificates"));
  tabs->addTab(tabSSLCA, i18n("SSL C.A.s"));

  tabs->resize(tabs->sizeHint());
  load();
}

KCryptoConfig::~KCryptoConfig()
{
}

void KCryptoConfig::configChanged()
{
    emit changed(true);
}


void KCryptoConfig::load()
{
  KConfig *config = new KConfig("cryptodefaults");
#if 0
  bGrp->setButton(config->readNumEntry("ServerType", 0));
#endif
  config->setGroup("TLSv1");
  mUseTLS->setChecked(config->readBoolEntry("Enabled", false));

  config->setGroup("SSLv2");
  mUseSSLv2->setChecked(config->readBoolEntry("Enabled", true));

  config->setGroup("SSLv3");
  mUseSSLv3->setChecked(config->readBoolEntry("Enabled", true));

  config->setGroup("Warnings");
  mWarnOnEnter->setChecked(config->readBoolEntry("OnEnter", false));
  mWarnOnLeave->setChecked(config->readBoolEntry("OnLeave", true));
  mWarnOnUnencrypted->setChecked(config->readBoolEntry("OnUnencrypted", false));
  mWarnOnMixed->setChecked(config->readBoolEntry("OnMixed", true));

#ifdef HAVE_SSL
  config->setGroup("SSLv2");
  for (unsigned int i = 0; i < SSLv2Box->count(); i++) {
    QString ciphername;
    ciphername.sprintf("cipher_%d", i);
    SSLv2Box->setSelected(i, config->readBoolEntry(ciphername, true));
  }

  config->setGroup("SSLv3");
  for (unsigned int i = 0; i < SSLv3Box->count(); i++) {
    QString ciphername;
    ciphername.sprintf("cipher_%d", i);
    SSLv3Box->setSelected(i, config->readBoolEntry(ciphername, true));
  }
#endif

  emit changed(false);
  delete config;
}

void KCryptoConfig::save()
{
  KConfig *config = new KConfig("cryptodefaults");

  config->setGroup("TLSv1");
  config->writeEntry("Enabled", mUseTLS->isChecked());

  config->setGroup("SSLv2");
  config->writeEntry("Enabled", mUseSSLv2->isChecked());
  
  config->setGroup("SSLv3");
  config->writeEntry("Enabled", mUseSSLv3->isChecked());
  
  config->setGroup("Warnings");
  config->writeEntry("OnEnter", mWarnOnEnter->isChecked());
  config->writeEntry("OnLeave", mWarnOnLeave->isChecked());
  config->writeEntry("OnUnencrypted", mWarnOnUnencrypted->isChecked());
  config->writeEntry("OnMixed", mWarnOnMixed->isChecked());
  
#ifdef HAVE_SSL
  int ciphercount = 0;
  config->setGroup("SSLv2");
  for (unsigned int i = 0; i < SSLv2Box->count(); i++) {
    QString ciphername;
    ciphername.sprintf("cipher_%d", i);
    if (SSLv2Box->isSelected(i)) {
      config->writeEntry(ciphername, true);
      ciphercount++;
    } else config->writeEntry(ciphername, false);
  }

  if (ciphercount == 0)
    KMessageBox::information(this, i18n("If you don't select at least one"
                                       " cipher, SSLv2 will not work."),
                                   i18n("SSLv2 Ciphers"));

  ciphercount = 0;
  config->setGroup("SSLv3");
  for (unsigned int i = 0; i < SSLv3Box->count(); i++) {
    QString ciphername;
    ciphername.sprintf("cipher_%d", i);
    if (SSLv3Box->isSelected(i)) {
      config->writeEntry(ciphername, true);
      ciphercount++;
    } else config->writeEntry(ciphername, false);
  }

  if (ciphercount == 0)
    KMessageBox::information(this, i18n("If you don't select at least one"
                                       " cipher, SSLv3 will not work."),
                                   i18n("SSLv3 Ciphers"));
#endif

  config->sync();

  // insure proper permissions -- contains sensitive data
  QString cfgName(KGlobal::dirs()->findResource("config", "kcmcryptorc"));
  if (!cfgName.isEmpty())
    ::chmod(cfgName.utf8(), 0600);

  emit changed(false);
  delete config;
}

void KCryptoConfig::defaults()
{
  mUseTLS->setChecked(false);
  mUseSSLv2->setChecked(true);
  mUseSSLv3->setChecked(true);
  mWarnOnEnter->setChecked(false);
  mWarnOnLeave->setChecked(true);
  mWarnOnUnencrypted->setChecked(false);
  mWarnOnMixed->setChecked(true);

#ifdef HAVE_SSL
  for (unsigned int i = 0; i < SSLv2Box->count(); i++)
    SSLv2Box->setSelected(i, true);
  for (unsigned int i = 0; i < SSLv3Box->count(); i++)
    SSLv3Box->setSelected(i, true);
#endif

  emit changed(true);
}

QString KCryptoConfig::quickHelp() const
{
  return i18n("<h1>crypto</h1> This module allows you to configure SSL for"
     " use with most KDE applications, as well as manage your personal"
     " certificates and the known certificate authorities.");
}

#ifdef HAVE_SSL
// This gets all the available ciphers from OpenSSL
bool KCryptoConfig::loadCiphers() {
int i;
SSL_CTX *ctx;
SSL *ssl;
SSL_METHOD *meth;
 
  meth = SSLv2_client_method();
  OpenSSL_add_ssl_algorithms();
  ctx = SSL_CTX_new(meth);
  if (ctx == NULL) return false;
 
  ssl = SSL_new(ctx);
  if (!ssl) return false;
 
  for (i=0; ; i++) {
    int j, k;
    SSL_CIPHER *sc;
    QString cname;
    sc = (meth->get_cipher)(i);
    if (!sc) break;
    k = SSL_CIPHER_get_bits(sc, &j);
    cname.sprintf("%s (%d of %d bits)", sc->name, k, j);
    SSLv2Box->insertItem(cname);
  }
 
  if (ctx) SSL_CTX_free(ctx);
  if (ssl) SSL_free(ssl);

  // We repeat for SSLv3
  meth = SSLv3_client_method();
  OpenSSL_add_ssl_algorithms();
  ctx = SSL_CTX_new(meth);
  if (ctx == NULL) return false;
 
  ssl = SSL_new(ctx);
  if (!ssl) return false;
 
  for (i=0; ; i++) {
    int j, k;
    SSL_CIPHER *sc;
    QString cname;
    sc = (meth->get_cipher)(i);
    if (!sc) break;
    k = SSL_CIPHER_get_bits(sc, &j);
    cname.sprintf("%s (%d of %d bits)", sc->name, k, j);
    SSLv3Box->insertItem(cname);
  }
 
  if (ctx) SSL_CTX_free(ctx);
  if (ssl) SSL_free(ssl);

return true;
}
#endif

extern "C"
{
  KCModule *create_crypto(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kcmcrypto");
    return new KCryptoConfig(parent, name);
  };
}
#include "crypto.moc"
