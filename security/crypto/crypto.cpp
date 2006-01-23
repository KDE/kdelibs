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

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>

#include <q3buttongroup.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qframe.h>
#include <q3groupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qregexp.h>

//Added by qt3to4:
#include <QGridLayout>
#include <QBoxLayout>
#include <QVBoxLayout>

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
#include <kpassdlg.h>
#include <kprocess.h>
#include <kpushbutton.h>
#include <kresolver.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <kurllabel.h>
#include <kurlrequester.h>
#include <kvbox.h>

#include <config.h>
#ifdef HAVE_SSL
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

#include "crypto.h"
#include "certexport.h"
#include "kdatetimedlg.h"

using namespace KNetwork;

typedef KGenericFactory<KCryptoConfig, QWidget> KryptoFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_crypto, KryptoFactory("kcmcrypto") )

CipherItem::CipherItem( Q3ListView *view, const QString& cipher, int bits,
			int maxBits, KCryptoConfig *module )
    : Q3CheckListItem( view, QString(), CheckBox )
{
    m_cipher = cipher;
    m_bits = bits;
    m_module = module;

    QString tmp( i18n("%1 (%2 of %3 bits)") );
    setText( 0, tmp.arg( cipher ).arg( bits ).arg( maxBits ));
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



OtherCertItem::OtherCertItem( Q3ListView *view, const QString& sub, const QString& md5, bool perm, int policy, QDateTime exp, KCryptoConfig *module )
    : Q3ListViewItem( view, QString() ), _sub(sub), _md5(md5), _exp(exp), _perm(perm), _policy(policy)

{
    m_module = module;
KSSLX509Map cert(sub);
    setText(0, cert.getValue("O"));
    setText(1, cert.getValue("CN").replace("\n", ", "));

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


YourCertItem::YourCertItem( Q3ListView *view, QString pkcs, QString pass, QString name, KCryptoConfig *module )
    : Q3ListViewItem( view, QString() )

{
    m_module = module;
KSSLX509Map cert(name);
    QString tmp = cert.getValue("CN").replace("\n", ", ");
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



CAItem::CAItem( Q3ListView *view, QString name, QString cert, bool site, bool email, bool code, KCryptoConfig *module )
    : Q3ListViewItem( view, QString() )

{
    m_module = module;
KSSLX509Map mcert(name);
QString tmp;
    setText(0, mcert.getValue("O"));
    tmp = mcert.getValue("OU");
    tmp.replace("\n", ", ");
    setText(1, tmp);
    tmp = mcert.getValue("CN");
    tmp.replace("\n", ", ");
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




KCryptoConfig::KCryptoConfig(QWidget *parent, const char *name, const QStringList &)
  : KCModule(KryptoFactory::instance(), parent)
{
QGridLayout *grid;
QBoxLayout *top = new QVBoxLayout(this);
QString whatstr;

  setQuickHelp( i18n("<h1>Crypto</h1> This module allows you to configure SSL for"
     " use with most KDE applications, as well as manage your personal"
     " certificates and the known certificate authorities."));

  ___lehack = false;
  otherCertDelList.setAutoDelete(true);
  yourCertDelList.setAutoDelete(true);
  authDelList.setAutoDelete(true);
  caDelList.setAutoDelete(true);

  _signers = new KSSLSigners;

     KAboutData *about =
     new KAboutData(I18N_NOOP("kcmcrypto"), I18N_NOOP("KDE Crypto Control Module"),
                    0, 0, KAboutData::License_GPL,
                    I18N_NOOP("(c) 2000 - 2001 George Staikos"));
     about->addAuthor("George Staikos", 0, "staikos@kde.org");
     about->addAuthor("Carsten Pfeiffer", 0, "pfeiffer@kde.org");
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

  tabs = new QTabWidget(this);
  top->addWidget(tabs);

  ///////////////////////////////////////////////////////////////////////////
  // FIRST TAB
  ///////////////////////////////////////////////////////////////////////////
  tabSSL = new QFrame(this);
  grid = new QGridLayout(tabSSL, 7, 2, KDialog::marginHint(),
                                       KDialog::spacingHint() );
  // no need to parse kdeglobals.
  config = new KConfig("cryptodefaults", false, false);
  policies = new KSimpleConfig("ksslpolicies", false);
  pcerts = new KSimpleConfig("ksslcertificates", false);
  authcfg = new KSimpleConfig("ksslauthmap", false);

#ifdef HAVE_SSL
  SSLv3Box = new Q3ListView(tabSSL, "v3ciphers");
  SSLv3Box->addColumn(i18n("SSL Ciphers to Use"));
  whatstr = i18n("Select the ciphers you wish to enable when using the"
                " SSL protocol. The actual protocol used will be"
                " negotiated with the server at connection time.");
  SSLv3Box->setWhatsThis( whatstr);
  SSLv3Box->setSelectionMode(Q3ListView::NoSelection);
  grid->addMultiCellWidget(SSLv3Box, 2, 2, 0, 1);
  loadCiphers();

  //
  //  CipherWizards
  //
  Q3GroupBox *cwbg = new Q3GroupBox(i18n("Cipher Wizard"), tabSSL);
  cwbg->setOrientation(Qt::Horizontal);
  QVBoxLayout *laygroup2 = new QVBoxLayout(cwbg->layout(), KDialog::spacingHint() );
  QComboBox *cwcb = new QComboBox(cwbg);
    laygroup2->addWidget( cwcb );
  grid->addMultiCellWidget(cwbg, 3, 3, 0, 1);
  QString whatStr = i18n("<qt>Use these preconfigurations to more easily configure the SSL encryption settings. You can choose among the following modes: <ul>");

  cwcb->insertItem(QString());
  cwcb->insertItem(i18n("Strong Ciphers Only"));
  whatStr += i18n("<li><b>Strong Ciphers Only:</b> Select only the strong (&gt;= 128 bit) encryption ciphers.</li>");
  cwcb->insertItem(i18n("Export Ciphers Only"));
  whatStr += i18n("<li><b>Export Ciphers Only:</b> Select only the weak ciphers (&lt;= 56 bit).</li>");
  cwcb->insertItem(i18n("Enable All"));
  whatStr += i18n("<li><b>Enable All:</b> Select all SSL ciphers and methods.</li></ul>");

  cwcb->setWhatsThis( whatStr);

  connect(cwcb, SIGNAL(activated(int)), SLOT(slotSelectCipher(int)));

#endif

  mWarnOnEnter = new QCheckBox(i18n("Warn on &entering SSL mode"), tabSSL);
  connect(mWarnOnEnter, SIGNAL(clicked()), SLOT(configChanged()));
  grid->addWidget(mWarnOnEnter, 5, 0);
  whatstr = i18n("If selected, you will be notified when entering an SSL"
                " enabled site");
  mWarnOnEnter->setWhatsThis( whatstr);

  mWarnOnLeave = new QCheckBox(i18n("Warn on &leaving SSL mode"), tabSSL);
  connect(mWarnOnLeave, SIGNAL(clicked()), SLOT(configChanged()));
  grid->addWidget(mWarnOnLeave, 5, 1);
  whatstr = i18n("If selected, you will be notified when leaving an SSL"
                " based site.");
  mWarnOnLeave->setWhatsThis( whatstr);

  mWarnOnUnencrypted = new QCheckBox(i18n("Warn on sending &unencrypted data"), tabSSL);
  connect(mWarnOnUnencrypted, SIGNAL(clicked()), SLOT(configChanged()));
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

#ifdef HAVE_SSL
  tabOSSL = new QFrame(this);
  QBoxLayout *vbox = new QVBoxLayout(tabOSSL, KDialog::marginHint(), KDialog::spacingHint());

  oInfo = new Q3GroupBox(i18n("Path to OpenSSL Shared Libraries"), tabOSSL);
  oInfo->setOrientation(Qt::Vertical);
  QVBoxLayout *laygroup1 = new QVBoxLayout(oInfo->layout(), KDialog::spacingHint() );
  vbox->addWidget(oInfo);
  oPath = new KUrlRequester(oInfo);
  laygroup1->addWidget( oPath );
  oPath->setMode(KFile::Directory);
  oTest = new QPushButton(i18n("&Test"), oInfo);
  laygroup1->addWidget( oTest );
  connect(oTest, SIGNAL(clicked()), SLOT(slotTestOSSL()));

  connect(oPath, SIGNAL(textChanged(const QString&)), SLOT(configChanged()));

  //
  //  Settings for the EGD
  //
  QFrame *eFrame = new QFrame(tabOSSL);
  QVBoxLayout *egrid = new QVBoxLayout(eFrame);
  mUseEGD = new QCheckBox(i18n("Use EGD"), eFrame);
  connect(mUseEGD, SIGNAL(clicked()), SLOT(slotUseEGD()));
  mUseEFile = new QCheckBox(i18n("Use entropy file"), eFrame);
  connect(mUseEFile, SIGNAL(clicked()), SLOT(slotUseEFile()));
  vbox->addWidget(eFrame);
  egrid->addWidget(mUseEGD);
  egrid->addWidget(mUseEFile);
  QFrame *egdframe = new QFrame(tabOSSL);
  QGridLayout *grid2 = new QGridLayout(egdframe, 2, 2, KDialog::marginHint(),
                                                       KDialog::spacingHint());
  mEGDLabel = new QLabel(i18n("Path to EGD:"), egdframe);
  grid2->addWidget(mEGDLabel, 0, 0);
  mEGDPath = new KUrlRequester(egdframe);
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

#ifdef HAVE_SSL
  grid = new QGridLayout(tabYourSSLCert, 16, 6, KDialog::marginHint(), KDialog::spacingHint() );

  yourSSLBox = new Q3ListView(tabYourSSLCert);
  yourSSLBox->setAllColumnsShowFocus(true);
  whatstr = i18n("This list box shows which certificates of yours KDE"
                " knows about. You can easily manage them from here.");
  yourSSLBox->setWhatsThis( whatstr);
  grid->addMultiCellWidget(yourSSLBox, 0, 5, 0, 4);
  yourSSLBox->addColumn(i18n("Common Name"));
  yourSSLBox->addColumn(i18n("Email Address"));
  connect(yourSSLBox, SIGNAL(selectionChanged()), SLOT(slotYourCertSelect()));

  yourSSLImport = new QPushButton(i18n("I&mport..."), tabYourSSLCert);
  connect(yourSSLImport, SIGNAL(clicked()), SLOT(slotYourImport()));
  grid->addWidget(yourSSLImport, 0, 5);

  yourSSLExport = new QPushButton(i18n("&Export..."), tabYourSSLCert);
  yourSSLExport->setEnabled(false);
  connect(yourSSLExport, SIGNAL(clicked()), SLOT(slotYourExport()));
  grid->addWidget(yourSSLExport, 1, 5);

  yourSSLRemove = new QPushButton(i18n("Remo&ve"), tabYourSSLCert);
  yourSSLRemove->setEnabled(false);
  connect(yourSSLRemove, SIGNAL(clicked()), SLOT(slotYourRemove()));
  grid->addWidget(yourSSLRemove, 2, 5);

  yourSSLUnlock = new QPushButton(i18n("&Unlock"), tabYourSSLCert);
  yourSSLUnlock->setEnabled(false);
  connect(yourSSLUnlock, SIGNAL(clicked()), SLOT(slotYourUnlock()));
  grid->addWidget(yourSSLUnlock, 3, 5);

  yourSSLVerify = new QPushButton(i18n("Verif&y"), tabYourSSLCert);
  yourSSLVerify->setEnabled(false);
  connect(yourSSLVerify, SIGNAL(clicked()), SLOT(slotYourVerify()));
  grid->addWidget(yourSSLVerify, 4, 5);

  yourSSLPass = new QPushButton(i18n("Chan&ge Password..."), tabYourSSLCert);
  yourSSLPass->setEnabled(false);
  connect(yourSSLPass, SIGNAL(clicked()), SLOT(slotYourPass()));
  grid->addWidget(yourSSLPass, 5, 5);

  grid->addMultiCellWidget(new KSeparator(Qt::Horizontal, tabYourSSLCert), 6, 6, 0, 5);
  ySubject = KSSLInfoDlg::certInfoWidget(tabYourSSLCert, QString(QString()));
  yIssuer = KSSLInfoDlg::certInfoWidget(tabYourSSLCert, QString(QString()));
  grid->addMultiCellWidget(ySubject, 7, 11, 0, 2);
  grid->addMultiCellWidget(yIssuer, 7, 11, 3, 5);
  whatstr = i18n("This is the information known about the owner of the certificate.");
  ySubject->setWhatsThis( whatstr);
  whatstr = i18n("This is the information known about the issuer of the certificate.");
  yIssuer->setWhatsThis( whatstr);

  grid->addWidget(new QLabel(i18n("Valid from:"), tabYourSSLCert), 12, 0);
  grid->addWidget(new QLabel(i18n("Valid until:"), tabYourSSLCert), 13, 0);
  yValidFrom = new QLabel(tabYourSSLCert);
  grid->addWidget(yValidFrom, 12, 1);
  yValidUntil = new QLabel(tabYourSSLCert);
  grid->addWidget(yValidUntil, 13, 1);
  whatstr = i18n("The certificate is valid starting at this date.");
  yValidFrom->setWhatsThis( whatstr);
  whatstr = i18n("The certificate is valid until this date.");
  yValidUntil->setWhatsThis( whatstr);
  grid->addWidget(new QLabel(i18n("MD5 digest:"), tabYourSSLCert), 14, 0);
  yHash = new QLabel(tabYourSSLCert);
  grid->addWidget(yHash, 14, 1);
  whatstr = i18n("A hash of the certificate used to identify it quickly.");
  yHash->setWhatsThis( whatstr);

#if 0
  Q3HButtonGroup *ocbg = new Q3HButtonGroup(i18n("On SSL Connection..."), tabYourSSLCert);
  yourSSLUseDefault = new QRadioButton(i18n("&Use default certificate"), ocbg);
  yourSSLList = new QRadioButton(i18n("&List upon connection"), ocbg);
  yourSSLDont = new QRadioButton(i18n("&Do not use certificates"), ocbg);
  grid->addMultiCellWidget(ocbg, 14, 14, 0, 5);
#endif
#else
  QLabel *nossllabel = new QLabel(i18n("SSL certificates cannot be managed"
                               " because this module was not linked"
                               " with OpenSSL."), tabYourSSLCert);
  grid->addMultiCellWidget(nossllabel, 3, 3, 0, 5);
#endif


  ///////////////////////////////////////////////////////////////////////////
  // FOURTH TAB
  ///////////////////////////////////////////////////////////////////////////
  tabAuth = new QFrame(this);

#ifdef HAVE_SSL
  grid = new QGridLayout(tabAuth, 20, 6, KDialog::marginHint(), KDialog::spacingHint());

  grid->addMultiCellWidget(new QLabel(i18n("Default Authentication Certificate"), tabAuth), 0, 0, 0, 2);
  defCertBG = new Q3VButtonGroup(i18n("Default Action"), tabAuth);
  defSend = new QRadioButton(i18n("&Send"), defCertBG);
  defPrompt = new QRadioButton(i18n("&Prompt"), defCertBG);
  defDont = new QRadioButton(i18n("Do &not send"), defCertBG);
  grid->addMultiCellWidget(defCertBG, 1, 3, 0, 2);
  grid->addMultiCellWidget(new QLabel(i18n("Default certificate:"), tabAuth), 1, 1, 3, 5);
  defCertBox = new KComboBox(false, tabAuth);
  grid->addMultiCellWidget(defCertBox, 2, 2, 3, 5);

  grid->addMultiCellWidget(new KSeparator(Qt::Horizontal, tabAuth), 4, 4, 0, 5);


  grid->addMultiCellWidget(new QLabel(i18n("Host authentication:"), tabAuth), 5, 5, 0, 1);
  hostAuthList = new Q3ListView(tabAuth);
  hostAuthList->setAllColumnsShowFocus(true);
  grid->addMultiCellWidget(hostAuthList, 6, 13, 0, 5);
  hostAuthList->addColumn(i18n("Host"));
  hostAuthList->addColumn(i18n("Certificate"));
  hostAuthList->addColumn(i18n("Policy"));

  grid->addWidget(new QLabel(i18n("Host:"), tabAuth), 14, 0);
  grid->addWidget(new QLabel(i18n("Certificate:"), tabAuth), 15, 0);

  authHost = new QLineEdit(tabAuth);
  grid->addMultiCellWidget(authHost, 14, 14, 1, 4);
  hostCertBox = new KComboBox(false, tabAuth);
  grid->addMultiCellWidget(hostCertBox, 15, 15, 1, 4);

  hostCertBG = new Q3HButtonGroup(i18n("Action"), tabAuth);
  hostSend = new QRadioButton(i18n("Send"), hostCertBG);
  hostPrompt = new QRadioButton(i18n("Prompt"), hostCertBG);
  hostDont = new QRadioButton(i18n("Do not send"), hostCertBG);
  grid->addMultiCellWidget(hostCertBG, 16, 16, 0, 5);

  authAdd = new QPushButton(i18n("Ne&w"), tabAuth);
  authRemove = new QPushButton(i18n("Remo&ve"), tabAuth);
  grid->addWidget(authAdd, 17, 4);
  grid->addWidget(authRemove, 17, 5);

  authHost->setEnabled(false);
  hostCertBox->setEnabled(false);
  hostCertBG->setEnabled(false);
  authRemove->setEnabled(false);

  connect(defCertBox, SIGNAL(activated(int)), this, SLOT(configChanged()));
  connect(defCertBG, SIGNAL(clicked(int)), this, SLOT(configChanged()));
  connect(hostAuthList, SIGNAL(selectionChanged()), this, SLOT(slotAuthItemChanged()));
  connect(authAdd, SIGNAL(clicked()), this, SLOT(slotNewHostAuth()));
  connect(authRemove, SIGNAL(clicked()), this, SLOT(slotRemoveHostAuth()));
  connect(authHost, SIGNAL(textChanged(const QString &)), this, SLOT(slotAuthText(const QString &)));
  connect(hostCertBG, SIGNAL(clicked(int)), this, SLOT(slotAuthButtons()));
  connect(hostCertBox, SIGNAL(activated(int)), this, SLOT(slotAuthCombo()));

#else
  nossllabel = new QLabel(i18n("SSL certificates cannot be managed"
                               " because this module was not linked"
                               " with OpenSSL."), tabAuth);
  grid->addMultiCellWidget(nossllabel, 3, 3, 0, 5);
#endif



  ///////////////////////////////////////////////////////////////////////////
  // FIFTH TAB
  ///////////////////////////////////////////////////////////////////////////
  tabOtherSSLCert = new QFrame(this);

#ifdef HAVE_SSL
  oGrid = grid = new QGridLayout(tabOtherSSLCert, 21, 6, KDialog::marginHint(), KDialog::spacingHint());

  otherSSLBox = new Q3ListView(tabOtherSSLCert);
  otherSSLBox->setAllColumnsShowFocus(true);
  connect(otherSSLBox, SIGNAL(selectionChanged()), SLOT(slotOtherCertSelect()));
  whatstr = i18n("This list box shows which site and person certificates KDE"
                " knows about. You can easily manage them from here.");
  otherSSLBox->setWhatsThis( whatstr);
  otherSSLBox->addColumn(i18n("Organization"));
  otherSSLBox->addColumn(i18n("Common Name"));
  grid->addMultiCellWidget(otherSSLBox, 0, 7, 0, 4);

  otherSSLExport = new QPushButton(i18n("&Export..."), tabOtherSSLCert);
  connect(otherSSLExport, SIGNAL(clicked()), SLOT(slotExportCert()));
  grid->addWidget(otherSSLExport, 0, 5);
  whatstr = i18n("This button allows you to export the selected certificate"
                " to a file of various formats.");
  otherSSLExport->setWhatsThis( whatstr);

  otherSSLRemove = new QPushButton(i18n("&Remove"), tabOtherSSLCert);
  connect(otherSSLRemove, SIGNAL(clicked()), SLOT(slotRemoveCert()));
  grid->addWidget(otherSSLRemove, 1, 5);
  whatstr = i18n("This button removes the selected certificate"
                " from the certificate cache.");
  otherSSLRemove->setWhatsThis( whatstr);

  otherSSLVerify = new QPushButton(i18n("&Verify"), tabOtherSSLCert);
  connect(otherSSLVerify, SIGNAL(clicked()), SLOT(slotVerifyCert()));
  grid->addWidget(otherSSLVerify, 2, 5);
  whatstr = i18n("This button tests the selected certificate"
                " for validity.");
  otherSSLVerify->setWhatsThis( whatstr);

      otherSSLExport->setEnabled(false);
      otherSSLVerify->setEnabled(false);
      otherSSLRemove->setEnabled(false);

  grid->addMultiCellWidget(new KSeparator(Qt::Horizontal, tabOtherSSLCert), 8, 8, 0, 5);
  oSubject = KSSLInfoDlg::certInfoWidget(tabOtherSSLCert, QString(QString()));
  oIssuer = KSSLInfoDlg::certInfoWidget(tabOtherSSLCert, QString(QString()));
  grid->addMultiCellWidget(oSubject, 9, 13, 0, 2);
  grid->addMultiCellWidget(oIssuer, 9, 13, 3, 5);
  whatstr = i18n("This is the information known about the owner of the certificate.");
  oSubject->setWhatsThis( whatstr);
  whatstr = i18n("This is the information known about the issuer of the certificate.");
  oIssuer->setWhatsThis( whatstr);

  fromLabel = new QLabel(i18n("Valid from:"), tabOtherSSLCert);
  untilLabel = new QLabel(i18n("Valid until:"), tabOtherSSLCert);
  grid->addWidget(fromLabel, 14, 0);
  grid->addWidget(untilLabel, 15, 0);
  fromLabel->setEnabled(false);
  untilLabel->setEnabled(false);
  validFrom = new QLabel(tabOtherSSLCert);
  grid->addWidget(validFrom, 14, 1);
  validUntil = new QLabel(tabOtherSSLCert);
  grid->addWidget(validUntil, 15, 1);
  whatstr = i18n("The certificate is valid starting at this date.");
  validFrom->setWhatsThis( whatstr);
  whatstr = i18n("The certificate is valid until this date.");
  validUntil->setWhatsThis( whatstr);

  cacheGroup = new Q3VButtonGroup(i18n("Cache"), tabOtherSSLCert);
  cachePerm = new QRadioButton(i18n("Permanentl&y"), cacheGroup);
  cacheUntil = new QRadioButton(i18n("&Until"), cacheGroup);
  untilDate = new KURLLabel(QString(), QString(), cacheGroup);
  cacheGroup->setEnabled(false);
  grid->addMultiCellWidget(cacheGroup, 16, 19, 0, 2);

  cachePerm->setEnabled(false);
  cacheUntil->setEnabled(false);
  untilDate->setEnabled(false);
  connect(cachePerm, SIGNAL(clicked()), SLOT(slotPermanent()));
  connect(cacheUntil, SIGNAL(clicked()), SLOT(slotUntil()));
  connect(untilDate, SIGNAL(leftClickedURL()), SLOT(slotDatePick()));
  whatstr = i18n("Select here to make the cache entry permanent.");
  cachePerm->setWhatsThis( whatstr);
  whatstr = i18n("Select here to make the cache entry temporary.");
  cacheUntil->setWhatsThis( whatstr);
  whatstr = i18n("The date and time until the certificate cache entry should expire.");
  untilDate->setWhatsThis( whatstr);

  policyGroup = new Q3VButtonGroup(i18n("Policy"), tabOtherSSLCert);
  policyAccept = new QRadioButton(i18n("Accep&t"), policyGroup);
  policyReject = new QRadioButton(i18n("Re&ject"), policyGroup);
  policyPrompt = new QRadioButton(i18n("&Prompt"), policyGroup);
  policyGroup->setEnabled(false);
  grid->addMultiCellWidget(policyGroup, 16, 19, 3, 5);
  connect(policyGroup, SIGNAL(clicked(int)), SLOT(slotPolicyChanged(int)));
  whatstr = i18n("Select this to always accept this certificate.");
  policyAccept->setWhatsThis( whatstr);
  whatstr = i18n("Select this to always reject this certificate.");
  policyReject->setWhatsThis( whatstr);
  whatstr = i18n("Select this if you wish to be prompted for action when receiving this certificate.");
  policyPrompt->setWhatsThis( whatstr);
  grid->addWidget(new QLabel(i18n("MD5 digest:"), tabOtherSSLCert), 20, 0);
  pHash = new QLabel(tabOtherSSLCert);
  grid->addWidget(pHash, 20, 1);
  whatstr = i18n("A hash of the certificate used to identify it quickly.");
  pHash->setWhatsThis( whatstr);

#else
  nossllabel = new QLabel(i18n("SSL certificates cannot be managed"
                               " because this module was not linked"
                               " with OpenSSL."), tabOtherSSLCert);
  grid->addMultiCellWidget(nossllabel, 1, 1, 0, 1);
#endif


  ///////////////////////////////////////////////////////////////////////////
  // SIXTH TAB
  ///////////////////////////////////////////////////////////////////////////
  tabSSLCA = new QFrame(this);

#ifdef HAVE_SSL
  grid = new QGridLayout(tabSSLCA, 11, 8, KDialog::marginHint(), KDialog::spacingHint());

  caList = new Q3ListView(tabSSLCA);
  caList->setAllColumnsShowFocus(true);
  whatstr = i18n("This list box shows which certificate authorities KDE"
                 " knows about. You can easily manage them from here.");
  caList->setWhatsThis( whatstr);
  grid->addMultiCellWidget(caList, 0, 3, 0, 6);
  caList->addColumn(i18n("Organization"));
  caList->addColumn(i18n("Organizational Unit"));
  caList->addColumn(i18n("Common Name"));
  connect(caList, SIGNAL(selectionChanged()), SLOT(slotCAItemChanged()));

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

  caSubject = KSSLInfoDlg::certInfoWidget(tabSSLCA, QString(QString()));
  caIssuer = KSSLInfoDlg::certInfoWidget(tabSSLCA, QString(QString()));
  grid->addMultiCellWidget(caSubject, 4, 6, 0, 3);
  grid->addMultiCellWidget(caIssuer, 4, 6, 4, 7);

  // Accept for Web Site Signing, Email Signing, Code Signing
  caSite = new QCheckBox(i18n("Accept for site signing"), tabSSLCA);
  caEmail = new QCheckBox(i18n("Accept for email signing"), tabSSLCA);
  caCode = new QCheckBox(i18n("Accept for code signing"), tabSSLCA);
  grid->addMultiCellWidget(caSite, 7, 7, 0, 3);
  connect(caSite, SIGNAL(clicked()), SLOT(slotCAChecked()));
  grid->addMultiCellWidget(caEmail, 8, 8, 0, 3);
  connect(caEmail, SIGNAL(clicked()), SLOT(slotCAChecked()));
  grid->addMultiCellWidget(caCode, 9, 9, 0, 3);
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
  grid->addMultiCellWidget(nossllabel, 1, 1, 0, 1);
#endif


#if 0
  ///////////////////////////////////////////////////////////////////////////
  // SEVENTH TAB
  ///////////////////////////////////////////////////////////////////////////
  tabSSLCOpts = new QFrame(this);

#ifdef HAVE_SSL
  grid = new QGridLayout(tabSSLCOpts, 9, 4, KDialog::marginHint(), KDialog::spacingHint());
  mWarnSelfSigned = new QCheckBox(i18n("Warn on &self-signed certificates or unknown CA's"), tabSSLCOpts);
  connect(mWarnSelfSigned, SIGNAL(clicked()), SLOT(configChanged()));
  mWarnExpired = new QCheckBox(i18n("Warn on &expired certificates"), tabSSLCOpts);
  connect(mWarnExpired, SIGNAL(clicked()), SLOT(configChanged()));
  mWarnRevoked = new QCheckBox(i18n("Warn on re&voked certificates"), tabSSLCOpts);
  connect(mWarnRevoked, SIGNAL(clicked()), SLOT(configChanged()));
  grid->addMultiCellWidget(mWarnSelfSigned, 0, 0, 0, 3);
  grid->addMultiCellWidget(mWarnExpired, 1, 1, 0, 3);
  grid->addMultiCellWidget(mWarnRevoked, 2, 2, 0, 3);

  macCert = new QLineEdit(tabSSLCOpts);
  grid->addMultiCellWidget(macCert, 4, 4, 0, 2);

  macBox = new Q3ListBox(tabSSLCOpts);
  whatstr = i18n("This list box shows which sites you have decided to accept"
                " a certificate from even though the certificate might fail"
                " the validation procedure.");
  macBox->setWhatsThis( whatstr);
  caSSLBox->setSelectionMode(Q3ListBox::Single);
  caSSLBox->setColumnMode(Q3ListBox::FixedNumber);
  grid->addMultiCellWidget(macBox, 5, 8, 0, 2);

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
  grid->addMultiCellWidget(nossllabel, 1, 1, 0, 1);
#endif
#endif

  ///////////////////////////////////////////////////////////////////////////
  // Add the tabs and startup
  ///////////////////////////////////////////////////////////////////////////
  tabs->addTab(tabSSL, i18n("SSL"));
#ifdef HAVE_SSL
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
#ifdef HAVE_SSL
  otherCertDelList.clear();
  yourCertDelList.clear();
  authDelList.clear();
  caDelList.clear();
  config->setGroup("Warnings");
  mWarnOnEnter->setChecked(config->readEntry("OnEnter", false));
  mWarnOnLeave->setChecked(config->readEntry("OnLeave", true));
  mWarnOnUnencrypted->setChecked(config->readEntry("OnUnencrypted", true));

#if 0 // NOT IMPLEMENTED IN KDE 2.0
  mWarnOnMixed->setChecked(config->readEntry("OnMixed", true));

  config->setGroup("Validation");
  mWarnSelfSigned->setChecked(config->readEntry("WarnSelfSigned", true));
  mWarnExpired->setChecked(config->readEntry("WarnExpired", true));
  mWarnRevoked->setChecked(config->readEntry("WarnRevoked", true));
#endif

  config->setGroup("EGD");
  slotUseEGD();  // set the defaults
  if (config->readEntry("UseEGD", false)) {
    mUseEGD->setChecked(true);
    slotUseEGD();
  } else if (config->readEntry("UseEFile", false)) {
    mUseEFile->setChecked(true);
    slotUseEFile();
  }
  mEGDPath->setURL(config->readPathEntry("EGDPath"));


#ifdef HAVE_SSL
  config->setGroup("OpenSSL");
  oPath->setURL(config->readPathEntry("Path"));
#endif

  config->setGroup("SSLv3");
  CipherItem *item = static_cast<CipherItem *>(SSLv3Box->firstChild());
  while ( item ) {
      item->setOn(config->readEntry(item->configName(),
					item->bits() >= 56));
      item = static_cast<CipherItem *>(item->nextSibling());
  }

  QStringList groups = policies->groupList();

  otherSSLBox->clear();
  for (QStringList::Iterator i = groups.begin(); i != groups.end(); ++i) {
    if ((*i).isEmpty() || *i == "<default>" || *i == "General") continue;
    policies->setGroup(*i);
    KSSLCertificate *cert = KSSLCertificate::fromString(policies->readEntry("Certificate", QString()).toLocal8Bit());
    if (cert) {
      new OtherCertItem(otherSSLBox, cert->getSubject(), *i,
                        policies->readEntry("Permanent", true),
                        policies->readEntry("Policy", 3),
                        policies->readEntry("Expires", QDateTime()), this );
      delete cert;
    }
  }

  groups = pcerts->groupList();

  yourSSLBox->clear();
  for (QStringList::Iterator i = groups.begin(); i != groups.end(); ++i) {
    if ((*i).isEmpty() || *i == "<default>") continue;
    pcerts->setGroup(*i);
    YourCertItem *j = new YourCertItem(yourSSLBox,
                     pcerts->readEntry("PKCS12Base64"),
                     pcerts->readEntry("Password"),
                     *i, this );
    j->setPassCache(QString());
  }

  setAuthCertLists();

  config->setGroup("Auth");
  QString whichAuth = config->readEntry("AuthMethod", "none");
  if (whichAuth == "send")
    defCertBG->setButton(defCertBG->id(defSend));
  else if (whichAuth == "prompt")
    defCertBG->setButton(defCertBG->id(defPrompt));
  else
    defCertBG->setButton(defCertBG->id(defDont));

  QString whichCert = config->readEntry("DefaultCert");
  defCertBox->setCurrentItem(0);
  for (int i = 0; i < defCertBox->count(); i++) {
     if (defCertBox->text(i) == whichCert) {
       defCertBox->setCurrentItem(i);
       break;
     }
  }
  hostAuthList->clear();
  groups = authcfg->groupList();
  for (QStringList::Iterator i = groups.begin();
                             i != groups.end();
                             ++i) {
    if ((*i).isEmpty() || *i == "<default>") continue;
    authcfg->setGroup(*i);
    KSSLCertificateHome::KSSLAuthAction aa = KSSLCertificateHome::AuthDont;
    if (authcfg->readEntry("send", false) == true)
       aa = KSSLCertificateHome::AuthSend;
    else if (authcfg->readEntry("prompt", false) == true)
       aa = KSSLCertificateHome::AuthPrompt;
    HostAuthItem *j = new HostAuthItem(hostAuthList,
                                       KResolver::domainToAscii(*i),
                                       authcfg->readEntry("certificate"),
                                       this );
    j->setAction(aa);
    j->setOriginalName(*i);
  }

  groups = _signers->list();
  KConfig sigcfg("ksslcalist", true, false);
  caList->clear();
  for (QStringList::Iterator i = groups.begin();
                             i != groups.end();
                             ++i) {
    if ((*i).isEmpty() || *i == "<default>") continue;
    if (!sigcfg.hasGroup(*i)) continue;
    sigcfg.setGroup(*i);
    if (!sigcfg.hasKey("x509")) continue;
                new CAItem(caList,
                     (*i),
                     sigcfg.readEntry("x509"),
                     sigcfg.readEntry("site", false),
                     sigcfg.readEntry("email", false),
                     sigcfg.readEntry("code", false),
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
#ifdef HAVE_SSL
  config->setGroup("Warnings");
  config->writeEntry("OnEnter", mWarnOnEnter->isChecked());
  config->writeEntry("OnLeave", mWarnOnLeave->isChecked());
  config->writeEntry("OnUnencrypted", mWarnOnUnencrypted->isChecked());

  config->setGroup("EGD");
  config->writeEntry("UseEGD", mUseEGD->isChecked());
  config->writeEntry("UseEFile", mUseEFile->isChecked());
  config->writePathEntry("EGDPath", mEGDPath->url());

#if 0  // NOT IMPLEMENTED IN KDE 2.0
  config->writeEntry("OnMixed", mWarnOnMixed->isChecked());

  config->setGroup("Validation");
  config->writeEntry("WarnSelfSigned", mWarnSelfSigned->isChecked());
  config->writeEntry("WarnExpired", mWarnExpired->isChecked());
  config->writeEntry("WarnRevoked", mWarnRevoked->isChecked());
#endif

#ifdef HAVE_SSL
  config->setGroup("OpenSSL");
  config->writePathEntry("Path", oPath->url());
#endif

  int ciphercount = 0;
  config->setGroup("SSLv3");
  CipherItem *item = static_cast<CipherItem *>(SSLv3Box->firstChild());
  while ( item ) {
    if (item->isOn()) {
      config->writeEntry(item->configName(), true);
      ciphercount++;
    } else config->writeEntry(item->configName(), false);

    item = static_cast<CipherItem *>(item->nextSibling());
  }

  KSSLCertificateCache _cc;

  if (ciphercount == 0)
    KMessageBox::information(this, i18n("If you do not select at least one"
                                       " cipher, SSL will not work."),
                                   i18n("SSL Ciphers"));
  // SSL Policies code
  for (OtherCertItem *x = otherCertDelList.first(); x != 0; x = otherCertDelList.next()) {
     KSSLX509Map cert(x->configName());
     QString thisCN = cert.getValue("CN");
     _cc.removeByCN(thisCN);
     otherCertDelList.remove(x);
  }
  // Go through the non-deleted ones and save them
  for (OtherCertItem *x =
        static_cast<OtherCertItem *>(otherSSLBox->firstChild());
                                                              x;
             x = static_cast<OtherCertItem *>(x->nextSibling())) {
     KSSLX509Map cert(x->configName());
     QString thisCN = cert.getValue("CN");
     QDateTime expires = x->getExpires();
     _cc.modifyByCN(thisCN, (KSSLCertificateCache::KSSLCertificatePolicy)x->getPolicy(), x->isPermanent(), expires);
  }

  // SSL Personal certificates code
  for (YourCertItem *x = yourCertDelList.first(); x != 0; x = yourCertDelList.next()) {
     pcerts->deleteGroup(x->configName());
     yourCertDelList.remove(x);
  }
  // Go through the non-deleted ones and save them
  for (YourCertItem *x =
        static_cast<YourCertItem *>(yourSSLBox->firstChild());
                                                            x;
             x = static_cast<YourCertItem *>(x->nextSibling())) {
     pcerts->setGroup(x->configName());
     pcerts->writeEntry("PKCS12Base64", x->getPKCS());
     pcerts->writeEntry("Password", x->getPass());
  }

  bool doGen = false;

  // CA certificates code
  for (CAItem *x = caDelList.first(); x != 0; x = caDelList.next()) {
     _signers->remove(x->configName());
     caDelList.remove(x);
     doGen = true;
  }

  // Go through the non-deleted ones and save them
  for (CAItem *x = static_cast<CAItem *>(caList->firstChild()); x;
               x = static_cast<CAItem *>(x->nextSibling())) {
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
  }

  if (doGen) genCAList();


  config->setGroup("Auth");
  QString whichAuth = config->readEntry("AuthMethod", "none");
  if (defCertBG->selected() == defSend)
    config->writeEntry("AuthMethod", "send");
  else if (defCertBG->selected() == defPrompt)
    config->writeEntry("AuthMethod", "prompt");
  else
    config->writeEntry("AuthMethod", "none");

  if (defCertBox->currentItem() == 0)
     config->writeEntry("DefaultCert", QString());
  else config->writeEntry("DefaultCert", defCertBox->currentText());

  for (HostAuthItem *x = authDelList.first(); x != 0; x = authDelList.next()) {
     authcfg->deleteGroup(x->configName());
     authDelList.remove(x);
  }

  for (HostAuthItem *x =
        static_cast<HostAuthItem *>(hostAuthList->firstChild());
                                                              x;
             x = static_cast<HostAuthItem *>(x->nextSibling())) {
     if (!x->originalName().isNull())
        authcfg->deleteGroup(x->originalName());
  }

  for (HostAuthItem *x =
        static_cast<HostAuthItem *>(hostAuthList->firstChild());
                                                              x;
             x = static_cast<HostAuthItem *>(x->nextSibling())) {
     authcfg->setGroup(KResolver::domainToAscii(x->configName()));
     authcfg->writeEntry("certificate", x->getCertName());
     authcfg->writeEntry("prompt", (x->getAction() == KSSLCertificateHome::AuthPrompt));
     authcfg->writeEntry("send", (x->getAction() == KSSLCertificateHome::AuthSend));
  }

#endif

  config->sync();
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

#ifdef HAVE_SSL
    // We don't want to make
    // ciphers < 56 bit a default selection.  This is very unsafe and
    // I have already witnessed OpenSSL negotiate a 0 bit connection
    // on me after tracing the https ioslave on a suspicion.

  CipherItem *item;
  for ( item = static_cast<CipherItem *>(SSLv3Box->firstChild()); item;
	item = static_cast<CipherItem *>(item->nextSibling()) ) {
    item->setOn( item->bits() >= 56 );
  }
  mUseEGD->setChecked(false);
  mUseEFile->setChecked(false);
  mEGDLabel->setEnabled(false);
  mEGDPath->setEnabled(false);
  mEGDPath->setURL(QString());
  oPath->setURL(QString());

  defCertBG->setButton(defCertBG->id(defDont));
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
  #ifdef HAVE_SSL
  CipherItem *item;
  for ( item = static_cast<CipherItem *>(SSLv3Box->firstChild()); item;
	item = static_cast<CipherItem *>(item->nextSibling()) ) {
    item->setOn( item->bits() >= 128 );
  }
  configChanged();
  #endif
}


void KCryptoConfig::cwExp() {
  #ifdef HAVE_SSL
  CipherItem *item;
  for ( item = static_cast<CipherItem *>(SSLv3Box->firstChild()); item;
	item = static_cast<CipherItem *>(item->nextSibling()) ) {
    item->setOn( item->bits() <= 56 && item->bits() > 0);
  }
  configChanged();
  #endif
}


void KCryptoConfig::cwAll() {
  #ifdef HAVE_SSL
  CipherItem *item;
  for ( item = static_cast<CipherItem *>(SSLv3Box->firstChild()); item;
	item = static_cast<CipherItem *>(item->nextSibling()) ) {
    item->setOn( true );
  }
  configChanged();
  #endif
}



void KCryptoConfig::slotExportCert() {
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->selectedItem());
   if (x) {
     policies->setGroup(x->getMD5());
     KSSLCertificate *cert = KSSLCertificate::fromString(policies->readEntry("Certificate", QString()).toLocal8Bit());
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
Q3ListViewItem *act = otherSSLBox->selectedItem();
OtherCertItem *x = static_cast<OtherCertItem *>(act);
   if (x) {
      Q3ListViewItem *next = act->itemBelow();
      if (!next) next = act->itemAbove();
      otherSSLBox->takeItem(x);
      otherCertDelList.append(x);
      configChanged();
      if (next) otherSSLBox->setSelected(next, true);
   }
}



void KCryptoConfig::slotVerifyCert() {
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->selectedItem());
  if (!x) return;

  policies->setGroup(x->getMD5());
  KSSLCertificate *cert = KSSLCertificate::fromString(policies->readEntry("Certificate", QString()).toLocal8Bit());

  if (!cert) {
    KMessageBox::error(this, i18n("Error obtaining the certificate."), i18n("SSL"));
    return;
  }

  cert->chain().setCertChain(policies->readEntry("Chain", QStringList()));

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
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->selectedItem());

   cachePerm->setChecked(false);
   untilDate->setEnabled(true);

   if (!x) return;
   x->setPermanent(false);

   configChanged();
}



void KCryptoConfig::slotPermanent() {
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->selectedItem());

   cacheUntil->setChecked(false);
   untilDate->setEnabled(false);

   if (!x) return;
   x->setPermanent(true);

   configChanged();
}



void KCryptoConfig::slotPolicyChanged(int id) {
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->selectedItem());
if (!x) return;

   if (id == policyGroup->id(policyAccept)) {
     x->setPolicy(KSSLCertificateCache::Accept);
   } else if (id == policyGroup->id(policyReject)) {
     x->setPolicy(KSSLCertificateCache::Reject);
   } else if (id == policyGroup->id(policyPrompt)) {
     x->setPolicy(KSSLCertificateCache::Prompt);
   }

   configChanged();
}



void KCryptoConfig::slotDatePick() {
KDateTimeDlg kdtd;
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->selectedItem());

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
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->selectedItem());
QString iss = QString();
   if (x) {
      otherSSLExport->setEnabled(true);
      otherSSLVerify->setEnabled(true);
      otherSSLRemove->setEnabled(true);

      fromLabel->setEnabled(true);
      untilLabel->setEnabled(true);
      policyGroup->setEnabled(true);
      cacheGroup->setEnabled(true);
      cachePerm->setEnabled(true);
      cacheUntil->setEnabled(true);
      policies->setGroup(x->getMD5());

      KSSLCertificate *cert = KSSLCertificate::fromString(policies->readEntry("Certificate", QString()).toLocal8Bit());

      if (cert) {
         QPalette cspl;
         iss = cert->getIssuer();
         cspl = validFrom->palette();
         if (QDateTime::currentDateTime(Qt::UTC) < cert->getQDTNotBefore()) {
            cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
         } else {
            cspl.setColor(QColorGroup::Foreground, QColor(42,153,59));
         }
         validFrom->setPalette(cspl);

         cspl = validUntil->palette();
         if (QDateTime::currentDateTime(Qt::UTC) > cert->getQDTNotAfter()) {
            cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
         } else {
            cspl.setColor(QColorGroup::Foreground, QColor(42,153,59));
         }
         validUntil->setPalette(cspl);

         validFrom->setText(cert->getNotBefore());
         validUntil->setText(cert->getNotAfter());
         untilDate->setText(x ? KGlobal::locale()->formatDateTime(x->getExpires())
                              : KGlobal::locale()->formatDateTime(QDateTime::currentDateTime(Qt::UTC)));
         untilDate->setEnabled(x && !x->isPermanent());
         pHash->setText(cert->getMD5DigestText());
         delete cert;
      } else {
         validFrom->setText(QString());
         validUntil->setText(QString());
         pHash->clear();
      }

      switch(x->getPolicy()) {
      case KSSLCertificateCache::Accept:
        policyGroup->setButton(policyGroup->id(policyAccept));
      break;
      case KSSLCertificateCache::Reject:
        policyGroup->setButton(policyGroup->id(policyReject));
      break;
      case KSSLCertificateCache::Prompt:
        policyGroup->setButton(policyGroup->id(policyPrompt));
      break;
      }

      cachePerm->setChecked(x->isPermanent());
      cacheUntil->setChecked(!x->isPermanent());

   } else {
      otherSSLExport->setEnabled(false);
      otherSSLVerify->setEnabled(false);
      otherSSLRemove->setEnabled(false);
      fromLabel->setEnabled(false);
      untilLabel->setEnabled(false);
      policyGroup->setEnabled(false);
      cacheGroup->setEnabled(false);
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
   }


   oSubject->setValues(x ? x->getSub() : QString(QString()));
   oIssuer->setValues(iss);

}


