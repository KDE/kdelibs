/**
 * crypto.cpp
 *
 * Copyright (c) 2000-2005 George Staikos <staikos@kde.org>
 *               2000 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

//
//   Attention.  The suck factor on this code is increasing.  It's a bit of a
//   hack. </understatement>  It might be time to rewrite it soon.
//

#include "crypto.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QFile>
#include <QFileInfo>
#include <QtGui/QFrame>
#include <QLabel>
#include <QLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QRegExp>

#include <kaboutdata.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdatepicker.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <klineedit.h>
#include <klocale.h>
#include <kcodecs.h>
#include <kmessagebox.h>
#include <kpassworddialog.h>
#include <knewpassworddialog.h>
#include <kprocess.h>
#include <kpushbutton.h>
#include <k3resolver.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>
#include <ktextbrowser.h>
#include <kurllabel.h>
#include <kurlrequester.h>
#include <kvbox.h>

#include <ksslconfig.h>
#ifdef KSSL_HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/stack.h>
#include <openssl/safestack.h>
#undef crypt
#endif

#include <ksslall.h>
#include <kopenssl.h>
#include <ksslcertificatebox.h>

#include "certexport.h"
#include "kdatetimedlg.h"

using namespace KNetwork;

K_PLUGIN_FACTORY(KryptoFactory, registerPlugin<KCryptoConfig>();)
K_EXPORT_PLUGIN(KryptoFactory("kcmcrypto"))

CipherItem::CipherItem( QTreeWidget *view, const QString& cipher, int bits,
			int maxBits, KCryptoConfig *module )
    : QTreeWidgetItem( view )
{
    setCheckState( 0, Qt::Unchecked );
    m_cipher = cipher;
    m_bits = bits;
    m_module = module;

    setText( 0, i18n("%1 (%2 of %3 bits)", cipher, bits, maxBits ) );
}

void CipherItem::stateChange( bool )
{
    m_module->configChanged();
}

QString CipherItem::configName() const
{
    QString cipherName("cipher_%1");
    return cipherName.arg( m_cipher );
}



OtherCertItem::OtherCertItem( QTreeWidget *view, const QString& sub, const QString& md5, bool perm, int policy, QDateTime exp, KCryptoConfig *module )
    : QTreeWidgetItem( view ), _sub(sub), _md5(md5), _exp(exp), _perm(perm), _policy(policy)

{
    m_module = module;
KSSLX509Map cert(sub);
    setText(0, cert.getValue("O"));
    setText(1, cert.getValue("CN").replace('\n', ", "));

    if (_exp.date().year() > 3000 || _exp.date().year() < 1900)
       _exp.setDate(QDate(3000,1,1));
}

void OtherCertItem::stateChange( bool )
{
    m_module->configChanged();
}

QString OtherCertItem::configName() const
{
    return _sub;
}


YourCertItem::YourCertItem( QTreeWidget *view, QString pkcs, QString pass, QString name, KCryptoConfig *module )
    : QTreeWidgetItem( view )

{
    m_module = module;
    KSSLX509Map cert(name);
    QString tmp = cert.getValue("CN").replace('\n', ", ");
    setText(0, tmp);
    setText(1, cert.getValue("Email"));
    _pkcs = pkcs;
    _name = name;
    _pass = pass;
}

void YourCertItem::stateChange( bool )
{
    m_module->configChanged();
}

QString YourCertItem::configName() const
{
    return _name;
}



CAItem::CAItem( QTreeWidget *view, QString name, QString cert, bool site, bool email, bool code, KCryptoConfig *module )
    : QTreeWidgetItem( view )

{
    m_module = module;
KSSLX509Map mcert(name);
QString tmp;
    setText(0, mcert.getValue("O"));
    tmp = mcert.getValue("OU");
    tmp.replace('\n', ", ");
    setText(1, tmp);
    tmp = mcert.getValue("CN");
    tmp.replace('\n', ", ");
    setText(2, tmp);
    _name = name;
    _cert = cert;
    _site = site;
    _email = email;
    _code = code;
    isNew = false;
    modified = false;
}

void CAItem::stateChange( bool )
{
    m_module->configChanged();
}

QString CAItem::configName() const
{
    return _name;
}




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////




KCryptoConfig::KCryptoConfig(QWidget *parent, const QVariantList &)
  : KCModule(KryptoFactory::componentData(), parent)
{
QGridLayout *grid;
QBoxLayout *top = new QVBoxLayout(this);
top->setMargin(0);

QString whatstr;

  setQuickHelp( i18n("<h1>Crypto</h1> This module allows you to configure SSL for"
     " use with most KDE applications, as well as manage your personal"
     " certificates and the known certificate authorities."));

  ___lehack = false;

  _signers = new KSSLSigners;

     KAboutData *about =
     new KAboutData("kcmcrypto", 0, ki18n("KCMCrypto"), 0,
                    ki18n("KDE Crypto Control Module"), KAboutData::License_GPL,
                    ki18n("(c) 2000 - 2001 George Staikos"));
     about->addAuthor(ki18n("George Staikos"), KLocalizedString(), "staikos@kde.org");
     about->addAuthor(ki18n("Carsten Pfeiffer"), KLocalizedString(), "pfeiffer@kde.org");
     setAboutData( about );

  ///////////////////////////////////////////////////////////////////////////
  // Create the GUI here - there are currently a total of 6 tabs.
  // The first is SSL and cipher related
  // The second is OpenSSL settings
  // The third is user's SSL certificate related
  // The fourth is certificate authentication related
  // The fifth is other SSL certificate related
  // The sixth is CA related
  // The seventh is misc. settings related   (unimplemented)
  // The eighth is peer [email] certificate related (unimplemented)
  ///////////////////////////////////////////////////////////////////////////

  tabs = new KTabWidget(this);
  top->addWidget(tabs);

  ///////////////////////////////////////////////////////////////////////////
  // FIRST TAB
  ///////////////////////////////////////////////////////////////////////////
  tabSSL = new QFrame(this);
  grid = new QGridLayout( tabSSL );
  // no need to parse kdeglobals.
  config = new KConfig("cryptodefaults", KConfig::NoGlobals);
  policies = new KConfig("ksslpolicies", KConfig::SimpleConfig);
  pcerts = new KConfig("ksslcertificates", KConfig::SimpleConfig);
  authcfg = new KConfig("ksslauthmap", KConfig::SimpleConfig);

#ifdef KSSL_HAVE_SSL
  SSLv3Box = new QTreeWidget(tabSSL);
  SSLv3Box->setHeaderLabel(i18n("SSL Ciphers to Use"));
  whatstr = i18n("Select the ciphers you wish to enable when using the"
                " SSL protocol. The actual protocol used will be"
                " negotiated with the server at connection time.");
  SSLv3Box->setWhatsThis( whatstr);
  SSLv3Box->setRootIsDecorated(false);
  SSLv3Box->setSelectionMode(QTreeWidget::NoSelection);
  grid->addWidget(SSLv3Box, 2, 0, 1, 2 );
  loadCiphers();
  connect(SSLv3Box, SIGNAL(itemChanged(QTreeWidgetItem *, int)), SLOT(configChanged()));

  //
  //  CipherWizards
  //
  QWidget *cwbg = new QWidget(tabSSL);
  QVBoxLayout *laygroup2 = new QVBoxLayout(cwbg);
  KComboBox *cwcb = new KComboBox(cwbg);
  laygroup2->addWidget( new QLabel(i18n("Cipher Wizard"),this) );
  laygroup2->addWidget( cwcb );
  grid->addWidget(cwbg, 3, 0, 1, 2 );

  cwcb->addItem(QString());
  cwcb->addItem(i18n("Strong Ciphers Only"));
  cwcb->addItem(i18n("Export Ciphers Only"));
  cwcb->addItem(i18n("Enable All"));

  whatstr = i18n("<qt>Use these preconfigurations to more easily configure "
                 "the SSL encryption settings. You can choose among the "
                 "following modes:<ul>"
                 "<li><b>Strong Ciphers Only:</b> Select only the strong "
                 "(&gt;= 128 bit) encryption ciphers.</li>"
                 "<li><b>Export Ciphers Only:</b> Select only the weak "
                 "ciphers (&lt;= 56 bit).</li>"
                 "<li><b>Enable All:</b> Select all SSL ciphers and "
                 "methods.</li>"
                 "</ul></qt>");

  cwcb->setWhatsThis( whatstr);

  connect(cwcb, SIGNAL(activated(int)), SLOT(slotSelectCipher(int)));

#endif

  mWarnOnEnter = new QCheckBox(i18n("Warn on &entering SSL mode"), tabSSL);
  connect(mWarnOnEnter, SIGNAL(toggled(bool)), SLOT(configChanged()));
  grid->addWidget(mWarnOnEnter, 5, 0);
  whatstr = i18n("If selected, you will be notified when entering an SSL"
                " enabled site");
  mWarnOnEnter->setWhatsThis( whatstr);

  mWarnOnLeave = new QCheckBox(i18n("Warn on &leaving SSL mode"), tabSSL);
  connect(mWarnOnLeave, SIGNAL(toggled(bool)), SLOT(configChanged()));
  grid->addWidget(mWarnOnLeave, 5, 1);
  whatstr = i18n("If selected, you will be notified when leaving an SSL"
                " based site.");
  mWarnOnLeave->setWhatsThis( whatstr);

  mWarnOnUnencrypted = new QCheckBox(i18n("Warn on sending &unencrypted data"), tabSSL);
  connect(mWarnOnUnencrypted, SIGNAL(toggled(bool)), SLOT(configChanged()));
  grid->addWidget(mWarnOnUnencrypted, 6, 0);
  whatstr = i18n("If selected, you will be notified before sending"
                " unencrypted data via a web browser.");
  mWarnOnUnencrypted->setWhatsThis( whatstr);

#if 0  // NOT IMPLEMENTED IN KDE 3.0
  mWarnOnMixed = new QCheckBox(i18n("Warn on &mixed SSL/non-SSL pages"), tabSSL);
  connect(mWarnOnMixed, SIGNAL(clicked()), SLOT(configChanged()));
  grid->addWidget(mWarnOnMixed, 6, 1);
  whatstr = i18n("If selected, you will be notified if you view a page"
                " that has both encrypted and non-encrypted parts.");
  mWarnOnMixed->setWhatsThis( whatstr);
#endif

  ///////////////////////////////////////////////////////////////////////////
  // SECOND TAB
  ///////////////////////////////////////////////////////////////////////////

#ifdef KSSL_HAVE_SSL
  tabOSSL = new QFrame(this);
  QBoxLayout *vbox = new QVBoxLayout(tabOSSL);

  oInfo = new QGroupBox(i18n("Path to OpenSSL Shared Libraries"), tabOSSL);
  QVBoxLayout *laygroup1 = new QVBoxLayout;
  oInfo->setLayout(laygroup1);
  vbox->addWidget(oInfo);
  oPath = new KUrlRequester(oInfo);
  laygroup1->addWidget( oPath );
  oPath->setMode(KFile::Directory);
  oTest = new QPushButton(i18n("&Test"));
  laygroup1->addWidget( oTest );
  connect(oTest, SIGNAL(clicked()), SLOT(slotTestOSSL()));

  connect(oPath, SIGNAL(textChanged(const QString&)), SLOT(configChanged()));

  //
  //  Settings for the EGD
  //
  QFrame *eFrame = new QFrame(tabOSSL);
  QVBoxLayout *egrid = new QVBoxLayout(eFrame);
  mUseEGD = new QCheckBox(i18n("Use EGD"), eFrame);
  connect(mUseEGD, SIGNAL(toggled(bool)), SLOT(slotUseEGD()));
  mUseEFile = new QCheckBox(i18n("Use entropy file"), eFrame);
  connect(mUseEFile, SIGNAL(toggled(bool)), SLOT(slotUseEFile()));
  vbox->addWidget(eFrame);
  egrid->addWidget(mUseEGD);
  egrid->addWidget(mUseEFile);
  QFrame *egdframe = new QFrame(tabOSSL);
  QGridLayout *grid2 = new QGridLayout(egdframe);
  mEGDLabel = new QLabel(i18n("Path to EGD:"), egdframe);
  grid2->addWidget(mEGDLabel, 0, 0);
  mEGDPath = new KUrlRequester(egdframe);
  mEGDPath->setMode(KFile::File|KFile::ExistingOnly|KFile::LocalOnly);
  grid2->addWidget(mEGDPath, 0, 1);
  connect(mEGDPath, SIGNAL(textChanged(const QString&)), SLOT(configChanged()));
  vbox->addWidget(egdframe);
  whatstr = i18n("If selected, OpenSSL will be asked to use the entropy gathering"
          " daemon (EGD) for initializing the pseudo-random number generator.");
  mUseEGD->setWhatsThis( whatstr);
  whatstr = i18n("If selected, OpenSSL will be asked to use the given file"
          " as entropy for initializing the pseudo-random number generator.");
  mUseEFile->setWhatsThis( whatstr);
  whatstr = i18n("Enter the path to the socket created by the entropy gathering"
                " daemon (or the entropy file) here.");
  mEGDPath->setWhatsThis( whatstr);
  whatstr = i18n("Click here to browse for the EGD socket file.");
  mEGDPath->setWhatsThis( whatstr);

  vbox->addStretch();
#endif



  ///////////////////////////////////////////////////////////////////////////
  // THIRD TAB
  ///////////////////////////////////////////////////////////////////////////
  tabYourSSLCert = new QFrame(this);

  QVBoxLayout *yourLay= new QVBoxLayout(tabYourSSLCert);

#ifdef KSSL_HAVE_SSL
  QHBoxLayout *treeLay=new QHBoxLayout();
  yourLay->addLayout(treeLay,50);

  yourSSLBox = new QTreeWidget(tabYourSSLCert);
  yourSSLBox->setAllColumnsShowFocus(true);
  whatstr = i18n("This list box shows which certificates of yours KDE"
                " knows about. You can easily manage them from here.");
  yourSSLBox->setWhatsThis( whatstr);
  yourSSLBox->setRootIsDecorated(false);
  treeLay->addWidget(yourSSLBox);
  yourSSLBox->setHeaderLabels(QStringList() << i18n("Common Name") << i18n("Email Address"));
  connect(yourSSLBox, SIGNAL(itemSelectionChanged()), SLOT(slotYourCertSelect()));


  QVBoxLayout *btnsLay=new QVBoxLayout();
  treeLay->addLayout(btnsLay);
  treeLay->setMargin(0);

  yourSSLImport = new QPushButton(i18n("I&mport..."), tabYourSSLCert);
  connect(yourSSLImport, SIGNAL(clicked()), SLOT(slotYourImport()));
  btnsLay->addWidget(yourSSLImport);

  yourSSLExport = new QPushButton(i18n("&Export..."), tabYourSSLCert);
  yourSSLExport->setEnabled(false);
  connect(yourSSLExport, SIGNAL(clicked()), SLOT(slotYourExport()));
  btnsLay->addWidget(yourSSLExport);

  yourSSLRemove = new QPushButton(i18n("Remo&ve"), tabYourSSLCert);
  yourSSLRemove->setEnabled(false);
  connect(yourSSLRemove, SIGNAL(clicked()), SLOT(slotYourRemove()));
  btnsLay->addWidget(yourSSLRemove);

  yourSSLUnlock = new QPushButton(i18n("&Unlock"), tabYourSSLCert);
  yourSSLUnlock->setEnabled(false);
  connect(yourSSLUnlock, SIGNAL(clicked()), SLOT(slotYourUnlock()));
  btnsLay->addWidget(yourSSLUnlock);

  yourSSLVerify = new QPushButton(i18n("Verif&y"), tabYourSSLCert);
  yourSSLVerify->setEnabled(false);
  connect(yourSSLVerify, SIGNAL(clicked()), SLOT(slotYourVerify()));
  btnsLay->addWidget(yourSSLVerify);

  yourSSLPass = new QPushButton(i18n("Chan&ge Password..."), tabYourSSLCert);
  yourSSLPass->setEnabled(false);
  connect(yourSSLPass, SIGNAL(clicked()), SLOT(slotYourPass()));
  btnsLay->addWidget(yourSSLPass);
  
  btnsLay->addStretch(1);

//   yourLay->addWidget(new KSeparator(Qt::Horizontal, tabYourSSLCert));
  
  QHBoxLayout* certLay=new QHBoxLayout();
  yourLay->addLayout(certLay);
  ySubject = new KSslCertificateBox(tabYourSSLCert);
  yIssuer = new KSslCertificateBox(tabYourSSLCert);
  certLay->addWidget(ySubject);
  certLay->addWidget(yIssuer);
  ySubject->setWhatsThis( i18n("This is the information known about the owner of the certificate.") );
  yIssuer->setWhatsThis(  i18n("This is the information known about the issuer of the certificate.") );

  QFormLayout* fl=new QFormLayout();
  yourLay->addLayout(fl);
  yValidFrom = new QLabel(tabYourSSLCert);
  yValidUntil = new QLabel(tabYourSSLCert);
  fl->addRow(i18n("Valid from:"), yValidFrom);
  fl->addRow(i18n("Valid until:"), yValidUntil);
  yValidFrom->setWhatsThis( i18n("The certificate is valid starting at this date.") );
  yValidUntil->setWhatsThis( i18n("The certificate is valid until this date.") );

  yHash = new QLabel(tabYourSSLCert);
  fl->addRow(i18n("MD5 digest:"),yHash);
  yHash->setWhatsThis( i18n("A hash of the certificate used to identify it quickly.") );

#if 0
  Q3HButtonGroup *ocbg = new Q3HButtonGroup(i18n("On SSL Connection..."), tabYourSSLCert);
  yourSSLUseDefault = new QRadioButton(i18n("&Use default certificate"), ocbg);
  yourSSLList = new QRadioButton(i18n("&List upon connection"), ocbg);
  yourSSLDont = new QRadioButton(i18n("&Do not use certificates"), ocbg);
  grid->addWidget(ocbg, 14, 0, 1, 6);
#endif
#else
  QLabel *nossllabel = new QLabel(i18n("SSL certificates cannot be managed"
                               " because this module was not linked"
                               " with OpenSSL."), tabYourSSLCert);
  yourLay->addWidget(nossllabel);
#endif


  ///////////////////////////////////////////////////////////////////////////
  // FOURTH TAB
  ///////////////////////////////////////////////////////////////////////////
  tabAuth = new QFrame(this);

#ifdef KSSL_HAVE_SSL
  grid = new QGridLayout(tabAuth);

//  grid->addWidget(new QLabel(i18n("Default Authentication Certificate"), tabAuth), 0, 0, 0, 3 );
  QGroupBox *defCertGroupBox = new QGroupBox(i18n("Default Action"), tabAuth);
  QButtonGroup *defCertBG = new QButtonGroup(defCertGroupBox);
  QVBoxLayout *defCertBGLayout = new QVBoxLayout(defCertGroupBox);
  defSend = new QRadioButton(i18n("&Send"), defCertGroupBox);
  defCertBG->addButton(defSend, 0);
  defCertBGLayout->addWidget(defSend);
  defPrompt = new QRadioButton(i18n("&Prompt"), defCertGroupBox);
  defCertBG->addButton(defPrompt, 1);
  defCertBGLayout->addWidget(defPrompt);
  defDont = new QRadioButton(i18n("Do &not send"), defCertGroupBox);
  defCertBG->addButton(defDont, 2);
  defCertBGLayout->addWidget(defDont);

  grid->addWidget(defCertGroupBox, 1, 0, 3, 3 );
  grid->addWidget(new QLabel(i18n("Default certificate:"), tabAuth), 1, 3, 1, 3 );
  defCertBox = new KComboBox(false, tabAuth);
  grid->addWidget(defCertBox, 2, 3, 1, 3);

  grid->addWidget(new KSeparator(Qt::Horizontal, tabAuth), 4, 0, 1, 6);

  grid->addWidget(new QLabel(i18n("Host authentication:"), tabAuth), 5, 0, 1, 2 );
  hostAuthList = new QTreeWidget(tabAuth);
  hostAuthList->setRootIsDecorated(false);
  hostAuthList->setAllColumnsShowFocus(true);
  grid->addWidget(hostAuthList, 6, 0, 8, 6);
  hostAuthList->setHeaderLabels(QStringList() << i18n("Host") << i18n("Certificate") << i18n("Policy"));
  grid->addWidget(new QLabel(i18n("Host:"), tabAuth), 14, 0);
  grid->addWidget(new QLabel(i18n("Certificate:"), tabAuth), 15, 0);

  authHost = new KLineEdit(tabAuth);
  grid->addWidget(authHost, 14, 1, 1, 4);
  hostCertBox = new KComboBox(false, tabAuth);
  grid->addWidget(hostCertBox, 15, 1, 1, 4);

  hostCertGroupBox = new QGroupBox(i18n("Action"), tabAuth);
  hostCertBG = new QButtonGroup(hostCertGroupBox);
  QHBoxLayout *hostCertBGLayout = new QHBoxLayout(hostCertGroupBox);
  hostSend = new QRadioButton(i18n("Send"), hostCertGroupBox);
  hostCertBG->addButton(hostSend, 0);
  hostCertBGLayout->addWidget(hostSend);
  hostPrompt = new QRadioButton(i18n("Prompt"), hostCertGroupBox);
  hostCertBG->addButton(hostPrompt, 1);
  hostCertBGLayout->addWidget(hostPrompt);
  hostDont = new QRadioButton(i18n("Do not send"), hostCertGroupBox);
  hostCertBG->addButton(hostDont, 2);
  hostCertBGLayout->addWidget(hostDont);
  grid->addWidget(hostCertGroupBox, 16, 0, 1, 6);

  authAdd = new QPushButton(i18n("Ne&w"), tabAuth);
  authRemove = new QPushButton(i18n("Remo&ve"), tabAuth);
  grid->addWidget(authAdd, 17, 4);
  grid->addWidget(authRemove, 17, 5);

  authHost->setEnabled(false);
  hostCertBox->setEnabled(false);
  hostCertGroupBox->setEnabled(false);
  authRemove->setEnabled(false);

  connect(defCertBox, SIGNAL(activated(int)), this, SLOT(configChanged()));
  connect(defCertBG, SIGNAL(buttonClicked(int)), this, SLOT(configChanged()));
  connect(hostAuthList, SIGNAL(itemSelectionChanged()), this, SLOT(slotAuthItemChanged()));
  connect(authAdd, SIGNAL(clicked()), this, SLOT(slotNewHostAuth()));
  connect(authRemove, SIGNAL(clicked()), this, SLOT(slotRemoveHostAuth()));
  connect(authHost, SIGNAL(textChanged(const QString &)), this, SLOT(slotAuthText(const QString &)));
  connect(hostCertBG, SIGNAL(buttonClicked(int)), this, SLOT(slotAuthButtons()));
  connect(hostCertBox, SIGNAL(activated(int)), this, SLOT(slotAuthCombo()));

#else
  nossllabel = new QLabel(i18n("SSL certificates cannot be managed"
                               " because this module was not linked"
                               " with OpenSSL."), tabAuth);
  grid->addWidget(nossllabel, 3, 0, 1, 6);
#endif



  ///////////////////////////////////////////////////////////////////////////
  // FIFTH TAB
  ///////////////////////////////////////////////////////////////////////////
  tabOtherSSLCert = new QFrame(this);

  QVBoxLayout* mainOther=new QVBoxLayout(tabOtherSSLCert);
#ifdef KSSL_HAVE_SSL

  QHBoxLayout* hb=new QHBoxLayout();
  mainOther->addLayout(hb);
  otherSSLBox = new QTreeWidget(tabOtherSSLCert);
  hb->addWidget(otherSSLBox);
  otherSSLBox->setAllColumnsShowFocus(true);
  otherSSLBox->setRootIsDecorated(false);
  connect(otherSSLBox, SIGNAL(itemSelectionChanged()), SLOT(slotOtherCertSelect()));
  whatstr = i18n("This list box shows which site and person certificates KDE"
                " knows about. You can easily manage them from here.");
  otherSSLBox->setWhatsThis( whatstr);
  otherSSLBox->setHeaderLabels(QStringList() << i18n("Organization") << i18n("Common Name"));

  QVBoxLayout* vb=new QVBoxLayout;
  hb->addLayout(vb);

  otherSSLExport = new QPushButton(i18n("&Export..."), tabOtherSSLCert);
  connect(otherSSLExport, SIGNAL(clicked()), SLOT(slotExportCert()));
  vb->addWidget(otherSSLExport);
  whatstr = i18n("This button allows you to export the selected certificate"
                " to a file of various formats.");
  otherSSLExport->setWhatsThis( whatstr);

  otherSSLRemove = new QPushButton(i18n("&Remove"), tabOtherSSLCert);
  connect(otherSSLRemove, SIGNAL(clicked()), SLOT(slotRemoveCert()));
  vb->addWidget(otherSSLRemove);
  whatstr = i18n("This button removes the selected certificate"
                " from the certificate cache.");
  otherSSLRemove->setWhatsThis( whatstr);

  otherSSLVerify = new QPushButton(i18n("&Verify"), tabOtherSSLCert);
  connect(otherSSLVerify, SIGNAL(clicked()), SLOT(slotVerifyCert()));
  vb->addWidget(otherSSLVerify);
  whatstr = i18n("This button tests the selected certificate"
                " for validity.");
  otherSSLVerify->setWhatsThis( whatstr);

  vb->addStretch(1);

      otherSSLExport->setEnabled(false);
      otherSSLVerify->setEnabled(false);
      otherSSLRemove->setEnabled(false);

//   mainOther->addWidget(new KSeparator(Qt::Horizontal, tabOtherSSLCert));
  
  grid=new QGridLayout();
  mainOther->addLayout(grid);
  oSubject = new KSslCertificateBox(tabOtherSSLCert);
  oIssuer = new KSslCertificateBox(tabOtherSSLCert);
  grid->addWidget(oSubject, 0, 0);
  grid->addWidget(oIssuer, 0, 1);
  whatstr = i18n("This is the information known about the owner of the certificate.");
  oSubject->setWhatsThis( whatstr);
  whatstr = i18n("This is the information known about the issuer of the certificate.");
  oIssuer->setWhatsThis( whatstr);

  fromLabel = new QLabel(i18n("Valid from:"), tabOtherSSLCert);
  untilLabel = new QLabel(i18n("Valid until:"), tabOtherSSLCert);
  grid->addWidget(fromLabel, 1, 0);
  grid->addWidget(untilLabel, 2, 0);
  fromLabel->setEnabled(false);
  untilLabel->setEnabled(false);
  validFrom = new QLabel(tabOtherSSLCert);
  grid->addWidget(validFrom, 1, 1);
  validUntil = new QLabel(tabOtherSSLCert);
  grid->addWidget(validUntil, 2, 1);
  whatstr = i18n("The certificate is valid starting at this date.");
  validFrom->setWhatsThis( whatstr);
  whatstr = i18n("The certificate is valid until this date.");
  validUntil->setWhatsThis( whatstr);

  cacheGroupBox = new QGroupBox(i18n("Cache"), tabOtherSSLCert);
  cacheGroup = new QButtonGroup(cacheGroupBox);
  QHBoxLayout *cacheGroupLayout = new QHBoxLayout(cacheGroupBox);
  cachePerm = new QRadioButton(i18n("Permanentl&y"), cacheGroupBox);
  cacheGroup->addButton(cachePerm, 0);
  cacheGroupLayout->addWidget(cachePerm);
  cacheUntil = new QRadioButton(i18n("&Until"), cacheGroupBox);
  cacheGroup->addButton(cacheUntil, 1);
  cacheGroupLayout->addWidget(cacheUntil);
  untilDate = new KUrlLabel(QString(), QString(), cacheGroupBox);
  cacheGroupLayout->addWidget(untilDate);
  cacheGroupBox->setEnabled(false);
  grid->addWidget(cacheGroupBox, 3, 0);

  cachePerm->setEnabled(false);
  cacheUntil->setEnabled(false);
  untilDate->setEnabled(false);
  connect(cachePerm, SIGNAL(clicked()), SLOT(slotPermanent()));
  connect(cacheUntil, SIGNAL(clicked()), SLOT(slotUntil()));
  connect(untilDate, SIGNAL(leftClickedUrl()), SLOT(slotDatePick()));
  whatstr = i18n("Select here to make the cache entry permanent.");
  cachePerm->setWhatsThis( whatstr);
  whatstr = i18n("Select here to make the cache entry temporary.");
  cacheUntil->setWhatsThis( whatstr);
  whatstr = i18n("The date and time until the certificate cache entry should expire.");
  untilDate->setWhatsThis( whatstr);

  policyGroupBox = new QGroupBox(i18n("Policy"), tabOtherSSLCert);
  policyGroup = new QButtonGroup(policyGroupBox);
  QHBoxLayout *policyGroupLayout = new QHBoxLayout(policyGroupBox);
  policyAccept = new QRadioButton(i18n("Accep&t"), policyGroupBox);
  policyGroup->addButton(policyAccept, 0);
  policyGroupLayout->addWidget(policyAccept);
  policyReject = new QRadioButton(i18n("Re&ject"), policyGroupBox);
  policyGroup->addButton(policyReject, 1);
  policyGroupLayout->addWidget(policyReject);
  policyPrompt = new QRadioButton(i18n("&Prompt"), policyGroupBox);
  policyGroup->addButton(policyPrompt, 2);
  policyGroupLayout->addWidget(policyPrompt);
  policyGroupBox->setEnabled(false);
  grid->addWidget(policyGroupBox, 3, 1);
  connect(policyGroup, SIGNAL(buttonClicked(int)), SLOT(slotPolicyChanged(int)));
  policyAccept->setWhatsThis( i18n("Select this to always accept this certificate."));
  policyReject->setWhatsThis( i18n("Select this to always reject this certificate.") );
  policyPrompt->setWhatsThis( i18n("Select this if you wish to be prompted for action when receiving this certificate.") );
  grid->addWidget(new QLabel(i18n("MD5 digest:"), tabOtherSSLCert), 4, 0);
  pHash = new QLabel(tabOtherSSLCert);
  grid->addWidget(pHash, 4,1);
  whatstr = i18n("A hash of the certificate used to identify it quickly.");
  pHash->setWhatsThis( whatstr);

#else
  nossllabel = new QLabel(i18n("SSL certificates cannot be managed"
                               " because this module was not linked"
                               " with OpenSSL."), tabOtherSSLCert);
  mainOther->addWidget(nossllabel);
#endif


  ///////////////////////////////////////////////////////////////////////////
  // SIXTH TAB
  ///////////////////////////////////////////////////////////////////////////
  tabSSLCA = new QFrame(this);

#ifdef KSSL_HAVE_SSL
  grid = new QGridLayout(tabSSLCA);

  caList = new QTreeWidget(tabSSLCA);
  caList->setAllColumnsShowFocus(true);
  whatstr = i18n("This list box shows which certificate authorities KDE"
                 " knows about. You can easily manage them from here.");
  caList->setWhatsThis( whatstr);
  caList->setRootIsDecorated(false);
  grid->addWidget(caList, 0, 0, 4, 7);
  caList->setHeaderLabels(QStringList() << i18n("Organization")
                                        << i18n("Organizational Unit")
                                        << i18n("Common Name"));
  connect(caList, SIGNAL(itemSelectionChanged()), SLOT(slotCAItemChanged()));

  caSSLImport = new QPushButton(i18n("I&mport..."), tabSSLCA);
  connect(caSSLImport, SIGNAL(clicked()), SLOT(slotCAImport()));
  grid->addWidget(caSSLImport, 0, 7);

  caSSLRemove = new QPushButton(i18n("&Remove"), tabSSLCA);
  connect(caSSLRemove, SIGNAL(clicked()), SLOT(slotCARemove()));
  grid->addWidget(caSSLRemove, 1, 7);
  caSSLRemove->setEnabled(false);

  caSSLRestore = new QPushButton(i18n("Res&tore"), tabSSLCA);
  connect(caSSLRestore, SIGNAL(clicked()), SLOT(slotCARestore()));
  grid->addWidget(caSSLRestore, 2, 7);

  caSubject = new KSslCertificateBox(tabSSLCA);
  caIssuer = new KSslCertificateBox(tabSSLCA);
  grid->addWidget(caSubject, 4, 0, 3, 4 );
  grid->addWidget(caIssuer, 4, 4, 3, 4);

  // Accept for Web Site Signing, Email Signing, Code Signing
  caSite = new QCheckBox(i18n("Accept for site signing"), tabSSLCA);
  caEmail = new QCheckBox(i18n("Accept for email signing"), tabSSLCA);
  caCode = new QCheckBox(i18n("Accept for code signing"), tabSSLCA);
  grid->addWidget(caSite, 7, 0, 1, 4 );
  connect(caSite, SIGNAL(clicked()), SLOT(slotCAChecked()));
  grid->addWidget(caEmail, 8, 0, 1, 4 );
  connect(caEmail, SIGNAL(clicked()), SLOT(slotCAChecked()));
  grid->addWidget(caCode, 9, 0, 1, 4 );
  connect(caCode, SIGNAL(clicked()), SLOT(slotCAChecked()));
  caSite->setEnabled(false);
  caEmail->setEnabled(false);
  caCode->setEnabled(false);
  grid->addWidget(new QLabel(i18n("MD5 digest:"), tabSSLCA), 10, 0);
  cHash = new QLabel(tabSSLCA);
  grid->addWidget(cHash, 10, 1);
  whatstr = i18n("A hash of the certificate used to identify it quickly.");
  cHash->setWhatsThis( whatstr);

#else
  nossllabel = new QLabel(i18n("SSL certificates cannot be managed"
                               " because this module was not linked"
                               " with OpenSSL."), tabSSLCA);
  grid->addWidget(nossllabel, 1, 0, 1, 2 );
#endif


#if 0
  ///////////////////////////////////////////////////////////////////////////
  // SEVENTH TAB
  ///////////////////////////////////////////////////////////////////////////
  tabSSLCOpts = new QFrame(this);

#ifdef KSSL_HAVE_SSL
  grid = new QGridLayout(tabSSLCOpts);
  mWarnSelfSigned = new QCheckBox(i18n("Warn on &self-signed certificates or unknown CA's"), tabSSLCOpts);
  connect(mWarnSelfSigned, SIGNAL(clicked()), SLOT(configChanged()));
  mWarnExpired = new QCheckBox(i18n("Warn on &expired certificates"), tabSSLCOpts);
  connect(mWarnExpired, SIGNAL(clicked()), SLOT(configChanged()));
  mWarnRevoked = new QCheckBox(i18n("Warn on re&voked certificates"), tabSSLCOpts);
  connect(mWarnRevoked, SIGNAL(clicked()), SLOT(configChanged()));
  grid->addWidget(mWarnSelfSigned, 0, 0, 1, 4 );
  grid->addWidget(mWarnExpired, 1, 0, 1, 4 );
  grid->addWidget(mWarnRevoked, 2, 0, 1, 4 );

  macCert = new KLineEdit(tabSSLCOpts);
  grid->addWidget(macCert, 4, 0, 1, 3 );

  macBox = new QTreeWidget(tabSSLCOpts);
  whatstr = i18n("This list box shows which sites you have decided to accept"
                " a certificate from even though the certificate might fail"
                " the validation procedure.");
  macBox->setWhatsThis( whatstr);
  caSSLBox->setSelectionMode(QTreeWidget::Single);
  caSSLBox->setColumnMode(QTreeWidget::FixedNumber);
  grid->addWidget(macBox, 5, 0, 4, 3 );

  macAdd = new QPushButton(i18n("&Add"), tabSSLCOpts);
  //connect(macAdd, SIGNAL(), SLOT());
  grid->addWidget(macAdd, 4, 3);

  macRemove = new QPushButton(i18n("&Remove"), tabSSLCOpts);
  //connect(macRemove, SIGNAL(), SLOT());
  grid->addWidget(macRemove, 5, 3);

  macClear = new KPushButton(KGuiItem::clear(), tabSSLCOpts);
  //connect(macAdd, SIGNAL(), SLOT());
  grid->addWidget(macClear, 6, 3);

#else
  nossllabel = new QLabel(i18n("These options are not configurable"
                               " because this module was not linked"
                               " with OpenSSL."), tabSSLCOpts);
  grid->addWidget(nossllabel, 1, 0, 1, 2 );
#endif
#endif

  ///////////////////////////////////////////////////////////////////////////
  // Add the tabs and startup
  ///////////////////////////////////////////////////////////////////////////
  tabs->addTab(tabSSL, i18n("SSL"));
#ifdef KSSL_HAVE_SSL
  tabs->addTab(tabOSSL, i18n("OpenSSL"));
#endif
  tabs->addTab(tabYourSSLCert, i18n("Your Certificates"));
  tabs->addTab(tabAuth, i18n("Authentication"));
  tabs->addTab(tabOtherSSLCert, i18n("Peer SSL Certificates"));
  tabs->addTab(tabSSLCA, i18n("SSL Signers"));

#if 0
  tabs->addTab(tabSSLCOpts, i18n("Validation Options"));
#endif

  tabs->resize(tabs->sizeHint());
  load();
}

KCryptoConfig::~KCryptoConfig()
{
    qDeleteAll(otherCertDelList);
    qDeleteAll(yourCertDelList);
    qDeleteAll(authDelList);
    qDeleteAll(caDelList);
    delete config;
    delete policies;
    delete pcerts;
    delete authcfg;
    delete _signers;
}

void KCryptoConfig::configChanged()
{
    emit changed(true);
}


void KCryptoConfig::load()
{
  KConfigGroup cg(config, "");
#ifdef KSSL_HAVE_SSL
  otherCertDelList.clear();
  yourCertDelList.clear();
  authDelList.clear();
  caDelList.clear();
  KConfigGroup cgWarning(config, "Warnings");
  mWarnOnEnter->setChecked(cgWarning.readEntry("OnEnter", false));
  mWarnOnLeave->setChecked(cgWarning.readEntry("OnLeave", true));
  mWarnOnUnencrypted->setChecked(cgWarning.readEntry("OnUnencrypted", false));

#if 0 // NOT IMPLEMENTED IN KDE 2.0
  mWarnOnMixed->setChecked(cg.readEntry("OnMixed", true));

  cg.changeGroup("Validation");
  mWarnSelfSigned->setChecked(cg.readEntry("WarnSelfSigned", true));
  mWarnExpired->setChecked(cg.readEntry("WarnExpired", true));
  mWarnRevoked->setChecked(cg.readEntry("WarnRevoked", true));
#endif

  KConfigGroup cgEGD(config,"EGD");
  slotUseEGD();  // set the defaults
  if (cgEGD.readEntry("UseEGD", false)) {
    mUseEGD->setChecked(true);
    slotUseEGD();
  } else if (cgEGD.readEntry("UseEFile", false)) {
    mUseEFile->setChecked(true);
    slotUseEFile();
  }
  mEGDPath->setUrl(KUrl(cgEGD.readPathEntry("EGDPath", QString())));


#ifdef KSSL_HAVE_SSL
  KConfigGroup cgSSL( config, "OpenSSL");
  oPath->setUrl(cgSSL.readPathEntry("Path", QString()));
#endif

  KConfigGroup sslV3(config, "SSLv3");
  QTreeWidgetItemIterator it(SSLv3Box);
  while (*it) {
      CipherItem *item = static_cast<CipherItem *>(*it);
      item->setCheckState( 0, sslV3.readEntry(item->configName(),
					item->bits() >= 56) ? Qt::Checked : Qt::Unchecked );
      it++;
  }

  QStringList groups = policies->groupList();

  otherSSLBox->clear();
  for (QStringList::const_iterator i = groups.constBegin(); i != groups.constEnd(); ++i) {
    if (*i == "General") continue;
    KConfigGroup _cg(policies, *i);
    KSSLCertificate *cert = KSSLCertificate::fromString(_cg.readEntry("Certificate", QString()).toLocal8Bit());
    if (cert) {
      new OtherCertItem(otherSSLBox, cert->getSubject(), *i,
                        _cg.readEntry("Permanent", true),
                        _cg.readEntry("Policy", 3),
                        _cg.readEntry("Expires", QDateTime()), this );
      delete cert;
    }
  }

  groups = pcerts->groupList();

  yourSSLBox->clear();
  for (QStringList::const_iterator i = groups.constBegin(); i != groups.constEnd(); ++i) {
    KConfigGroup _cg(pcerts, *i);
    YourCertItem *j = new YourCertItem(yourSSLBox,
                     _cg.readEntry("PKCS12Base64"),
                     _cg.readEntry("Password"),
                     *i, this );
    j->setPassCache(QString());
  }

  setAuthCertLists();

  KConfigGroup auth(config, "Auth");
  QString whichAuth = auth.readEntry("AuthMethod", "none");
  if (whichAuth == "send")
    defSend->setChecked(true);
  else if (whichAuth == "prompt")
    defPrompt->setChecked(true);
  else
    defDont->setChecked(true);

  QString whichCert = auth.readEntry("DefaultCert");
  defCertBox->setCurrentIndex(0);
  for (int i = 0; i < defCertBox->count(); i++) {
     if (defCertBox->itemText(i) == whichCert) {
       defCertBox->setCurrentIndex(i);
       break;
     }
  }
  hostAuthList->clear();
  groups = authcfg->groupList();
  for (QStringList::const_iterator i = groups.constBegin();
                             i != groups.constEnd();
                             ++i) {
    KConfigGroup _cg(authcfg, *i);
    KSSLCertificateHome::KSSLAuthAction aa = KSSLCertificateHome::AuthDont;
    if (_cg.readEntry("send", false) == true)
       aa = KSSLCertificateHome::AuthSend;
    else if (_cg.readEntry("prompt", false) == true)
       aa = KSSLCertificateHome::AuthPrompt;
    HostAuthItem *j = new HostAuthItem(hostAuthList,
                                       KResolver::domainToAscii(*i),
                                       _cg.readEntry("certificate"),
                                       this );
    j->setAction(aa);
    j->setOriginalName(*i);
  }

  groups = _signers->list();
  KConfig sigcfg( "ksslcalist", KConfig::NoGlobals );
  caList->clear();
  for (QStringList::const_iterator i = groups.constBegin();
                             i != groups.constEnd();
                             ++i) {
    if (!sigcfg.hasGroup(*i)) continue;
    KConfigGroup _cg(&sigcfg, *i);
    if (!_cg.hasKey("x509")) continue;
                new CAItem(caList,
                     (*i),
                     _cg.readEntry("x509"),
                     _cg.readEntry("site", false),
                     _cg.readEntry("email", false),
                     _cg.readEntry("code", false),
                     this );
  }

  slotCAItemChanged();
  slotOtherCertSelect();
  slotYourCertSelect();
#endif

  emit changed(false);
}

void KCryptoConfig::save()
{
  KConfigGroup cg(config, "");
#ifdef KSSL_HAVE_SSL
  KConfigGroup cgWarning(config, "Warnings");
  cgWarning.writeEntry("OnEnter", mWarnOnEnter->isChecked());
  cgWarning.writeEntry("OnLeave", mWarnOnLeave->isChecked());
  cgWarning.writeEntry("OnUnencrypted", mWarnOnUnencrypted->isChecked());

  KConfigGroup cgEGD(config,"EGD");
  cgEGD.writeEntry("UseEGD", mUseEGD->isChecked());
  cgEGD.writeEntry("UseEFile", mUseEFile->isChecked());
  cgEGD.writePathEntry("EGDPath", mEGDPath->url().path());

#if 0  // NOT IMPLEMENTED IN KDE 2.0
  cg.writeEntry("OnMixed", mWarnOnMixed->isChecked());

  cg.setGroup("Validation");
  cg.writeEntry("WarnSelfSigned", mWarnSelfSigned->isChecked());
  cg.writeEntry("WarnExpired", mWarnExpired->isChecked());
  cg.writeEntry("WarnRevoked", mWarnRevoked->isChecked());
#endif

#ifdef KSSL_HAVE_SSL
  KConfigGroup cgSSL(config,"OpenSSL");
  cgSSL.writePathEntry("Path", oPath->url().path());
#endif

  int ciphercount = 0;
  KConfigGroup cgSSLV3(config,"SSLv3");
  QTreeWidgetItemIterator it(SSLv3Box);
  while (*it) {
    CipherItem *item = static_cast<CipherItem *>(*it);
    if (item->checkState(0) == Qt::Checked) {
      cgSSLV3.writeEntry(item->configName(), true);
      ciphercount++;
    } else cgSSLV3.writeEntry(item->configName(), false);

    it++;
  }

//###   KSSLCertificateCache _cc;

  if (ciphercount == 0)
    KMessageBox::information(this, i18n("If you do not select at least one"
                                       " cipher, SSL will not work."),
                                   i18n("SSL Ciphers"));
  // SSL Policies code
  while (!otherCertDelList.isEmpty()) {
     OtherCertItem *x = otherCertDelList.takeFirst();
     KSSLX509Map cert(x->configName());
     QString thisCN = cert.getValue("CN");
//###      _cc.removeByCN(thisCN);
     delete x;
  }
  // Go through the non-deleted ones and save them
  it = QTreeWidgetItemIterator(otherSSLBox);
  while (*it) {
     OtherCertItem *x = static_cast<OtherCertItem *>(*it);
     KSSLX509Map cert(x->configName());
     QString thisCN = cert.getValue("CN");
     QDateTime expires = x->getExpires();
//###      _cc.modifyByCN(thisCN, (KSSLCertificateCache::KSSLCertificatePolicy)x->getPolicy(), x->isPermanent(), expires);
     it++;
  }

  // SSL Personal certificates code
  while (!yourCertDelList.isEmpty()) {
     YourCertItem *x = yourCertDelList.takeFirst();
     pcerts->deleteGroup(x->configName());
     delete x;
  }
  // Go through the non-deleted ones and save them
  it = QTreeWidgetItemIterator(yourSSLBox);
  while (*it) {
     YourCertItem *x = static_cast<YourCertItem *>(*it);
     KConfigGroup _cg(pcerts, x->configName());
     _cg.writeEntry("PKCS12Base64", x->getPKCS());
     _cg.writeEntry("Password", x->getPass());
     it++;
  }

  bool doGen = false;

  // CA certificates code
  while (!caDelList.isEmpty()) {
     CAItem *x = caDelList.takeFirst();
     _signers->remove(x->configName());
     delete x;
     doGen = true;
  }

  // Go through the non-deleted ones and save them
  it = QTreeWidgetItemIterator(caList);
  while (*it) {
     CAItem *x = static_cast<CAItem *>(*it);
     if (!x->modified && !x->isNew) continue;
     if (x->isNew) {
        x->isNew = false;
        _signers->addCA(x->getCert(),
                        x->getSite(),
                        x->getEmail(),
                        x->getCode());
     } else {
        _signers->setUse(x->configName(),
                         x->getSite(),
                         x->getEmail(),
                         x->getCode());
     }
     x->modified = false;
     doGen = true;

     it++;
  }

  if (doGen) genCAList();


  KConfigGroup cgAuth(config,"Auth");
  QString whichAuth = cgAuth.readEntry("AuthMethod", "none");
  if (defSend->isChecked())
    cgAuth.writeEntry("AuthMethod", "send");
  else if (defPrompt->isChecked())
    cgAuth.writeEntry("AuthMethod", "prompt");
  else
    cgAuth.writeEntry("AuthMethod", "none");

  if (defCertBox->currentIndex() == 0)
     cgAuth.writeEntry("DefaultCert", QString());
  else cgAuth.writeEntry("DefaultCert", defCertBox->currentText());

  while (!authDelList.isEmpty()) {
     HostAuthItem *x = authDelList.takeFirst();
     authcfg->deleteGroup(x->configName());
     delete x;
  }

  it = QTreeWidgetItemIterator(hostAuthList);
  while (*it) {
     HostAuthItem *x = static_cast<HostAuthItem *>(*it);
     if (!x->originalName().isNull())
        authcfg->deleteGroup(x->originalName());

     it++;
  }

  it = QTreeWidgetItemIterator(hostAuthList);
  while (*it) {
     HostAuthItem *x = static_cast<HostAuthItem *>(*it);
     KConfigGroup _cg(authcfg, KResolver::domainToAscii(x->configName()).constData());
     _cg.writeEntry("certificate", x->getCertName());
     _cg.writeEntry("prompt", (x->getAction() == KSSLCertificateHome::AuthPrompt));
     _cg.writeEntry("send", (x->getAction() == KSSLCertificateHome::AuthSend));

     it++;
  }

#endif

  cg.sync();
  policies->sync();
  pcerts->sync();
  authcfg->sync();

  // insure proper permissions -- contains sensitive data
  QString cfgName(KGlobal::dirs()->findResource("config", "cryptodefaults"));
  if (!cfgName.isEmpty())
    ::chmod(QFile::encodeName(cfgName), 0600);

  emit changed(false);
}

void KCryptoConfig::defaults()
{
  mWarnOnEnter->setChecked(false);
  mWarnOnLeave->setChecked(true);
  mWarnOnUnencrypted->setChecked(false);
#if 0  // NOT IMPLEMENTED IN KDE 2.0
  mWarnOnMixed->setChecked(true);
  mWarnSelfSigned->setChecked(true);
  mWarnExpired->setChecked(true);
  mWarnRevoked->setChecked(true);
#endif

#ifdef KSSL_HAVE_SSL
    // We don't want to make
    // ciphers < 56 bit a default selection.  This is very unsafe and
    // I have already witnessed OpenSSL negotiate a 0 bit connection
    // on me after tracing the https ioslave on a suspicion.

  QTreeWidgetItemIterator it(SSLv3Box);
  while (*it) {
      CipherItem *item = static_cast<CipherItem *>(*it);
      item->setCheckState( 0, item->bits() >= 56 ? Qt::Checked : Qt::Unchecked );
      it++;
  }
  mUseEGD->setChecked(false);
  mUseEFile->setChecked(false);
  mEGDLabel->setEnabled(false);
  mEGDPath->setEnabled(false);
  mEGDPath->clear();
  oPath->clear();

  defDont->setChecked(true);
#endif

  emit changed(true);
}

void KCryptoConfig::genCAList() {
   _signers->regenerate();

}

void KCryptoConfig::slotSelectCipher(int id) {
    switch(id) {
	    case 1:
		    cwUS();
		    break;
	    case 2:
		    cwExp();
		    break;
	    case 3:
		    cwAll();
    }
}

void KCryptoConfig::cwUS() {
  #ifdef KSSL_HAVE_SSL
  QTreeWidgetItemIterator it( SSLv3Box );
  while (*it) {
      CipherItem *item = static_cast<CipherItem *>(*it);
      item->setCheckState( 0, item->bits() >= 128 ? Qt::Checked : Qt::Unchecked );
      it++;
  }
  configChanged();
  #endif
}


void KCryptoConfig::cwExp() {
  #ifdef KSSL_HAVE_SSL
  QTreeWidgetItemIterator it( SSLv3Box );
  while (*it) {
      CipherItem *item = static_cast<CipherItem *>(*it);
      item->setCheckState( 0, item->bits() <= 56 && item->bits() > 0 ? Qt::Checked : Qt::Unchecked );
      it++;
  }
  configChanged();
  #endif
}


void KCryptoConfig::cwAll() {
  #ifdef KSSL_HAVE_SSL
  QTreeWidgetItemIterator it( SSLv3Box );
  while (*it) {
      (*it)->setCheckState( 0, Qt::Checked );
      it++;
  }
  configChanged();
  #endif
}



void KCryptoConfig::slotExportCert() {
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->currentItem());
   if (x) {
     KConfigGroup cg(policies, x->getMD5());
     KSSLCertificate *cert = KSSLCertificate::fromString(cg.readEntry("Certificate", QString()).toLocal8Bit());
     if (cert) {
        KCertExport kce;
        kce.setCertificate(cert);
        kce.exec();
        delete cert;
     } else KMessageBox::sorry(this, i18n("Could not open the certificate."),
                                     i18n("SSL"));
   }
}



void KCryptoConfig::slotRemoveCert() {
QTreeWidgetItem *act = otherSSLBox->currentItem();
OtherCertItem *x = static_cast<OtherCertItem *>(act);
   if (x) {
      QTreeWidgetItem *next = otherSSLBox->itemBelow(act);
      if (!next) next = otherSSLBox->itemAbove(act);
      otherSSLBox->takeTopLevelItem(otherSSLBox->indexOfTopLevelItem(x));
      otherCertDelList.append(x);
      configChanged();
      if (next) otherSSLBox->setCurrentItem(next);
   }
}



void KCryptoConfig::slotVerifyCert() {
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->currentItem());
  if (!x) return;

  KConfigGroup cg(policies, x->getMD5());
  KSSLCertificate *cert = KSSLCertificate::fromString(cg.readEntry("Certificate", QString()).toLocal8Bit());

  if (!cert) {
    KMessageBox::error(this, i18n("Error obtaining the certificate."), i18n("SSL"));
    return;
  }

  cert->chain().setCertChain(cg.readEntry("Chain", QStringList()));

  KSSLCertificate::KSSLValidation v = cert->revalidate(KSSLCertificate::SSLServer);

  /*
   *  Don't enable this until we keep this info in the cache
  if (v != KSSLCerticiate::Ok)
	  v = cert->revalidate(KSSLCertificate::SMIMESign);
  if (v != KSSLCerticiate::Ok)
	  v = cert->revalidate(KSSLCertificate::SMIMEEncrypt);
  */

  if (v == KSSLCertificate::Ok) {
     KMessageBox::information(this, i18n("This certificate passed the verification tests successfully."), i18n("SSL"));
  } else {
     KMessageBox::detailedError(this, i18n("This certificate has failed the tests and should be considered invalid."), KSSLCertificate::verifyText(v), i18n("SSL"));
  }

  delete cert;
}



