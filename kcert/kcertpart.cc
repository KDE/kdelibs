/* This file is part of the KDE project
 *
 * Copyright (C) 2001,2002 George Staikos <staikos@kde.org>
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


#include "kcertpart.h"
#include <kparts/genericfactory.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include <qframe.h>
#include <klocale.h>
#include <kdebug.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qbutton.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <kmessagebox.h>
#include <kpassdlg.h>
#include <ksslall.h>
#include <kopenssl.h>
#include <ksslpemcallback.h>
#include <kfiledialog.h>
#include <kprocess.h>
#include <qtabwidget.h>
#include <kseparator.h>
#include <klistview.h>
#include <kio/kmimemagic.h>
#include <qmultilineedit.h>
#include <qregexp.h>
#include <kcombobox.h>
#include <kparts/browserextension.h>
#include <kparts/browserinterface.h>
#include <kio/kservicetypefactory.h>

K_EXPORT_COMPONENT_FACTORY( libkcertpart, KParts::GenericFactory<KCertPart> )


KX509Item::KX509Item(KListViewItem *parent, KSSLCertificate *x) :
			KListViewItem(parent, 0L)
{
	setup(x);
}

KX509Item::KX509Item(KListView *parent, KSSLCertificate *x) :
			KListViewItem(parent)
{
	setup(x);
}

void KX509Item::setup(KSSLCertificate *x) {
	cert = x;
	if (x) {
		KSSLX509Map xm(x->getSubject());
		QString OU = "OU";
		QString CN = "CN";
		OU = xm.getValue(OU);
		CN = xm.getValue(CN);
		OU.replace(QRegExp("\n.*"), "");
		CN.replace(QRegExp("\n.*"), "");

		if (OU.length() > 0)
		  _prettyName = OU;
		if (CN.length() > 0) {
		  if (_prettyName.length() > 0)
			  _prettyName += " - ";
		  _prettyName += CN;
		}
		setText(0, _prettyName);
	} else {
		setText(0, i18n("Invalid certificate!"));
	}
}


KX509Item::~KX509Item() 
{
	if (cert)
		delete cert;
}


KPKCS12Item::KPKCS12Item(KListViewItem *parent, KSSLPKCS12 *x) :
			KListViewItem(parent, 0L)
{
	cert = x;
	if (x) {
		KSSLX509Map xm(x->getCertificate()->getSubject());
		QString CN = "CN";
		CN = xm.getValue(CN);
		CN.replace(QRegExp("\n.*"), "");
		_prettyName = CN;
		setText(0, _prettyName);
	} else {
		setText(0, i18n("Invalid certificate!"));
	}
}


KPKCS12Item::~KPKCS12Item() 
{
	if (cert)
		delete cert;
}


class KCertPartPrivate {
	public:
		KParts::BrowserExtension *browserExtension;
};


KCertPart::KCertPart(QWidget *parentWidget, const char *widgetName,
                     QObject *parent, const char *name,
		     const QStringList & /*args*/ ) 
          : KParts::ReadWritePart(parent, name) {
 KInstance *instance = new KInstance("KCertPart");
 QGridLayout *grid;
 setInstance(instance);


 _signers = new KSSLSigners;
 // This is a bit confusing now.  Here's how it works:
 //    We create a _frame and split it left/right
 //    Then we add the ListView to the left and create
 //    a new frame on the right.  We set the main widget
 //    on the right.

 _p12 = NULL;
 _ca = NULL;
 _silentImport = false;
 d = new KCertPartPrivate;
 d->browserExtension = new KParts::BrowserExtension(this);

 _frame = new QFrame(parentWidget, widgetName);
 setWidget(_frame);

 _baseGrid = new QGridLayout(_frame, 15, 9, KDialog::marginHint(),
		 			   KDialog::spacingHint());

 _sideList = new KListView(_frame);
 _sideList->setRootIsDecorated(true);
 _sideList->addColumn(i18n("Certificates"));
 _parentCA = new KListViewItem(_sideList, i18n("Signers"));
 _parentCA->setExpandable(true);
 _sideList->setOpen(_parentCA, true);
 _parentP12 = new KListViewItem(_sideList, i18n("Client"));
 _parentP12->setExpandable(true);
 _sideList->setOpen(_parentP12, true);

 _baseGrid->addMultiCellWidget(_sideList, 0, 13, 0, 1);

 _importAll = new QPushButton(i18n("Import &All"), _frame);
 _baseGrid->addMultiCellWidget(_importAll, 14, 14, 0, 1);
 connect(_importAll, SIGNAL(clicked()), SLOT(slotImportAll()));



//------------------------------------------------------------------------
//   The PKCS widget
//------------------------------------------------------------------------
 _pkcsFrame = new QFrame(_frame);

 grid = new QGridLayout(_pkcsFrame, 13, 6, KDialog::marginHint(),
                                       KDialog::spacingHint() );
 grid->addMultiCellWidget(new QLabel(i18n("KDE Secure Certificate Import"), _pkcsFrame), 0, 0, 0, 5);
 grid->addWidget(new QLabel(i18n("Chain:"), _pkcsFrame), 1, 0);
 _p12_chain = new KComboBox(_pkcsFrame);
 grid->addMultiCellWidget(_p12_chain, 1, 1, 1, 4);
 connect(_p12_chain, SIGNAL(activated(int)), SLOT(slotChain(int)));

 grid->addWidget(new QLabel(i18n("Subject:"), _pkcsFrame), 2, 0);
 grid->addWidget(new QLabel(i18n("Issued by:"), _pkcsFrame), 2, 3);
 _p12_subject = KSSLInfoDlg::certInfoWidget(_pkcsFrame, QString(""));
 _p12_issuer = KSSLInfoDlg::certInfoWidget(_pkcsFrame, QString(""));
 grid->addMultiCellWidget(_p12_subject, 3, 6, 0, 2);
 grid->addMultiCellWidget(_p12_issuer, 3, 6, 3, 5);

 grid->addWidget(new QLabel(i18n("File:"), _pkcsFrame), 7, 0);
 _p12_filenameLabel = new QLabel("", _pkcsFrame);
 grid->addWidget(_p12_filenameLabel, 7, 1);
 grid->addWidget(new QLabel(i18n("File format:"), _pkcsFrame), 7, 3);
 grid->addWidget(new QLabel("PKCS#12", _pkcsFrame), 7, 4);


 //
 //  Make the first tab
 //
 _tabs = new QTabWidget(_pkcsFrame);
 grid->addMultiCellWidget(_tabs, 8, 12, 0, 5);

 QFrame *tab = new QFrame(_pkcsFrame);
 QGridLayout *tabGrid = new QGridLayout(tab, 4, 5, KDialog::marginHint(),
                                                   KDialog::spacingHint() );
 tabGrid->addWidget(new QLabel(i18n("State:"), tab), 0, 0);
 _p12_certState = new QLabel("", tab);
 tabGrid->addMultiCellWidget(_p12_certState, 0, 0, 1, 4);

 tabGrid->addWidget(new QLabel(i18n("Valid from:"), tab), 1, 0);
 _p12_validFrom = new QLabel("", tab);
 tabGrid->addMultiCellWidget(_p12_validFrom, 1, 1, 1, 4);

 tabGrid->addWidget(new QLabel(i18n("Valid until:"), tab), 2, 0);
 _p12_validUntil = new QLabel("", tab);
 tabGrid->addMultiCellWidget(_p12_validUntil, 2, 2, 1, 4);

 tabGrid->addWidget(new QLabel(i18n("Serial number:"), tab), 3, 0);
 _p12_serialNum = new QLabel("", tab);
 tabGrid->addWidget(_p12_serialNum, 3, 1);
 _tabs->addTab(tab, i18n("State"));


 //
 // Make the second tab
 //
 tab = new QFrame(_pkcsFrame);
 tabGrid = new QGridLayout(tab, 4, 5, KDialog::marginHint(),
                                      KDialog::spacingHint() );
 tabGrid->addWidget(new QLabel(i18n("MD5 digest:"), tab), 0, 0);
 _p12_digest = new QLabel(tab);
 tabGrid->addMultiCellWidget(_p12_digest, 0, 0, 1, 4);
 tabGrid->addWidget(new QLabel(i18n("Signature:"), tab), 1, 0);
 _p12_sig = new QMultiLineEdit(tab);
 tabGrid->addMultiCellWidget(_p12_sig, 1, 3, 1, 4);
 _p12_sig->setReadOnly(true);

 _tabs->addTab(tab, i18n("Signature"));


 //
 // Make the third tab
 //
 tab = new QFrame(_pkcsFrame);
 tabGrid = new QGridLayout(tab, 4, 5, KDialog::marginHint(),
                                      KDialog::spacingHint() );
 tabGrid->addWidget(new QLabel(i18n("Public key:"), tab), 0, 0);
 _p12_pubkey = new QMultiLineEdit(tab);
 tabGrid->addMultiCellWidget(_p12_pubkey, 0, 3, 1, 4);
 _p12_pubkey->setReadOnly(true);
 

 _tabs->addTab(tab, i18n("Public Key"));

 _pkcsFrame->hide();

//------------------------------------------------------------------------
//   The X509 widget
//------------------------------------------------------------------------
//   Note: this is almost identical to the above, but I duplicate it for
//   the simple reason that the above has potential to display much
//   more information, and this one has potential to display different
//   information.
 _x509Frame = new QFrame(_frame);

 grid = new QGridLayout(_x509Frame, 12, 6, KDialog::marginHint(),
                                       KDialog::spacingHint() );
 grid->addMultiCellWidget(new QLabel(i18n("KDE Secure Certificate Import"), _x509Frame), 0, 0, 0, 5);

 grid->addWidget(new QLabel(i18n("Subject:"), _x509Frame), 1, 0);
 grid->addWidget(new QLabel(i18n("Issued by:"), _x509Frame), 1, 3);
 _ca_subject = KSSLInfoDlg::certInfoWidget(_x509Frame, QString(""));
 _ca_issuer = KSSLInfoDlg::certInfoWidget(_x509Frame, QString(""));
 grid->addMultiCellWidget(_ca_subject, 2, 5, 0, 2);
 grid->addMultiCellWidget(_ca_issuer, 2, 5, 3, 5);

 grid->addWidget(new QLabel(i18n("File:"), _x509Frame), 6, 0);
 _ca_filenameLabel = new QLabel("", _x509Frame);
 grid->addWidget(_ca_filenameLabel, 6, 1);
 grid->addWidget(new QLabel(i18n("File format:"), _x509Frame), 6, 3);
 grid->addWidget(new QLabel("PEM or DER Encoded X.509", _x509Frame), 6, 4);


 //
 //  Make the first tab
 //
 _tabs = new QTabWidget(_x509Frame);
 grid->addMultiCellWidget(_tabs, 7, 11, 0, 5);

 tab = new QFrame(_x509Frame);
 tabGrid = new QGridLayout(tab, 4, 5, KDialog::marginHint(),
                                      KDialog::spacingHint() );
 tabGrid->addWidget(new QLabel(i18n("State:"), tab), 0, 0);
 _ca_certState = new QLabel("", tab);
 tabGrid->addMultiCellWidget(_ca_certState, 0, 0, 1, 4);

 tabGrid->addWidget(new QLabel(i18n("Valid from:"), tab), 1, 0);
 _ca_validFrom = new QLabel("", tab);
 tabGrid->addMultiCellWidget(_ca_validFrom, 1, 1, 1, 4);

 tabGrid->addWidget(new QLabel(i18n("Valid until:"), tab), 2, 0);
 _ca_validUntil = new QLabel("", tab);
 tabGrid->addMultiCellWidget(_ca_validUntil, 2, 2, 1, 4);

 tabGrid->addWidget(new QLabel(i18n("Serial number:"), tab), 3, 0);
 _ca_serialNum = new QLabel("", tab);
 tabGrid->addWidget(_ca_serialNum, 3, 1);
 _tabs->addTab(tab, i18n("State"));


 //
 // Make the second tab
 //
 tab = new QFrame(_x509Frame);
 tabGrid = new QGridLayout(tab, 4, 5, KDialog::marginHint(),
                                      KDialog::spacingHint() );
 tabGrid->addWidget(new QLabel(i18n("MD5 digest:"), tab), 0, 0);
 _ca_digest = new QLabel(tab);
 tabGrid->addMultiCellWidget(_ca_digest, 0, 0, 1, 4);
 tabGrid->addWidget(new QLabel(i18n("Signature:"), tab), 1, 0);
 _ca_sig = new QMultiLineEdit(tab);
 tabGrid->addMultiCellWidget(_ca_sig, 1, 3, 1, 4);
 _ca_sig->setReadOnly(true);

 _tabs->addTab(tab, i18n("Signature"));


 //
 // Make the third tab
 //
 tab = new QFrame(_x509Frame);
 tabGrid = new QGridLayout(tab, 4, 5, KDialog::marginHint(),
                                      KDialog::spacingHint() );
 tabGrid->addWidget(new QLabel(i18n("Public key:"), tab), 0, 0);
 _ca_pubkey = new QMultiLineEdit(tab);
 tabGrid->addMultiCellWidget(_ca_pubkey, 0, 3, 1, 4);
 _ca_pubkey->setReadOnly(true);
 

 _tabs->addTab(tab, i18n("Public Key"));

 _x509Frame->hide();




//------------------------------------------------------------------------
//   The blank widget
//------------------------------------------------------------------------
 _blankFrame = new QFrame(_frame);
 grid = new QGridLayout(_blankFrame, 1, 1, KDialog::marginHint(),
                                           KDialog::spacingHint() );
 grid->addMultiCellWidget(new QLabel(i18n("KDE Secure Certificate Import"), _blankFrame), 0, 0, 0, 0);
 _blankFrame->show();



 //
 // Finish it off
 //
 _baseGrid->addMultiCellWidget(new KSeparator(KSeparator::HLine, _frame), 13, 13, 2, 8);
 _launch = new QPushButton(i18n("&Crypto Manager..."), _frame);
 _import = new QPushButton(i18n("&Import"), _frame);
 _save = new QPushButton(i18n("&Save..."), _frame);
 _done = new QPushButton(i18n("&Done"), _frame);
 _baseGrid->addMultiCellWidget(_launch, 14, 14, 4, 5);
 _baseGrid->addWidget(_import, 14, 6);
 _baseGrid->addWidget(_save, 14, 7);
 _baseGrid->addWidget(_done, 14, 8);
 connect(_launch, SIGNAL(clicked()), SLOT(slotLaunch()));
 connect(_import, SIGNAL(clicked()), SLOT(slotImport()));
 connect(_save, SIGNAL(clicked()), SLOT(slotSave()));
 connect(_done, SIGNAL(clicked()), SLOT(slotDone()));
 _import->setEnabled(false);
 _save->setEnabled(false);



 _baseGrid->addMultiCellWidget(_pkcsFrame, 0, 12, 2, 8);
 _baseGrid->addMultiCellWidget(_x509Frame, 0, 12, 2, 8);
 _baseGrid->addMultiCellWidget(_blankFrame, 0, 12, 2, 8);

 connect(_sideList, SIGNAL(selectionChanged(QListViewItem*)),
	 this, SLOT(slotSelectionChanged(QListViewItem*)));
 setReadWrite(true);
}