void KCryptoConfig::slotYourImport() {

   QString certFile = KFileDialog::getOpenFileName(QString(), "application/x-pkcs12");
   if (certFile.isEmpty())
      return;

#ifdef HAVE_SSL
KSSLPKCS12 *cert = NULL;
QByteArray pass;

TryImportPassAgain:
   int rc = KPasswordDialog::getPassword(this,pass, i18n("Certificate password"));
   if (rc != KPasswordDialog::Accepted) return;

   cert = KSSLPKCS12::loadCertFile(certFile, QString(pass));

   if (!cert) {
      rc = KMessageBox::warningYesNo(this, i18n("The certificate file could not be loaded. Try a different password?"), i18n("SSL"),i18n("Try"),i18n("Do Not Try"));
      if (rc == KMessageBox::Yes) goto TryImportPassAgain;
      return;
   }

   // At this point, we know that we can read the certificate in.
   // The procedure will be to convert it to Base64 in it's raw form
   // and add it to the ListView - eventually going into the SimpleConfig.

   // FIXME: prompt if the user wants the password stored along with the
   //        certificate

   QString name = cert->getCertificate()->getSubject();
   for (YourCertItem *i =
        static_cast<YourCertItem *>(yourSSLBox->firstChild());
                                                            i;
             i = static_cast<YourCertItem *>(i->nextSibling())) {
      if (i->configName() == name) {
         rc = KMessageBox::warningContinueCancel(this, i18n("A certificate with that name already exists. Are you sure that you wish to replace it?"), i18n("SSL"), i18n("Replace"));
         if (rc == KMessageBox::Cancel) {
            delete cert;
            return;
         }
         yourSSLBox->takeItem(i);
         yourCertDelList.append(i);
      }
   }

   new YourCertItem(yourSSLBox,
                    cert->toString(),
                    QString(),  // the password - don't store it yet!
                    name,
                    this );

   setAuthCertLists();
   configChanged();
   delete cert;
#endif

   offerImportToKMail( certFile );
}