void KCryptoConfig::slotUntil() {
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->currentItem());

   cachePerm->setChecked(false);
   untilDate->setEnabled(true);

   if (!x) return;
   x->setPermanent(false);

   configChanged();
}



void KCryptoConfig::slotPermanent() {
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->currentItem());

   cacheUntil->setChecked(false);
   untilDate->setEnabled(false);

   if (!x) return;
   x->setPermanent(true);

   configChanged();
}



void KCryptoConfig::slotPolicyChanged(int id) {
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->currentItem());
if (!x) return;

//###    if (id == policyGroup->id(policyAccept)) {
//###      x->setPolicy(KSSLCertificateCache::Accept);
//###    } else if (id == policyGroup->id(policyReject)) {
//###      x->setPolicy(KSSLCertificateCache::Reject);
//###    } else if (id == policyGroup->id(policyPrompt)) {
//###      x->setPolicy(KSSLCertificateCache::Prompt);
//###    }

   configChanged();
}



void KCryptoConfig::slotDatePick() {
KDateTimeDlg kdtd;
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->currentItem());

   if (!x || !untilDate->isEnabled()) return;

QDateTime qdt = x->getExpires();

   kdtd.setDateTime(qdt);
   int rc = kdtd.exec();
   if (rc == KDialog::Accepted) {
      x->setExpires(kdtd.getDateTime());
      untilDate->setText(KGlobal::locale()->formatDateTime(x->getExpires()));
      configChanged();
   }
}