KCertPart::~KCertPart() {
 delete _signers;
 delete d->browserExtension;
 delete d;
}


void KCertPart::setReadWrite(bool rw) {
  if (rw == false) {
  	_import->setEnabled(false);
  	_save->setEnabled(false);
  }
  KParts::ReadWritePart::setReadWrite(rw);
}


bool KCertPart::saveFile() {
if (_p12) {
  QString certFile = KFileDialog::getSaveFileName(QString::null, "application/x-pkcs12");
  if (certFile.isEmpty())
    return false;
 
  if (!_p12->toFile(certFile)) {
    KMessageBox::sorry(_frame, i18n("Save failed."), i18n("Certificate Import"));
    return false;
  }

	return true;
} else if (_ca) {
  QString certFile = KFileDialog::getSaveFileName(QString::null, "application/x-x509-ca-cert");
  if (certFile.isEmpty())
    return false;

  QByteArray enc;
  if (certFile.endsWith("der") || certFile.endsWith("crt"))
	enc = _ca->toDer();
  else if (certFile.endsWith("netscape"))
	enc = _ca->toNetscape();
  else enc = _ca->toPem();

  QFile of(certFile);

  if (!of.open(IO_WriteOnly) || (unsigned)of.writeBlock(enc) != enc.size()) {
    KMessageBox::sorry(_frame, i18n("Save failed."), i18n("Certificate Import"));
    return false;
  }

  of.flush();

	return true;
} else {
	return false;
}
}