void KCryptoConfig::slotYourExport() {
YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->selectedItem());
   if (!x) return;

   KSSLPKCS12 *pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPass());
   if (!pkcs)
      pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPassCache());
   if (!pkcs) {
      QString pprompt = i18n("Enter the certificate password:");
      QByteArray oldpass;
      do {
         int i = KPasswordDialog::getPassword(this,oldpass, pprompt);
         if (i != KPasswordDialog::Accepted) return;
         pkcs = KSSLPKCS12::fromString(x->getPKCS(), oldpass);
         pprompt = i18n("Decoding failed. Please try again:");
      } while (!pkcs);
      x->setPassCache(oldpass);
      slotYourUnlock();
   }

  // For now, we will only export to PKCS#12
   QString certFile = KFileDialog::getSaveFileName(QString(),
		                                   "application/x-pkcs12");
   if (certFile.isEmpty())
      return;

   if (!pkcs->toFile(certFile))
      KMessageBox::sorry(this, i18n("Export failed."), i18n("SSL"));
}


void KCryptoConfig::slotYourVerify() {
YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->selectedItem());
QString iss;
   if (!x) return;

   KSSLPKCS12 *pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPass());
   if (!pkcs)
      pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPassCache());
   if (!pkcs) {
      QString pprompt = i18n("Enter the certificate password:");
      QByteArray oldpass;
      do {
         int i = KPasswordDialog::getPassword(this,oldpass, pprompt);
         if (i != KPasswordDialog::Accepted) return;
         pkcs = KSSLPKCS12::fromString(x->getPKCS(), oldpass);
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
YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->selectedItem());
   if (x) {
      yourSSLBox->takeItem(x);
      yourCertDelList.append(x);
      setAuthCertLists();
      configChanged();
   }
}