void KCryptoConfig::slotOtherCertSelect() {
   OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->currentItem());
   QString iss;
   if (x) {
      otherSSLExport->setEnabled(true);
      otherSSLVerify->setEnabled(true);
      otherSSLRemove->setEnabled(true);

      fromLabel->setEnabled(true);
      untilLabel->setEnabled(true);
      policyGroupBox->setEnabled(true);
      cacheGroupBox->setEnabled(true);
      cachePerm->setEnabled(true);
      cacheUntil->setEnabled(true);
      KConfigGroup cg(policies, x->getMD5());

      KSSLCertificate *cert = KSSLCertificate::fromString(cg.readEntry("Certificate", QString()).toLocal8Bit());

      if (cert) {
         QPalette cspl;
         iss = cert->getIssuer();
         cspl = validFrom->palette();
         if (QDateTime::currentDateTime() < cert->getQDTNotBefore()) {
            cspl.setColor(QPalette::Foreground, QColor(196,33,21));
         } else {
            cspl.setColor(QPalette::Foreground, QColor(42,153,59));
         }
         validFrom->setPalette(cspl);

         cspl = validUntil->palette();
         if (QDateTime::currentDateTime() > cert->getQDTNotAfter()) {
            cspl.setColor(QPalette::Foreground, QColor(196,33,21));
         } else {
            cspl.setColor(QPalette::Foreground, QColor(42,153,59));
         }
         validUntil->setPalette(cspl);

         validFrom->setText(cert->getNotBefore());
         validUntil->setText(cert->getNotAfter());
         untilDate->setText(x ? KGlobal::locale()->formatDateTime(x->getExpires())
                              : KGlobal::locale()->formatDateTime(QDateTime::currentDateTime()));
         untilDate->setEnabled(x && !x->isPermanent());
         pHash->setText(cert->getMD5DigestText());
         oSubject->setCertificate(QSslCertificate(cert->toPem()), KSslCertificateBox::Subject);
         oIssuer->setCertificate(QSslCertificate(cert->toPem()), KSslCertificateBox::Issuer);
         delete cert;
      } else {
         validFrom->setText(QString());
         validUntil->setText(QString());
         pHash->clear();
         oSubject->clear();
         oIssuer->clear();
      }

//###       switch(x->getPolicy()) {
//###       case KSSLCertificateCache::Accept:
//###         policyGroup->setButton(policyGroup->id(policyAccept));
//###       break;
//###       case KSSLCertificateCache::Reject:
//###         policyGroup->setButton(policyGroup->id(policyReject));
//###       break;
//###       case KSSLCertificateCache::Prompt:
//###         policyGroup->setButton(policyGroup->id(policyPrompt));
//###       break;
//###       }

      cachePerm->setChecked(x->isPermanent());
      cacheUntil->setChecked(!x->isPermanent());

   } else {
      otherSSLExport->setEnabled(false);
      otherSSLVerify->setEnabled(false);
      otherSSLRemove->setEnabled(false);
      fromLabel->setEnabled(false);
      untilLabel->setEnabled(false);
      policyGroupBox->setEnabled(false);
      cacheGroupBox->setEnabled(false);
      cachePerm->setChecked(false);
      cacheUntil->setChecked(false);
      policyAccept->setChecked(false);
      policyReject->setChecked(false);
      policyPrompt->setChecked(false);
      cachePerm->setEnabled(false);
      cacheUntil->setEnabled(false);
      validFrom->setText(QString());
      validUntil->setText(QString());
      untilDate->setText(QString());
      untilDate->setEnabled(false);
      pHash->clear();
      oSubject->clear();
      oIssuer->clear();
   }


   //### oSubject->setValues(x ? x->getSub() : QString());
   //### oIssuer->setValues(iss);

}