bool KCertPart::openFile() {
#ifndef HAVE_SSL
  KMessageBox::sorry(_frame, i18n("You don't seem to have compiled KDE with SSL support."), i18n("Certificate Import"));
 return false;
#else


  QString whatType = d->browserExtension->urlArgs().serviceType;
                    //whatType = KMimeType::findByURL(m_url,0,true)->name();
  if (whatType == "" || whatType == QString::null)
     whatType = KServiceTypeFactory::self()->findFromPattern(m_file)->name();

  /*
  QString blah = "file: " + m_file
	       + "\nurl: " + m_url.url()
	       + "\nserviceType: " + d->browserExtension->urlArgs().serviceType
	       + "\nfactory: " + KServiceTypeFactory::self()->findFromPattern(m_file)->name()
	       + "\nmimeType: " + KMimeType::findByURL(m_url)->name();
  KMessageBox::information(_frame, blah, "ssl");
  */
  

  emit completed();

/////////////////////////////////////////////////////////////////////////////
//       x-pkcs12 loading
/////////////////////////////////////////////////////////////////////////////
if (whatType == "application/x-pkcs12") {
QCString pass;
  _p12 = KSSLPKCS12::loadCertFile(m_file);

  while (!_p12) {
     // try prompting for a password.
    int rc = KPasswordDialog::getPassword(pass, i18n("Certificate password"));
    if (rc != KPasswordDialog::Accepted) break;

    _p12 = KSSLPKCS12::loadCertFile(m_file, QString(pass));

    if (!_p12) {
      rc = KMessageBox::warningYesNo(_frame, i18n("The certificate file could not be loaded. Try a different password?"), i18n("Certificate Import"));
      if (rc == KMessageBox::Yes) continue;
      break;
    }
  }

  if (!_p12) return false;

  new KPKCS12Item(_parentP12, _p12);
  _p12 = NULL;
 return true;
/////////////////////////////////////////////////////////////////////////////
//       x-509-ca-cert loading
/////////////////////////////////////////////////////////////////////////////
} else if (whatType == "application/x-x509-ca-cert" ||
	   whatType == "application/binary-certificate") {
 FILE *fp; 
 bool isPEM = false;
 
 /////////////  UGLY HACK TO GET AROUND OPENSSL PROBLEMS ///////////
 if (whatType == "application/x-x509-ca-cert") {
    // Check if it is PEM or not
    QFile qf(m_file);
    qf.open(IO_ReadOnly);
    QByteArray theFile = qf.readAll();
    qf.close();

    const char *signature = "-----BEGIN CERTIFICATE-----";
    theFile[qf.size()-1] = 0;
    isPEM = (QCString(theFile.data()).find(signature) >= 0);
 }

 fp = fopen(m_file.local8Bit(), "r");
 if (!fp) {
    KMessageBox::sorry(_frame, i18n("This file cannot be opened."), i18n("Certificate Import"));
    return false;
 }

 /*
 kdDebug() << "Reading in a file in "
	   << (isPEM ? "PEM" : "DER")
	   << " format." << endl;
 */

 if (!isPEM) {
    X509 *dx = KOSSL::self()->X509_d2i_fp(fp, NULL);

    if (dx) {
   	KSSLCertificate *xc = KSSLCertificate::fromX509(dx);
	if (xc) {
		if (xc->x509V3Extensions().certTypeCA())
			new KX509Item(_parentCA, xc);
		else
			new KX509Item(_sideList, xc);
		fclose(fp);
		return true;
	}
	KOSSL::self()->X509_free(dx);
    } 
    return false;
 }

#define sk_free KOSSL::self()->sk_free
#define sk_num KOSSL::self()->sk_num
#define sk_value KOSSL::self()->sk_value
 STACK_OF(X509_INFO) *sx5i = KOSSL::self()->PEM_X509_INFO_read(fp, NULL, KSSLPemCallback, NULL);

 if (!sx5i) {
    KMessageBox::sorry(_frame, i18n("This file cannot be opened."), i18n("Certificate Import"));
    fclose(fp);
    return false;
 }

 _ca_filenameLabel->setText(m_file);
 for (int i = 0; i < sk_X509_INFO_num(sx5i); i++) {
	X509_INFO* x5i = sk_X509_INFO_value(sx5i, i);
	if (x5i->x_pkey && x5i->x509) {   // a personal cert (like PKCS12)
		KSSLCertificate *xc = KSSLCertificate::fromX509(x5i->x509);
		new KX509Item(_sideList, xc);
	} else if (x5i->x509) {   // something else - maybe a CA file
		KSSLCertificate *xc = KSSLCertificate::fromX509(x5i->x509);
		if (xc->x509V3Extensions().certTypeCA())
			new KX509Item(_parentCA, xc);
		else new KX509Item(_sideList, xc);
	} else if (x5i->crl) {   // a crl
		kdDebug() << "Found a CRL..." << endl;
	}
 }

 sk_X509_INFO_free(sx5i);

#undef sk_free
#undef sk_num
#undef sk_value

 fclose(fp);
 return true;
/////////////////////////////////////////////////////////////////////////////
//       Dunno how to load this
/////////////////////////////////////////////////////////////////////////////
} else {
  QString emsg = i18n("I don't know how to handle this type of file.") + "\n" + whatType;
  KMessageBox::sorry(_frame, emsg, i18n("Certificate Import"));
 return false;
}
#endif
}