void KCryptoConfig::slotYourUnlock() {
YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->selectedItem());
QString iss;
   if (!x || !yourSSLUnlock->isEnabled()) return;

   KSSLPKCS12 *pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPass());
   if (!pkcs)
      pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPassCache());
   if (!pkcs) {
      QString pprompt = i18n("Enter the certificate password:");
      QByteArray oldpass;
      do {
         int i = KPasswordDialog::getPassword(this,oldpass, pprompt);
         if (i != KPasswordDialog::Accepted) return;
         pkcs = KSSLPKCS12::fromString(x->getPKCS(), oldpass);
         pprompt = i18n("Decoding failed. Please try again:");
      } while (!pkcs);
      x->setPassCache(oldpass);
   }

   // update the info
   iss = pkcs->getCertificate()->getIssuer();
   ySubject->setValues(x ? x->getName() : QString(QString()));
   yIssuer->setValues(iss);
   QPalette cspl;
   KSSLCertificate *cert = pkcs->getCertificate();
   cspl = yValidFrom->palette();
   if (QDateTime::currentDateTime(Qt::UTC) < cert->getQDTNotBefore()) {
	   cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
   } else {
	   cspl.setColor(QColorGroup::Foreground, QColor(42,153,59));
   }
   yValidFrom->setPalette(cspl);

   cspl = yValidUntil->palette();
   if (QDateTime::currentDateTime(Qt::UTC) > cert->getQDTNotAfter()) {
	   cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
   } else {
	   cspl.setColor(QColorGroup::Foreground, QColor(42,153,59));
   }
   yValidUntil->setPalette(cspl);

   yValidFrom->setText(cert->getNotBefore());
   yValidUntil->setText(cert->getNotAfter());
   yHash->setText(cert->getMD5DigestText());
   yourSSLUnlock->setEnabled(false);
   delete pkcs;
}