void KCryptoConfig::slotYourImport() {

   QString certFile = KFileDialog::getOpenFileName(QString(), "application/x-pkcs12", this);
   if (certFile.isEmpty())
      return;

#ifdef KSSL_HAVE_SSL
KSSLPKCS12 *cert = NULL;

TryImportPassAgain:
   KPasswordDialog dlg(this);
   dlg.setPrompt(i18n("Certificate Password"));
   dlg.setCaption(i18n("Certificate password"));
   if(!dlg.exec())
	return;

   cert = KSSLPKCS12::loadCertFile(certFile, dlg.password());

   if (!cert) {
      int rc = KMessageBox::warningYesNo(this, i18n("The certificate file could not be loaded. Try a different password?"), i18n("SSL"),KGuiItem(i18n("Try")),KGuiItem(i18n("Do Not Try")));
      if (rc == KMessageBox::Yes) goto TryImportPassAgain;
      return;
   }

   // At this point, we know that we can read the certificate in.
   // The procedure will be to convert it to Base64 in it's raw form
   // and add it to the ListView - eventually going into the SimpleConfig.

   // FIXME: prompt if the user wants the password stored along with the
   //        certificate

   QString name = cert->getCertificate()->getSubject();
   QTreeWidgetItemIterator it(yourSSLBox);
   while (*it) {
      YourCertItem *i = static_cast<YourCertItem *>(*it);
      if (i->configName() == name) {
         int rc = KMessageBox::warningContinueCancel(this, i18n("A certificate with that name already exists. Are you sure that you wish to replace it?"), i18n("SSL"), KGuiItem(i18n("Replace")));
         if (rc == KMessageBox::Cancel) {
            delete cert;
            return;
         }
         yourSSLBox->takeTopLevelItem(yourSSLBox->indexOfTopLevelItem(i));
         yourCertDelList.append(i);
      }
      it++;
   }

   new YourCertItem(yourSSLBox,
                    cert->toString(),
                    QString(), // the password - don't store it yet!
                    name,
                    this );

   setAuthCertLists();
   configChanged();
   delete cert;
#endif

   offerImportToKMail( certFile );
}