void KCertPart::displayPKCS12() {
  KSSLCertificate *xc = _p12->getCertificate();
  _p12_filenameLabel->setText(m_file);
  displayPKCS12Cert(xc);
  _p12_certState->setText(KSSLCertificate::verifyText(_p12->validate()));

  // Set the chain if it's there
  if (xc->chain().depth() > 1) {
    QPtrList<KSSLCertificate> cl = xc->chain().getChain();
    int cnt = 0;
    _p12_chain->setEnabled(true);
    _p12_chain->clear();
    _p12_chain->insertItem(i18n("0 - Site Certificate"));
    for (KSSLCertificate *c = cl.first(); c != 0; c = cl.next()) {
       KSSLX509Map map(c->getSubject());
       _p12_chain->insertItem(QString::number(++cnt)+" - "+map.getValue("CN"));
    }
    _p12_chain->setCurrentItem(0);
  } else {
    _p12_chain->clear();
    _p12_chain->setEnabled(false);
  }
}


void KCertPart::displayCACert(KSSLCertificate *c) {
  // We have the file, lets work with it.
  _ca_subject->setValues(c->getSubject());
  _ca_issuer->setValues(c->getIssuer());

  // Set the valid period
         QPalette cspl = _ca_validFrom->palette();
         if (QDateTime::currentDateTime() < c->getQDTNotBefore()) {
            cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
         } else {
            cspl.setColor(QColorGroup::Foreground, QColor(42,153,59));
         }
         _ca_validFrom->setPalette(cspl);
         _ca_validFrom->setText(c->getNotBefore());

         cspl = _ca_validUntil->palette();
         if (QDateTime::currentDateTime() > c->getQDTNotAfter()) {
            cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
         } else {
            cspl.setColor(QColorGroup::Foreground, QColor(42,153,59));
         }
         _ca_validUntil->setPalette(cspl);
         _ca_validUntil->setText(c->getNotAfter());

  _ca_serialNum->setText(c->getSerialNumber());
         cspl = _ca_certState->palette();
         if (!c->isValid()) {
            cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
         } else {
            cspl.setColor(QColorGroup::Foreground, QColor(42,153,59));
         }
         _ca_certState->setPalette(cspl);
  _ca_certState->setText(KSSLCertificate::verifyText(c->validate()));

  _ca_pubkey->setText(c->getPublicKeyText());
  _ca_digest->setText(c->getMD5DigestText());
  _ca_sig->setText(c->getSignatureText());
}