void KCryptoConfig::slotYourCertSelect() {
YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->selectedItem());
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
         if (QDateTime::currentDateTime(Qt::UTC) < cert->getQDTNotBefore()) {
            cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
         } else {
            cspl.setColor(QColorGroup::Foreground, QColor(42,153,59));
         }
         yValidFrom->setPalette(cspl);

         cspl = yValidUntil->palette();
         if (QDateTime::currentDateTime(Qt::UTC) > cert->getQDTNotAfter()) {
            cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
         } else {
            cspl.setColor(QColorGroup::Foreground, QColor(42,153,59));
         }
         yValidUntil->setPalette(cspl);

         yValidFrom->setText(cert->getNotBefore());
         yValidUntil->setText(cert->getNotAfter());
         yHash->setText(cert->getMD5DigestText());
         delete pkcs;
      } else {
         yourSSLUnlock->setEnabled(x != NULL);
         yHash->clear();
      }
   } else {
      yHash->clear();
   }

   ySubject->setValues(x ? x->getName() : QString(QString()));
   yIssuer->setValues(iss);
}


void KCryptoConfig::slotYourPass() {
YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->selectedItem());
QByteArray oldpass = "";
   if (!x) return;

   KSSLPKCS12 *pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPass());
   if (!pkcs)
      pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPassCache());
   if (!pkcs) {
      QString pprompt = i18n("Enter the OLD password for the certificate:");
      do {
         int i = KPasswordDialog::getPassword(this,oldpass, pprompt);
         if (i != KPasswordDialog::Accepted) break;
         pkcs = KSSLPKCS12::fromString(x->getPKCS(), oldpass);
         pprompt = i18n("Decoding failed. Please try again:");
      } while (!pkcs);
   }

   if (pkcs) {
      x->setPassCache(oldpass);
      slotYourUnlock();
      KPasswordDialog *kpd = new KPasswordDialog(KPasswordDialog::NewPassword, false, 0, this);
      kpd->setPrompt(i18n("Enter the new certificate password"));
      kpd->setAllowEmptyPasswords(true);

      int i = kpd->exec();
      if (i == KPasswordDialog::Accepted) {
         QByteArray pass = kpd->password();
         pkcs->changePassword(QString(oldpass), QString(pass));
         x->setPKCS(pkcs->toString());
         x->setPassCache(pass);
         configChanged();
      }
      delete kpd;
      delete pkcs;
   }
}