void KCryptoConfig::slotYourExport() {
YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->currentItem());
   if (!x) return;

   KSSLPKCS12 *pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPass());
   if (!pkcs)
      pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPassCache());
   if (!pkcs) {
      QString pprompt = i18n("Enter the certificate password:");
      QString pcaption = i18n("Password For '%1'", x->getName());
      QString oldpass;
      do {
         KPasswordDialog dlg(this);
         dlg.setPrompt(pprompt);
         dlg.setCaption(pcaption);
         if(!dlg.exec())
            return;
         pkcs = KSSLPKCS12::fromString(x->getPKCS(), dlg.password());
         pprompt = i18n("Decoding failed. Please try again:");
      } while (!pkcs);
      x->setPassCache(oldpass);
      slotYourUnlock();
   }

   // For now, we will only export to PKCS#12
   QString certFile = KFileDialog::getSaveFileName(QString(),
		                                   "application/x-pkcs12",
                                                   this);
   if (!certFile.isEmpty() && !pkcs->toFile(certFile))
      KMessageBox::sorry(this, i18n("Export failed."), i18n("SSL"));

   delete pkcs;
}


void KCryptoConfig::slotYourVerify() {
YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->currentItem());
QString iss;
   if (!x) return;

   KSSLPKCS12 *pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPass());
   if (!pkcs)
      pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPassCache());
   if (!pkcs) {
      QString pprompt = i18n("Enter the certificate password:");
      QString pcaption = i18n("Password For '%1'", x->getName());
      QString oldpass;
      do {
         KPasswordDialog dlg(this);
         dlg.setPrompt(pprompt);
         dlg.setCaption(pcaption);
         if(!dlg.exec())
            return;

         pkcs = KSSLPKCS12::fromString(x->getPKCS(), dlg.password());
         pprompt = i18n("Decoding failed. Please try again:");
      } while (!pkcs);
      x->setPassCache(oldpass);
      slotYourUnlock();
   }

  KSSLCertificate::KSSLValidation v = pkcs->revalidate(KSSLCertificate::SSLClient);
  if (v != KSSLCertificate::Ok)
	  v = pkcs->revalidate(KSSLCertificate::SMIMEEncrypt);
  if (v != KSSLCertificate::Ok)
	  v = pkcs->revalidate(KSSLCertificate::SMIMESign);

  if (v == KSSLCertificate::Ok) {
     KMessageBox::information(this, i18n("This certificate passed the verification tests successfully."), i18n("SSL"));
  } else {
     KMessageBox::detailedError(this, i18n("This certificate has failed the tests and should be considered invalid."), KSSLCertificate::verifyText(v), i18n("SSL"));
  }

  delete pkcs;
}