void KCertPart::displayPKCS12Cert(KSSLCertificate *c) {
  // We have the file, lets work with it.
  _p12_subject->setValues(c->getSubject());
  _p12_issuer->setValues(c->getIssuer());

  // Set the valid period
         QPalette cspl = _p12_validFrom->palette();
         if (QDateTime::currentDateTime() < c->getQDTNotBefore()) {
            cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
         } else {
            cspl.setColor(QColorGroup::Foreground, QColor(42,153,59));
         }
         _p12_validFrom->setPalette(cspl);
         _p12_validFrom->setText(c->getNotBefore());

         cspl = _p12_validUntil->palette();
         if (QDateTime::currentDateTime() > c->getQDTNotAfter()) {
            cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
         } else {
            cspl.setColor(QColorGroup::Foreground, QColor(42,153,59));
         }
         _p12_validUntil->setPalette(cspl);
         _p12_validUntil->setText(c->getNotAfter());

  _p12_serialNum->setText(c->getSerialNumber());
         cspl = _p12_certState->palette();
         if (!c->isValid()) {
            cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
         } else {
            cspl.setColor(QColorGroup::Foreground, QColor(42,153,59));
         }
         _p12_certState->setPalette(cspl);
  _p12_certState->setText(KSSLCertificate::verifyText(c->validate()));

  _p12_pubkey->setText(c->getPublicKeyText());
  _p12_digest->setText(c->getMD5DigestText());
  _p12_sig->setText(c->getSignatureText());
}