void KCryptoConfig::slotCAImport() {
    QString certFile = KFileDialog::getOpenFileName(QString(), "application/x-x509-ca-cert");

    if (certFile.isEmpty())
        return;

#ifdef HAVE_SSL
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
			for (CAItem *m = static_cast<CAItem *>(caList->firstChild());
                                                                   m;
        	                 m = static_cast<CAItem *>(m->nextSibling())) {
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
			certtext = certtext.replace("-----BEGIN CERTIFICATE-----", QString());
			certtext = certtext.replace("-----END CERTIFICATE-----", QString());
			certtext = certtext.trimmed();
			certtext = certtext.replace("\n", QString());
		} else {
			// Must [could?] be DER
			qf.close();
			qf.open(QIODevice::ReadOnly);
			char *cr;
			cr = new char[qf.size()+1];
			qf.readBlock(cr, qf.size());
			QByteArray qba;
			qba.duplicate(cr, qf.size());
			certtext = KCodecs::base64Encode(qba);
			delete cr;
		}

		qf.close();

		x = KSSLCertificate::fromString(certtext.latin1());

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
			return;
		}

		name = x->getSubject();

		for (CAItem *i = static_cast<CAItem *>(caList->firstChild());
                                                                   i;
                         i = static_cast<CAItem *>(i->nextSibling())) {
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
    if ( KMessageBox::questionYesNo( this, i18n( "Do you want to make this certificate available to KMail as well?" ), QString(), i18n("Make Available"), i18n("Do Not Make Available") ) == KMessageBox::Yes ) {
       KProcess proc;
       proc << "kleopatra";
       proc << "--import-certificate";
       proc << certFile;
       if ( !proc.start( KProcess::DontCare ) )
           KMessageBox::error( this, i18n( "Could not execute Kleopatra. You might have to install or update the kdepim package." ) );
   }
}