void KCryptoConfig::slotYourRemove() {
YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->currentItem());
   if (x) {
      yourSSLBox->takeTopLevelItem(yourSSLBox->indexOfTopLevelItem(x));
      yourCertDelList.append(x);
      setAuthCertLists();
      configChanged();
   }
}


void KCryptoConfig::slotYourUnlock() {
YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->currentItem());
QString iss;
   if (!x || !yourSSLUnlock->isEnabled()) return;

   KSSLPKCS12 *pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPass());
   if (!pkcs)
      pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPassCache());
   if (!pkcs) {
      QString pprompt = i18n("Enter the certificate password:");
      QString pcaption = i18n("Password For '%1'", x->getName());
      QString oldpass;
      do {
         KPasswordDialog dlg(this);
         dlg.setPrompt(pprompt);
         dlg.setCaption(pcaption);
         if(!dlg.exec())
            return;

         pkcs = KSSLPKCS12::fromString(x->getPKCS(), dlg.password());
         pprompt = i18n("Decoding failed. Please try again:");
      } while (!pkcs);
      x->setPassCache(oldpass);
   }

   // update the info
   iss = pkcs->getCertificate()->getIssuer();
   //### ySubject->setValues(x->getName());
   //### yIssuer->setValues(iss);
   QPalette cspl;
   KSSLCertificate *cert = pkcs->getCertificate();
   cspl = yValidFrom->palette();
   if (QDateTime::currentDateTime() < cert->getQDTNotBefore()) {
	   cspl.setColor(QPalette::Foreground, QColor(196,33,21));
   } else {
	   cspl.setColor(QPalette::Foreground, QColor(42,153,59));
   }
   yValidFrom->setPalette(cspl);

   cspl = yValidUntil->palette();
   if (QDateTime::currentDateTime() > cert->getQDTNotAfter()) {
	   cspl.setColor(QPalette::Foreground, QColor(196,33,21));
   } else {
	   cspl.setColor(QPalette::Foreground, QColor(42,153,59));
   }
   yValidUntil->setPalette(cspl);

   yValidFrom->setText(cert->getNotBefore());
   yValidUntil->setText(cert->getNotAfter());
   yHash->setText(cert->getMD5DigestText());
   yourSSLUnlock->setEnabled(false);
   delete pkcs;
}