void KCertPart::slotChain(int c) {

  if (c == 0) {
     displayPKCS12Cert(_p12->getCertificate());
     _p12_certState->setText(KSSLCertificate::verifyText(_p12->validate()));
  } else {
     displayPKCS12Cert(_p12->getCertificate()->chain().getChain().at(c-1));
  }
}


void KCertPart::slotImport() {
if (_p12) {
  KSimpleConfig cfg("ksslcertificates", false);

  if (cfg.hasGroup(_p12->getCertificate()->getSubject())) {
     QString msg = _curName + "\n" + i18n("A certificate with that name already exists. Are you sure that you wish to replace it?");
     int rc= KMessageBox::warningYesNo(_frame, msg, i18n("Certificate Import"));
     if (rc == KMessageBox::No) {
        return;
     }
  }

  cfg.setGroup(_p12->getCertificate()->getSubject());
  cfg.writeEntry("PKCS12Base64", _p12->toString());
  cfg.writeEntry("Password", "");
  cfg.sync();
  if (!_silentImport)
  KMessageBox::information(_frame, i18n("Certificate has been successfully imported into KDE.\nYou can manage your certificate settings from the KDE Control Center."), i18n("Certificate Import"));
} else if (_ca) {
  KConfig cfg("ksslcalist", true, false);
  if (cfg.hasGroup(_ca->getSubject())) {
     QString msg = _curName + "\n" + i18n("A certificate with that name already exists. Are you sure that you wish to replace it?");
     int rc= KMessageBox::warningYesNo(_frame, msg, i18n("Certificate Import"));
     if (rc == KMessageBox::No) {
        return;
     }
  } 
  _signers->addCA(_ca->toString(),
		  _ca->x509V3Extensions().certTypeSSLCA(),
		  _ca->x509V3Extensions().certTypeEmailCA(),
		  _ca->x509V3Extensions().certTypeCodeCA());
  if (!_silentImport)
	  _signers->regenerate();

  if (!_silentImport)
  KMessageBox::information(_frame, i18n("Certificate has been successfully imported into KDE.\nYou can manage your certificate settings from the KDE Control Center."), i18n("Certificate Import"));
}
}