void KCryptoConfig::slotCARemove() {
CAItem *x = static_cast<CAItem *>(caList->selectedItem());
 if (x) {
    caList->takeItem(x);
    caDelList.append(x);
    configChanged();
    slotCAItemChanged();
 }
}


void KCryptoConfig::slotCARestore() {

 int rc = KMessageBox::warningContinueCancel(this, i18n("This will revert your certificate signers database to the KDE default.\nThis operation cannot be undone.\nAre you sure you wish to continue?"), i18n("SSL"),i18n("Revert"));
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
  KConfig sigcfg("ksslcalist", true, false);

  for (QStringList::Iterator i = groups.begin();
                             i != groups.end();
                             ++i) {
    if ((*i).isEmpty() || *i == "<default>") continue;
    if (!sigcfg.hasGroup(*i)) continue;
    sigcfg.setGroup(*i);
    if (!sigcfg.hasKey("x509")) continue;
                new CAItem(caList,
                     (*i),
                     sigcfg.readEntry("x509", QString()),
                     sigcfg.readEntry("site", false),
                     sigcfg.readEntry("email", false),
                     sigcfg.readEntry("code", false),
                     this );
  }

   genCAList();
   slotCAItemChanged();
}


void KCryptoConfig::slotCAItemChanged() {
CAItem *x = static_cast<CAItem *>(caList->selectedItem());
 if (x) {
    caSSLRemove->setEnabled(true);
    caSubject->setValues(x ? x->getName() : QString(QString()));
    KSSLCertificate *cert = KSSLCertificate::fromString(x->getCert().toLocal8Bit());
    if (!cert) {
       caIssuer->setValues(QString(QString()));
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
       caIssuer->setValues(cert->getIssuer());
       cHash->setText(cert->getMD5DigestText());
       delete cert;
    }
 } else {
    caSSLRemove->setEnabled(false);
    caSite->setEnabled(false);
    caEmail->setEnabled(false);
    caCode->setEnabled(false);
    caSubject->setValues(QString(QString()));
    caIssuer->setValues(QString(QString()));
    cHash->clear();
 }
}