void KCryptoConfig::slotYourCertSelect() {
YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->currentItem());
QString iss;

   yourSSLExport->setEnabled(x != NULL);
   yourSSLPass->setEnabled(x != NULL);
   yourSSLUnlock->setEnabled(false);
   yourSSLVerify->setEnabled(x != NULL);
   yourSSLRemove->setEnabled(x != NULL);

   if (x) {
      KSSLPKCS12 *pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPass());
      if (pkcs) {
         QPalette cspl;
         KSSLCertificate *cert = pkcs->getCertificate();
         iss = cert->getIssuer();
         cspl = yValidFrom->palette();
         if (QDateTime::currentDateTime() < cert->getQDTNotBefore()) {
            cspl.setColor(QPalette::Foreground, QColor(196,33,21));
         } else {
            cspl.setColor(QPalette::Foreground, QColor(42,153,59));
         }
         yValidFrom->setPalette(cspl);

         cspl = yValidUntil->palette();
         if (QDateTime::currentDateTime() > cert->getQDTNotAfter()) {
            cspl.setColor(QPalette::Foreground, QColor(196,33,21));
         } else {
            cspl.setColor(QPalette::Foreground, QColor(42,153,59));
         }
         yValidUntil->setPalette(cspl);

         yValidFrom->setText(cert->getNotBefore());
         yValidUntil->setText(cert->getNotAfter());
         yHash->setText(cert->getMD5DigestText());
         ySubject->setCertificate(QSslCertificate(cert->toPem()), KSslCertificateBox::Subject);
         yIssuer->setCertificate(QSslCertificate(cert->toPem()), KSslCertificateBox::Issuer);
         delete pkcs;
      } else {
         yourSSLUnlock->setEnabled(x != NULL);
         yHash->clear();
         ySubject->clear();
         yIssuer->clear();
      }
   } else {
      yHash->clear();
      ySubject->clear();
      yIssuer->clear();
   }

   //### ySubject->setValues(x ? x->getName() : QString());
   //### yIssuer->setValues(iss);
}


void KCryptoConfig::slotYourPass() {
   YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->currentItem());
   QString oldpass ;
   if (!x) return;

   KSSLPKCS12 *pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPass());
   if (!pkcs)
      pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPassCache());
   if (!pkcs) {
      QString pprompt = i18n("Enter the OLD password for the certificate:");
      QString pcaption = i18n("Password For '%1'", x->getName());
      QString oldpass;
      do {
         KPasswordDialog dlg(this);
         dlg.setPrompt(pprompt);
         dlg.setCaption(pcaption);
         if(!dlg.exec())
            break;
         pkcs = KSSLPKCS12::fromString(x->getPKCS(), dlg.password());
         pprompt = i18n("Decoding failed. Please try again:");
      } while (!pkcs);
   }

   if (pkcs) {
      x->setPassCache(oldpass);
      slotYourUnlock();
      KNewPasswordDialog *kpd = new KNewPasswordDialog(this);
      kpd->setPrompt(i18n("Enter the new certificate password"));
      kpd->setAllowEmptyPasswords(true);

      int i = kpd->exec();
      if (i == KPasswordDialog::Accepted) {
         QString pass = kpd->password();
         pkcs->changePassword(oldpass, pass);
         x->setPKCS(pkcs->toString());
         x->setPassCache(pass);
         configChanged();
      }
      delete kpd;
      delete pkcs;
   }
}


void KCryptoConfig::slotCAImport() {
    QString certFile = KFileDialog::getOpenFileName(QString(), "application/x-x509-ca-cert", this);

    if (certFile.isEmpty())
        return;

#ifdef KSSL_HAVE_SSL
#define sk_free KOSSL::self()->sk_free
#define sk_num KOSSL::self()->sk_num
#define sk_value KOSSL::self()->sk_value

	// First try to load using the OpenSSL method
	X509_STORE *certStore = KOSSL::self()->X509_STORE_new();
	X509_LOOKUP *certLookup = KOSSL::self()->X509_STORE_add_lookup(certStore, KOSSL::self()->X509_LOOKUP_file());

	if (certLookup &&
	    KOSSL::self()->X509_LOOKUP_load_file(certLookup,
		                                 certFile.toLocal8Bit(),
						 X509_FILETYPE_PEM)) {
		for (int i = 0; i < sk_X509_OBJECT_num(certStore->objs); i++) {
			X509_OBJECT* x5o = sk_X509_OBJECT_value(certStore->objs, i);
			if (!x5o) continue;

			if (x5o->type != X509_LU_X509) continue;

			X509 *x5 = x5o->data.x509;
			if (!x5) continue;

			// Easier to use in this form
			KSSLCertificate *x = KSSLCertificate::fromX509(x5);

			// Only import CA's
			if (!x || !x->x509V3Extensions().certTypeCA()) {
				if (x) {
					QString emsg = x->getSubject() + ":\n" +
					i18n("This is not a signer certificate.");
				 	KMessageBox::error(this,
							   emsg,
						   	   i18n("SSL"));
					delete x;
				}
				continue;
			}

			QString name = x->getSubject();

			// search for dups
			QTreeWidgetItemIterator it(caList);
			while (*it) {
			         CAItem *m = static_cast<CAItem *>(*it);
			         if (m->configName() == name) {
				    KSSLCertificate *y = KSSLCertificate::fromString(m->getCert().toLocal8Bit());
				    if (!y) continue;
				    if (*x == *y) {
					QString emsg = name + ":\n" +
						i18n("You already have this signer certificate installed.");
				 	KMessageBox::error(this,
							   emsg,
						   	   i18n("SSL"));
					 delete x;
					 delete y;
					 x = NULL;
					 break;
			            }
				    delete y;
				 }
				 it++;
			}

			if (!x) continue;

			// Ok, add it to the list
			(new CAItem(caList,
				    name,
				    x->toString(),
				    true, true, true, this)
			 )->isNew = true;
			delete x;
		}

		  // Can the PEM code be wiped out now?
	} else {   // try to load it manually as a single X.509 DER encoded
		// ASSUMPTION: we only read one certificate in this code
		QFile qf(certFile);
		QString name;
		KSSLCertificate *x;
		qf.open(QIODevice::ReadOnly);
		QString certtext = qf.readLine();

		if (certStore) { KOSSL::self()->X509_STORE_free(certStore);
				certStore = NULL; }

		if (certtext.contains("-----BEGIN CERTIFICATE-----")) {
			qf.reset();
			certtext.clear();
			while (!qf.atEnd()) {
				QString xx = qf.readLine();
				certtext += xx;
			}
			certtext = certtext.remove("-----BEGIN CERTIFICATE-----");
			certtext = certtext.remove("-----END CERTIFICATE-----");
			certtext = certtext.trimmed();
			certtext = certtext.remove('\n');
		} else {
			// Must [could?] be DER
			qf.close();
			qf.open(QIODevice::ReadOnly);
			char *cr;
			cr = new char[qf.size()+1];
			qf.read(cr, qf.size());
			QByteArray qba(cr, qf.size());
			certtext = KCodecs::base64Encode(qba);
			delete [] cr;
		}

		qf.close();

		x = KSSLCertificate::fromString(certtext.toLatin1());

		if (!x) {
			KMessageBox::sorry(this,
				i18n("The certificate file could not be loaded."),
				i18n("SSL"));
			return;
		}

		if (!x->x509V3Extensions().certTypeCA()) {
			KMessageBox::sorry(this,
					i18n("This is not a signer certificate."),
					i18n("SSL"));
			delete x;
			return;
		}

		name = x->getSubject();

		QTreeWidgetItemIterator it(caList);
		while (*it) {
		         CAItem *i = static_cast<CAItem *>(*it);
		         if (i->configName() == name) {
			    KSSLCertificate *y = KSSLCertificate::fromString(i->getCert().toLocal8Bit());
			    if (!y) continue;
			    if (*x == *y) {
				 KMessageBox::error(this,
				    i18n("You already have this signer certificate installed."),
				    i18n("SSL"));
				 delete x;
				 delete y;
				 return;
			    }
			    delete y;
			 }
			 it++;
		}

		(new CAItem(caList, name, x->toString(), true, true, true, this))->isNew = true;

		delete x;
	}


	if (certStore) KOSSL::self()->X509_STORE_free(certStore);

	configChanged();
#undef sk_free
#undef sk_num
#undef sk_value
#endif

        offerImportToKMail( certFile );
}

void KCryptoConfig::offerImportToKMail( const QString& certFile )
{
    QString kleopatraPath = KStandardDirs::findExe("kleopatra");
    if (kleopatraPath.isEmpty())
        return;
    if ( KMessageBox::questionYesNo( this, i18n( "Do you want to make this certificate available to KMail as well?" ), QString(), KGuiItem(i18n("Make Available")), KGuiItem(i18n("Do Not Make Available")) ) == KMessageBox::Yes ) {
       QStringList args = QStringList() << "--import-certificate" << certFile;
       if ( !QProcess::startDetached( kleopatraPath, args ) )
           KMessageBox::error( this, i18n( "Could not execute Kleopatra. You might have to install or update the kdepim package." ) );
   }
}


void KCryptoConfig::slotCARemove() {
CAItem *x = static_cast<CAItem *>(caList->currentItem());
 if (x) {
    caList->takeTopLevelItem(caList->indexOfTopLevelItem(x));
    caDelList.append(x);
    configChanged();
    slotCAItemChanged();
 }
}


void KCryptoConfig::slotCARestore() {

 int rc = KMessageBox::warningContinueCancel(this, i18n("This will revert your certificate signers database to the KDE default.\nThis operation cannot be undone.\nAre you sure you wish to continue?"), i18n("SSL"),KGuiItem(i18n("Revert")));
      if (rc == KMessageBox::Cancel) {
          return;
      }

// For now, we just rm the existing file and rebuild

   QString path = KGlobal::dirs()->saveLocation("config");

   path += "/ksslcalist";

   QFile::remove(path);

   // Remove all our old work and rebuild the GUI/List
   caDelList.clear();
   caList->clear();


  QStringList groups = _signers->list();
  KConfig sigcfg( "ksslcalist", KConfig::NoGlobals );

  for (QStringList::const_iterator i = groups.constBegin();
                             i != groups.constEnd();
                             ++i) {
    if (!sigcfg.hasGroup(*i)) continue;
    KConfigGroup _cg(&sigcfg, *i);
    if (!_cg.hasKey("x509")) continue;
                new CAItem(caList,
                     (*i),
                     _cg.readEntry("x509", QString()),
                     _cg.readEntry("site", false),
                     _cg.readEntry("email", false),
                     _cg.readEntry("code", false),
                     this );
  }

   genCAList();
   slotCAItemChanged();
}