void KCertPart::slotSave() {
  saveFile();
}


void KCertPart::slotDone() {
  KParts::BrowserInterface *iface = d->browserExtension->browserInterface();
  iface->callMethod("goHistory(int)", -1);
}


void KCertPart::slotLaunch() {
KShellProcess p;
  p << "kcmshell" << "crypto";
  p.start(KProcess::DontCare);
}


void KCertPart::slotSelectionChanged(QListViewItem *x) {
  _p12 = NULL;
  _ca = NULL;
  if (x && x->parent() == _parentCA) {
        _blankFrame->hide();
        _pkcsFrame->hide();
	_x509Frame->show();
	_ca = dynamic_cast<KX509Item*>(x)->cert;
	_import->setEnabled(true);
	_save->setEnabled(true);
	_curName = dynamic_cast<KX509Item*>(x)->_prettyName;
	displayCACert(_ca);
  } else if (x && x->parent() == NULL && x->rtti() == 1) {
        _blankFrame->hide();
        _pkcsFrame->hide();
	_x509Frame->show();
	_ca = dynamic_cast<KX509Item*>(x)->cert;
	_import->setEnabled(false);
	_save->setEnabled(false);
	_curName = dynamic_cast<KX509Item*>(x)->_prettyName;
	displayCACert(_ca);
  } else if (x && x->parent() == _parentP12) {
        _blankFrame->hide();
        _x509Frame->hide();
	_pkcsFrame->show();
	_p12 = dynamic_cast<KPKCS12Item*>(x)->cert;
	_import->setEnabled(true);
	_save->setEnabled(true);
	_curName = dynamic_cast<KPKCS12Item*>(x)->_prettyName;
	displayPKCS12();
  } else {
        _pkcsFrame->hide();
        _x509Frame->hide();
	_blankFrame->show();
        _import->setEnabled(false);
        _save->setEnabled(false);
	_curName = "";
  }
}