void KCryptoConfig::slotCAChecked() {
CAItem *x = static_cast<CAItem *>(caList->selectedItem());
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
    hostAuthList->setSelected(j, true);
  authHost->setEnabled(true);
  hostCertBox->setEnabled(true);
  hostCertBG->setEnabled(true);
  authRemove->setEnabled(true);

  hostCertBG->setButton(0);

  authHost->setFocus();
}



void KCryptoConfig::slotRemoveHostAuth() {
Q3ListViewItem *act = hostAuthList->selectedItem();
HostAuthItem *x = static_cast<HostAuthItem *>(act);

  if (x) {
      Q3ListViewItem *next = act->itemBelow();
      if (!next) next = act->itemAbove();
      hostAuthList->takeItem(x);
      authDelList.append(x);
      configChanged();
      if (next) hostAuthList->setSelected(next, true);
  }
}


void KCryptoConfig::slotAuthItemChanged() {
HostAuthItem *x = static_cast<HostAuthItem *>(hostAuthList->selectedItem());

if (x) {
  // Make sure the fields are enabled
  authHost->setEnabled(true);
  hostCertBox->setEnabled(true);
  hostCertBG->setEnabled(true);
  authRemove->setEnabled(true);

  // Populate the fields
  switch(x->getAction()) {
  case KSSLCertificateHome::AuthSend:
    hostCertBG->setButton(hostCertBG->id(hostSend));
   break;
  case KSSLCertificateHome::AuthPrompt:
    hostCertBG->setButton(hostCertBG->id(hostPrompt));
   break;
  case KSSLCertificateHome::AuthDont:
    hostCertBG->setButton(hostCertBG->id(hostDont));
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
  hostCertBox->setCurrentItem(0);

  QString theCert = x->getCertName();
  for (int i = 0; i < hostCertBox->count(); i++) {
    if (hostCertBox->text(i) == theCert) {
       hostCertBox->setCurrentItem(i);
       break;
    }
  }
} else {
  authHost->clear();
  authHost->setEnabled(false);
  hostCertBox->setEnabled(false);
  hostCertBG->setEnabled(false);
  authRemove->setEnabled(false);
}
}


void KCryptoConfig::slotAuthText(const QString &t) {
if (___lehack) return;
HostAuthItem *x = static_cast<HostAuthItem *>(hostAuthList->selectedItem());

if (x) {
   x->setHost(t);
configChanged();
}
}



void KCryptoConfig::slotAuthButtons() {
HostAuthItem *x = static_cast<HostAuthItem *>(hostAuthList->selectedItem());

if (x) {
   KSSLCertificateHome::KSSLAuthAction aa = KSSLCertificateHome::AuthDont;
   int sel = hostCertBG->id(hostCertBG->selected());

   if (sel ==  hostCertBG->id(hostSend))
    aa = KSSLCertificateHome::AuthSend;
   else if (sel == hostCertBG->id(hostPrompt))
    aa = KSSLCertificateHome::AuthPrompt;
   else
    aa =  KSSLCertificateHome::AuthDont;

   x->setAction(aa);
configChanged();
}
}


void KCryptoConfig::slotAuthCombo() {
HostAuthItem *x = static_cast<HostAuthItem *>(hostAuthList->selectedItem());

  if (x) {
    if (hostCertBox->currentItem() == 0)
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
               << i18n("Server certificate authority", "Server CA")
               << i18n("Personal certificate authority", "Personal CA")
               << i18n("Secure MIME certificate authority", "S/MIME CA");
#endif

}


#ifdef HAVE_SSL
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
    if (QString(sc->name).contains("ADH-")) {
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
  noneDef = (0 == defCertBox->currentItem());
  noneHost = (0 == hostCertBox->currentItem());

// repopulate
  defCertBox->clear();
  hostCertBox->clear();
  QStringList defCertStrList; // = KSSLCertificateHome::getCertificateList();
  defCertStrList.append(i18n("None"));
  for (YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->firstChild());
                                                                             x;
                           x = static_cast<YourCertItem *>(x->nextSibling())) {
     defCertStrList.append(x->configName());
  }
  defCertBox->insertStringList(defCertStrList);
  hostCertBox->insertStringList(defCertStrList);

// try to restore it

  defCertBox->setCurrentItem(0);
  if (!noneDef) {
    for (int i = 0; i < defCertBox->count(); i++) {
      if (defCertBox->text(i) == oldDef) {
         defCertBox->setCurrentItem(i);
         break;
      }
    }
  }

  if (!noneDef && defCertBox->currentItem() == 0) configChanged();

  hostCertBox->setCurrentItem(0);
  if (!noneHost) {
    for (int i = 0; i < hostCertBox->count(); i++) {
      if (hostCertBox->text(i) == oldHost) {
         hostCertBox->setCurrentItem(i);
         break;
      }
    }
  }

  if (!noneHost && hostCertBox->currentItem() == 0) configChanged();

  // Update the host entries too
  for (HostAuthItem *x =
        static_cast<HostAuthItem *>(hostAuthList->firstChild());
                                                              x;
             x = static_cast<HostAuthItem *>(x->nextSibling())) {
     QString newValue = QString();
     for (int i = 1; i < hostCertBox->count(); i++) {
        if (hostCertBox->text(i) == x->getCertName()) {
           newValue = x->getCertName();
           break;
        }
     }
     if (newValue != x->getCertName()) configChanged();
     x->setCertName(newValue);
  }


}

#include "crypto.moc"