void KCryptoConfig::slotCAItemChanged() {
 CAItem *x = static_cast<CAItem *>(caList->currentItem());
 if (x) {
    caSSLRemove->setEnabled(true);
    //### caSubject->setValues(x->getName());
    KSSLCertificate *cert = KSSLCertificate::fromString(x->getCert().toLocal8Bit());
    if (!cert) {
       //### caIssuer->setValues(QString());
       caSite->setEnabled(false);
       caEmail->setEnabled(false);
       caCode->setEnabled(false);
       caSite->setChecked(false);
       caEmail->setChecked(false);
       caCode->setChecked(false);
       cHash->clear();
    } else {
       caSite->setEnabled(cert->x509V3Extensions().certTypeSSLCA());
       caEmail->setEnabled(cert->x509V3Extensions().certTypeEmailCA());
       caCode->setEnabled(cert->x509V3Extensions().certTypeCodeCA());
       caSite->setChecked(x->getSite());
       caEmail->setChecked(x->getEmail());
       caCode->setChecked(x->getCode());
       //### caIssuer->setValues(cert->getIssuer());
       cHash->setText(cert->getMD5DigestText());
       delete cert;
    }
 } else {
    caSSLRemove->setEnabled(false);
    caSite->setEnabled(false);
    caEmail->setEnabled(false);
    caCode->setEnabled(false);
    caSubject->clear();
    caIssuer->clear();
    cHash->clear();
 }
}


void KCryptoConfig::slotCAChecked() {
CAItem *x = static_cast<CAItem *>(caList->currentItem());
 if (x) {
   x->setSite(caSite->isChecked());
   x->setEmail(caEmail->isChecked());
   x->setCode(caCode->isChecked());
   x->modified = true;
   configChanged();
 }
}



void KCryptoConfig::slotNewHostAuth() {
    HostAuthItem *j = new HostAuthItem(hostAuthList,
                                       QString(),
                                       QString(),
                                       this );
    j->setAction(KSSLCertificateHome::AuthSend);
    hostAuthList->setCurrentItem(j);
  authHost->setEnabled(true);
  hostCertBox->setEnabled(true);
  hostCertGroupBox->setEnabled(true);
  authRemove->setEnabled(true);

  authHost->setFocus();
}



void KCryptoConfig::slotRemoveHostAuth() {
QTreeWidgetItem *act = hostAuthList->currentItem();
HostAuthItem *x = static_cast<HostAuthItem *>(act);

  if (x) {
      QTreeWidgetItem *next = hostAuthList->itemBelow(act);
      if (!next) next = hostAuthList->itemAbove(act);
      hostAuthList->takeTopLevelItem(hostAuthList->indexOfTopLevelItem(x));
      authDelList.append(x);
      configChanged();
      if (next) hostAuthList->setCurrentItem(next);
  }
}


void KCryptoConfig::slotAuthItemChanged() {
HostAuthItem *x = static_cast<HostAuthItem *>(hostAuthList->currentItem());

if (x) {
  // Make sure the fields are enabled
  authHost->setEnabled(true);
  hostCertBox->setEnabled(true);
  hostCertGroupBox->setEnabled(true);
  authRemove->setEnabled(true);

  // Populate the fields
  switch(x->getAction()) {
  case KSSLCertificateHome::AuthSend:
    hostSend->setChecked(true);
   break;
  case KSSLCertificateHome::AuthPrompt:
    hostPrompt->setChecked(true);
   break;
  case KSSLCertificateHome::AuthDont:
    hostDont->setChecked(true);
   break;
  default:
    hostSend->setChecked(false);
    hostPrompt->setChecked(false);
    hostDont->setChecked(false);
   break;
  }

  ___lehack = true;
  authHost->setText(x->configName());
  ___lehack = false;
  hostCertBox->setCurrentIndex(0);

  QString theCert = x->getCertName();
  for (int i = 0; i < hostCertBox->count(); i++) {
    if (hostCertBox->itemText(i) == theCert) {
       hostCertBox->setCurrentIndex(i);
       break;
    }
  }
} else {
  authHost->clear();
  authHost->setEnabled(false);
  hostCertBox->setEnabled(false);
  hostCertGroupBox->setEnabled(false);
  authRemove->setEnabled(false);
}
}


void KCryptoConfig::slotAuthText(const QString &t) {
if (___lehack) return;
HostAuthItem *x = static_cast<HostAuthItem *>(hostAuthList->currentItem());

if (x) {
   x->setHost(t);
configChanged();
}
}



void KCryptoConfig::slotAuthButtons() {
HostAuthItem *x = static_cast<HostAuthItem *>(hostAuthList->currentItem());

if (x) {
   KSSLCertificateHome::KSSLAuthAction aa = KSSLCertificateHome::AuthDont;

   if (hostSend->isChecked())
    aa = KSSLCertificateHome::AuthSend;
   else if (hostPrompt->isChecked())
    aa = KSSLCertificateHome::AuthPrompt;
   else
    aa =  KSSLCertificateHome::AuthDont;

   x->setAction(aa);
configChanged();
}
}


void KCryptoConfig::slotAuthCombo() {
HostAuthItem *x = static_cast<HostAuthItem *>(hostAuthList->currentItem());

  if (x) {
    if (hostCertBox->currentIndex() == 0)
      x->setCertName(QString());
    else x->setCertName(hostCertBox->currentText());
    configChanged();
  }
}


void KCryptoConfig::slotTestOSSL() {
KOSSL::self()->destroy();

if (!KOSSL::self()->hasLibSSL()) {
   KMessageBox::detailedSorry(this,
                              i18n("Failed to load OpenSSL."),
                              i18n("libssl was not found or successfully loaded."),
                              i18n("OpenSSL"));
   return;
}

if (!KOSSL::self()->hasLibCrypto()) {
   KMessageBox::detailedSorry(this,
                              i18n("Failed to load OpenSSL."),
                              i18n("libcrypto was not found or successfully loaded."),
                              i18n("OpenSSL"));
   return;
}

   KMessageBox::information(this, i18n("OpenSSL was successfully loaded."), i18n("OpenSSL"));
}


void KCryptoConfig::slotUseEGD() {
  if (mUseEGD->isChecked()) {
    mUseEFile->setChecked(false);
  }
  mEGDLabel->setText(i18n("Path to EGD:"));
  mEGDPath->setEnabled(mUseEGD->isChecked());
  mEGDLabel->setEnabled(mUseEGD->isChecked());
  configChanged();
}


void KCryptoConfig::slotUseEFile() {
  if (mUseEFile->isChecked()) {
    mUseEGD->setChecked(false);
  }
  mEGDLabel->setText(i18n("Path to entropy file:"));
  mEGDPath->setEnabled(mUseEFile->isChecked());
  mEGDLabel->setEnabled(mUseEFile->isChecked());
  configChanged();
}


//  Lets make this a separate module.  it's a whole lot of work and can really be
// encompassed in a separate module quite nicely.
void KCryptoConfig::slotGeneratePersonal() {
#if 0
  QStringList qslCertTypes;

  qslCertTypes << i18n("Personal SSL")
               << i18n("Server SSL")
               << i18n("S/MIME")
               << i18n("PGP")
               << i18n("GPG")
               << i18n("SSL Personal Request")
               << i18n("SSL Server Request")
               << i18n("Netscape SSL")
               << i18nc("Server certificate authority", "Server CA")
               << i18nc("Personal certificate authority", "Personal CA")
               << i18nc("Secure MIME certificate authority", "S/MIME CA");
#endif

}


#ifdef KSSL_HAVE_SSL
// This gets all the available ciphers from OpenSSL
bool KCryptoConfig::loadCiphers() {
unsigned int i;
SSL_CTX *ctx;
SSL *ssl;
SSL_METHOD *meth;

  SSLv3Box->clear();
  meth = SSLv3_client_method();
  SSLeay_add_ssl_algorithms();
  ctx = SSL_CTX_new(meth);
  if (ctx == NULL) return false;

  ssl = SSL_new(ctx);
  if (!ssl) return false;

  for (i=0; ; i++) {
    int j, k;
    SSL_CIPHER *sc;
    sc = (meth->get_cipher)(i);
    if (!sc)
      break;
    // Leak of sc*?
    QString scn(sc->name);
    if (scn.contains("ADH-") || scn.contains("NULL-") || scn.contains("FZA-") || scn.contains("DES-CBC3-SHA")) {
      continue;
    }
    k = SSL_CIPHER_get_bits(sc, &j);

    new CipherItem( SSLv3Box, sc->name, k, j, this );
  }

  if (ctx) SSL_CTX_free(ctx);
  if (ssl) SSL_free(ssl);

return true;
}
#endif


void KCryptoConfig::setAuthCertLists() {
QString oldDef, oldHost;
bool noneDef, noneHost;

// get the old setting
  oldDef = defCertBox->currentText();
  oldHost = hostCertBox->currentText();
  noneDef = (0 == defCertBox->currentIndex());
  noneHost = (0 == hostCertBox->currentIndex());

// repopulate
  defCertBox->clear();
  hostCertBox->clear();
  QStringList defCertStrList; // = KSSLCertificateHome::getCertificateList();
  defCertStrList.append(i18nc("No Certificates on List", "None"));
  QTreeWidgetItemIterator it(yourSSLBox);
  while (*it) {
     defCertStrList.append(static_cast<YourCertItem *>(*it)->configName());
     it++;
  }
  defCertBox->addItems(defCertStrList);
  hostCertBox->addItems(defCertStrList);

// try to restore it

  defCertBox->setCurrentIndex(0);
  if (!noneDef) {
    for (int i = 0; i < defCertBox->count(); i++) {
      if (defCertBox->itemText(i) == oldDef) {
         defCertBox->setCurrentIndex(i);
         break;
      }
    }
  }

  if (!noneDef && defCertBox->currentIndex() == 0) configChanged();

  hostCertBox->setCurrentIndex(0);
  if (!noneHost) {
    for (int i = 0; i < hostCertBox->count(); i++) {
      if (hostCertBox->itemText(i) == oldHost) {
         hostCertBox->setCurrentIndex(i);
         break;
      }
    }
  }

  if (!noneHost && hostCertBox->currentIndex() == 0) configChanged();

  // Update the host entries too
  it = QTreeWidgetItemIterator(hostAuthList);
  while (*it) {
     HostAuthItem *x = static_cast<HostAuthItem *>(*it);
     QString newValue;
     for (int i = 1; i < hostCertBox->count(); i++) {
        if (hostCertBox->itemText(i) == x->getCertName()) {
           newValue = x->getCertName();
           break;
        }
     }
     if (newValue != x->getCertName()) configChanged();
     x->setCertName(newValue);
     it++;
  }


}

#include "crypto.moc"