void KCertPart::slotImportAll() {
KSSLPKCS12 *p12Save = _p12;
KSSLCertificate *caSave = _ca;
QString curNameSave = _curName;

_p12 = NULL;
_ca = NULL;
_silentImport = true;

for (KPKCS12Item *t = dynamic_cast<KPKCS12Item*>(_parentP12->firstChild()); 
		                                                         t; 
			   t = dynamic_cast<KPKCS12Item*>(t->nextSibling())) {
	_p12 = t->cert;
	_curName = t->_prettyName;
	slotImport();
}
_p12 = NULL;

for (KX509Item *t = dynamic_cast<KX509Item*>(_parentCA->firstChild()); 
		                                                    t; 
			t = dynamic_cast<KX509Item*>(t->nextSibling())) {
	_ca = t->cert;
	_curName = t->_prettyName;
	slotImport();
}
_ca = NULL;

_signers->regenerate();
_silentImport = false;
_p12 = p12Save;
_ca = caSave;
_curName = curNameSave;
  KMessageBox::information(_frame, i18n("Certificates have been successfully imported into KDE.\nYou can manage your certificate settings from the KDE Control Center."), i18n("Certificate Import"));
}


KAboutData *KCertPart::createAboutData()
{
    return new KAboutData("KCertPart", I18N_NOOP("KDE Certificate Part"), "1.0");
}

#include "kcertpart.moc"

