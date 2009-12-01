/* This file is part of the KDE project
 *
 * Copyright (C) 2001-2003 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kcertpart.h"
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <QtGui/QFrame>
#include <klocale.h>
#include <kdebug.h>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <kmessagebox.h>
#include <kpassworddialog.h>
#include <ksslall.h>
#include <kopenssl.h>
#include <ksslcertificatebox.h>
#include <ksslpemcallback.h>
#include <kfiledialog.h>
#include <QtCore/Q_PID>
#include <kseparator.h>
#include <QtGui/QTreeWidget>
#include <QtCore/QRegExp>
#include <kcombobox.h>
#include <kparts/browserextension.h>
#include <kparts/browserinterface.h>
#include <kmimetype.h>
#include <ktabwidget.h>
#include <ktextedit.h>

K_PLUGIN_FACTORY( KCertPartFactory, registerPlugin<KCertPart>(); )
K_EXPORT_PLUGIN( KCertPartFactory("KCertPart") )


KX509Item::KX509Item(QTreeWidgetItem *parent, KSSLCertificate *x) :
	QTreeWidgetItem(parent, 1001)
{
	setup(x);
}

KX509Item::KX509Item(QTreeWidget *parent, KSSLCertificate *x) :
	QTreeWidgetItem(parent)
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
		OU.remove(QRegExp("\n.*"));
		CN.remove(QRegExp("\n.*"));

		if (OU.length() > 0) {
			_prettyName = OU;
		}

		if (CN.length() > 0) {
			if (_prettyName.length() > 0) {
				_prettyName += " - ";
			}
			_prettyName += CN;
		}
		setText(0, _prettyName);
	} else {
		setText(0, i18n("Invalid certificate"));
	}
}


KX509Item::~KX509Item()
{
	delete cert;
}


KPKCS12Item::KPKCS12Item(QTreeWidgetItem *parent, KSSLPKCS12 *x) :
	QTreeWidgetItem(parent)
{
	cert = x;
	if (x) {
		KSSLX509Map xm(x->getCertificate()->getSubject());
		QString CN = "CN";
		CN = xm.getValue(CN);
		CN.remove(QRegExp("\n.*"));
		_prettyName = CN;
		setText(0, _prettyName);
	} else {
		setText(0, i18n("Invalid certificate"));
	}
}


KPKCS12Item::~KPKCS12Item()
{
	delete cert;
}


class KCertPartPrivate {
	public:
		KParts::BrowserExtension *browserExtension;
};


KCertPart::KCertPart(QWidget *parentWidget,
					 QObject *parent,
					 const QVariantList & /*args*/ )
    : KParts::ReadWritePart(parent),
    d(new KCertPartPrivate)
{
    setComponentData(KCertPartFactory::componentData());
    QGridLayout *grid;


	_signers = new KSSLSigners;
// This is a bit confusing now.	 Here's how it works:
//	  We create a _frame and split it left/right
//	  Then we add the ListView to the left and create
//	  a new frame on the right.	 We set the main widget
//	  on the right.

	_p12 = NULL;
	_ca = NULL;
	_silentImport = false;
	d->browserExtension = new KParts::BrowserExtension(this);

	_frame = new QFrame(parentWidget);
	setWidget(_frame);

	_baseGrid = new QGridLayout(_frame);
	_baseGrid->setMargin(KDialog::marginHint());
	_baseGrid->setSpacing(KDialog::spacingHint());

	_sideList = new QTreeWidget(_frame);
	_sideList->setRootIsDecorated(true);
	_sideList->setHeaderLabels(QStringList() << i18n("Certificates"));
	_parentCA = new QTreeWidgetItem(_sideList, QStringList() << i18n("Signers"));
	_parentCA->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
	_parentCA->setExpanded(true);
	_parentP12 = new QTreeWidgetItem(_sideList, QStringList() << i18n("Client"));
	_parentP12->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
	_parentP12->setExpanded(true);

	_baseGrid->addWidget(_sideList, 0, 0, 14, 2);

	_importAll = new QPushButton(i18n("Import &All"), _frame);
	_baseGrid->addWidget(_importAll, 14, 0, 1, 2);
	connect(_importAll, SIGNAL(clicked()), SLOT(slotImportAll()));



//------------------------------------------------------------------------
//	 The PKCS widget
//------------------------------------------------------------------------
	_pkcsFrame = new QFrame(_frame);

	grid = new QGridLayout(_pkcsFrame);
	grid->setMargin(KDialog::marginHint());
	grid->setSpacing(KDialog::spacingHint());
	grid->addWidget(new QLabel(i18n("KDE Secure Certificate Import"), _pkcsFrame), 0, 0, 1, 6);
	grid->addWidget(new QLabel(i18n("Chain:"), _pkcsFrame), 1, 0);
	_p12_chain = new KComboBox(_pkcsFrame);
	grid->addWidget(_p12_chain, 1, 1, 1, 4);
	connect(_p12_chain, SIGNAL(activated(int)), SLOT(slotChain(int)));

	grid->addWidget(new QLabel(i18n("Subject:"), _pkcsFrame), 2, 0);
	grid->addWidget(new QLabel(i18n("Issued by:"), _pkcsFrame), 2, 3);
	_p12_subject = new KSslCertificateBox(_pkcsFrame);
	_p12_issuer = new KSslCertificateBox(_pkcsFrame);
	grid->addWidget(_p12_subject, 3, 0, 4, 3);
	grid->addWidget(_p12_issuer, 3, 3, 4, 3);

	grid->addWidget(new QLabel(i18n("File:"), _pkcsFrame), 7, 0);
	_p12_filenameLabel = new QLabel("", _pkcsFrame);
	grid->addWidget(_p12_filenameLabel, 7, 1);
	grid->addWidget(new QLabel(i18n("File format:"), _pkcsFrame), 7, 3);
	grid->addWidget(new QLabel("PKCS#12", _pkcsFrame), 7, 4);


//
//	Make the first tab
//
	_tabs = new KTabWidget(_pkcsFrame);
	grid->addWidget(_tabs, 8, 0, 5, 6);

	QFrame *tab = new QFrame(_pkcsFrame);
	QGridLayout *tabGrid = new QGridLayout(tab);
	tabGrid->setMargin(KDialog::marginHint());
	tabGrid->setSpacing(KDialog::spacingHint());
	tabGrid->addWidget(new QLabel(i18nc("State of the certification", "State:"), tab), 0, 0);
	_p12_certState = new QLabel("", tab);
	tabGrid->addWidget(_p12_certState, 0, 1, 1, 4);

	tabGrid->addWidget(new QLabel(i18n("Valid from:"), tab), 1, 0);
	_p12_validFrom = new QLabel("", tab);
	tabGrid->addWidget(_p12_validFrom, 1, 1, 1, 4);

	tabGrid->addWidget(new QLabel(i18n("Valid until:"), tab), 2, 0);
	_p12_validUntil = new QLabel("", tab);
	tabGrid->addWidget(_p12_validUntil, 2, 1, 1, 4);

	tabGrid->addWidget(new QLabel(i18n("Serial number:"), tab), 3, 0);
	_p12_serialNum = new QLabel("", tab);
	tabGrid->addWidget(_p12_serialNum, 3, 1);
	_tabs->addTab(tab, i18nc("State of the certification", "State"));


//
// Make the second tab
//
	tab = new QFrame(_pkcsFrame);
	tabGrid = new QGridLayout(tab);
	tabGrid->setMargin(KDialog::marginHint());
	tabGrid->setSpacing(KDialog::spacingHint());
	tabGrid->addWidget(new QLabel(i18n("MD5 digest:"), tab), 0, 0);
	_p12_digest = new QLabel(tab);
	tabGrid->addWidget(_p12_digest, 0, 1, 1, 4);
	tabGrid->addWidget(new QLabel(i18n("Signature:"), tab), 1, 0);
	_p12_sig = new KTextEdit(tab);
	tabGrid->addWidget(_p12_sig, 1, 1, 3, 4);
	_p12_sig->setReadOnly(true);

	_tabs->addTab(tab, i18n("Signature"));


//
// Make the third tab
//
	tab = new QFrame(_pkcsFrame);
	tabGrid = new QGridLayout(tab);
	tabGrid->setMargin(KDialog::marginHint());
	tabGrid->setSpacing(KDialog::spacingHint());
	tabGrid->addWidget(new QLabel(i18n("Public key:"), tab), 0, 0);
	_p12_pubkey = new KTextEdit(tab);
	tabGrid->addWidget(_p12_pubkey, 0, 1, 4, 4);
	_p12_pubkey->setReadOnly(true);


	_tabs->addTab(tab, i18n("Public Key"));

	_pkcsFrame->hide();

//------------------------------------------------------------------------
//	 The X509 widget
//------------------------------------------------------------------------
//	 Note: this is almost identical to the above, but I duplicate it for
//	 the simple reason that the above has potential to display much
//	 more information, and this one has potential to display different
//	 information.
	_x509Frame = new QFrame(_frame);

	grid = new QGridLayout(_x509Frame);
	grid->setMargin(KDialog::marginHint());
	grid->setSpacing(KDialog::spacingHint());
	grid->addWidget(new QLabel(i18n("KDE Secure Certificate Import"), _x509Frame), 0, 0, 1, 6);

	grid->addWidget(new QLabel(i18n("Subject:"), _x509Frame), 1, 0);
	grid->addWidget(new QLabel(i18n("Issued by:"), _x509Frame), 1, 3);
	_ca_subject = new KSslCertificateBox(_x509Frame);
	_ca_issuer = new KSslCertificateBox(_x509Frame);
	grid->addWidget(_ca_subject, 2, 0, 4, 3);
	grid->addWidget(_ca_issuer, 2, 3, 4, 3);

	grid->addWidget(new QLabel(i18n("File:"), _x509Frame), 6, 0);
	_ca_filenameLabel = new QLabel("", _x509Frame);
	grid->addWidget(_ca_filenameLabel, 6, 1);
	grid->addWidget(new QLabel(i18n("File format:"), _x509Frame), 6, 3);
	grid->addWidget(new QLabel(i18n("PEM or DER Encoded X.509"), _x509Frame), 6, 4);


//
//	Make the first tab
//
	_tabs = new KTabWidget(_x509Frame);
	grid->addWidget(_tabs, 7, 0, 5, 6);

	tab = new QFrame(_x509Frame);
	tabGrid = new QGridLayout(tab);
	tabGrid->setMargin(KDialog::marginHint());
	tabGrid->setSpacing(KDialog::spacingHint());
	tabGrid->addWidget(new QLabel(i18nc("State of the certification", "State:"), tab), 0, 0);
	_ca_certState = new QLabel("", tab);
	tabGrid->addWidget(_ca_certState, 0, 1, 1, 4);

	tabGrid->addWidget(new QLabel(i18n("Valid from:"), tab), 1, 0);
	_ca_validFrom = new QLabel("", tab);
	tabGrid->addWidget(_ca_validFrom, 1, 1, 1, 4);

	tabGrid->addWidget(new QLabel(i18n("Valid until:"), tab), 2, 0);
	_ca_validUntil = new QLabel("", tab);
	tabGrid->addWidget(_ca_validUntil, 2, 1, 1, 4);

	tabGrid->addWidget(new QLabel(i18n("Serial number:"), tab), 3, 0);
	_ca_serialNum = new QLabel("", tab);
	tabGrid->addWidget(_ca_serialNum, 3, 1);
	_tabs->addTab(tab, i18nc("State of the certification", "State"));


//
// Make the second tab
//
	tab = new QFrame(_x509Frame);
	tabGrid = new QGridLayout(tab);
	tabGrid->setMargin(KDialog::marginHint());
	tabGrid->setSpacing(KDialog::spacingHint());
	tabGrid->addWidget(new QLabel(i18n("MD5 digest:"), tab), 0, 0);
	_ca_digest = new QLabel(tab);
	tabGrid->addWidget(_ca_digest, 0, 1, 1, 4);
	tabGrid->addWidget(new QLabel(i18n("Signature:"), tab), 1, 0);
	_ca_sig = new KTextEdit(tab);
	tabGrid->addWidget(_ca_sig, 1, 1, 3, 4);
	_ca_sig->setReadOnly(true);

	_tabs->addTab(tab, i18n("Signature"));


//
// Make the third tab
//
	tab = new QFrame(_x509Frame);
	tabGrid = new QGridLayout(tab);
	tabGrid->setMargin(KDialog::marginHint());
	tabGrid->setSpacing(KDialog::spacingHint());
	tabGrid->addWidget(new QLabel(i18n("Public key:"), tab), 0, 0);
	_ca_pubkey = new KTextEdit(tab);
	tabGrid->addWidget(_ca_pubkey, 0, 1, 4, 4);
	_ca_pubkey->setReadOnly(true);


	_tabs->addTab(tab, i18n("Public Key"));

	_x509Frame->hide();




//------------------------------------------------------------------------
//	 The blank widget
//------------------------------------------------------------------------
	_blankFrame = new QFrame(_frame);
	grid = new QGridLayout(_blankFrame);
	grid->setMargin(KDialog::marginHint());
	grid->setSpacing(KDialog::spacingHint());
	grid->addWidget(new QLabel(i18n("KDE Secure Certificate Import"), _blankFrame), 0, 0, 1, 1);
	_blankFrame->show();



//
// Finish it off
//
	_baseGrid->addWidget(new KSeparator(_frame), 13, 2, 1, 7);
	_launch = new QPushButton(i18n("&Crypto Manager..."), _frame);
	_import = new QPushButton(i18n("&Import"), _frame);
	_save = new QPushButton(i18n("&Save..."), _frame);
	_done = new QPushButton(i18n("&Done"), _frame);
	_baseGrid->addWidget(_launch, 14, 4, 1, 2);
	_baseGrid->addWidget(_import, 14, 6);
	_baseGrid->addWidget(_save, 14, 7);
	_baseGrid->addWidget(_done, 14, 8);
	connect(_launch, SIGNAL(clicked()), SLOT(slotLaunch()));
	connect(_import, SIGNAL(clicked()), SLOT(slotImport()));
	connect(_save, SIGNAL(clicked()), SLOT(slotSave()));
	connect(_done, SIGNAL(clicked()), SLOT(slotDone()));
	_import->setEnabled(false);
	_save->setEnabled(false);



	_baseGrid->addWidget(_pkcsFrame, 0, 2, 13, 7);
	_baseGrid->addWidget(_x509Frame, 0, 2, 13, 7);
	_baseGrid->addWidget(_blankFrame, 0, 2, 13, 7);

	connect(_sideList, SIGNAL(itemSelectionChanged()),
			this, SLOT(slotSelectionChanged()));
	setReadWrite(true);
}


KCertPart::~KCertPart() {
	delete _signers;
	delete d->browserExtension;
	delete d;
}


void KCertPart::setReadWrite(bool rw) {
	if (!rw) {
		_import->setEnabled(false);
		_save->setEnabled(false);
	}
	KParts::ReadWritePart::setReadWrite(rw);
}


bool KCertPart::saveFile() {
	if (_p12) {
		QString certFile = KFileDialog::getSaveFileName(QString(), "application/x-pkcs12");
		if (certFile.isEmpty())
			return false;

		if (!_p12->toFile(certFile)) {
			KMessageBox::sorry(_frame, i18n("Save failed."), i18n("Certificate Import"));
			return false;
		}

		return true;
	} else if (_ca) {
		QString certFile = KFileDialog::getSaveFileName(QString(), "application/x-x509-ca-cert");
		if (certFile.isEmpty())
			return false;

		QByteArray enc;
		if (certFile.endsWith(QLatin1String("der")) || certFile.endsWith(QLatin1String("crt"))) {
			enc = _ca->toDer();
		} else if (certFile.endsWith(QLatin1String("netscape"))) {
			enc = _ca->toNetscape();
		} else {
			enc = _ca->toPem();
		}

		QFile of(certFile);

		if (!of.open(QIODevice::WriteOnly) || of.write(enc) != enc.size()) {
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
#ifndef KSSL_HAVE_SSL
	KMessageBox::sorry(_frame, i18n("You do not seem to have compiled KDE with SSL support."), i18n("Certificate Import"));
	return false;
#else

	if (QFileInfo(localFilePath()).size() == 0) {
		KMessageBox::sorry(_frame, i18n("Certificate file is empty."), i18n("Certificate Import"));
		return false;
	}

	QString whatType = arguments().mimeType();
//whatType = KMimeType::findByURL(m_url,0,true)->name();
	if (whatType.isEmpty())
		whatType = KMimeType::findByPath(localFilePath(), 0, true)->name();

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
//		 x-pkcs12 loading
/////////////////////////////////////////////////////////////////////////////
		if (whatType == "application/x-pkcs12") {
			_p12 = KSSLPKCS12::loadCertFile(localFilePath());

			while (!_p12) {
				// try prompting for a password.
				KPasswordDialog dlg(_frame);
				dlg.setCaption(i18n("Certificate Password"));
				dlg.setPrompt(i18n("Certificate Password"));
				if( !dlg.exec() )
					break;

				_p12 = KSSLPKCS12::loadCertFile(localFilePath(), dlg.password());

				if (!_p12) {
					int rc = KMessageBox::warningContinueCancel(_frame, i18n("The certificate file could not be loaded. Try a different password?"), i18n("Certificate Import"),KGuiItem(i18n("Try Different")));
					if (rc == KMessageBox::Continue) continue;
					break;
				}
			}

			if (!_p12) return false;

			new KPKCS12Item(_parentP12, _p12);
			_p12 = NULL;
			return true;
			/////////////////////////////////////////////////////////////////////////////
				//		 x-509-ca-cert loading
				/////////////////////////////////////////////////////////////////////////////
				} else if (whatType == "application/x-x509-ca-cert" ||
						   whatType == "application/binary-certificate") { // DF: this mimetype does not exist
			FILE *fp;
			bool isPEM = false;

			_ca_filenameLabel->setText(localFilePath());

			/////////////  UGLY HACK TO GET AROUND OPENSSL PROBLEMS ///////////
				if (whatType == "application/x-x509-ca-cert") {
					// Check if it is PEM or not
					QFile qf(localFilePath());
					qf.open(QIODevice::ReadOnly);
					QByteArray theFile = qf.readAll();
					qf.close();

					const char *signature = "-----BEGIN CERTIFICATE-----";
					isPEM = theFile.contains(signature);
				}

				fp = fopen(localFilePath().toLocal8Bit(), "r");
				if (!fp) {
					KMessageBox::sorry(_frame, i18n("This file cannot be opened."), i18n("Certificate Import"));
					return false;
				}

				/*
				  kDebug() << "Reading in a file in "
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

				_ca_filenameLabel->setText(localFilePath());
				for (int i = 0; i < sk_X509_INFO_num(sx5i); i++) {
					X509_INFO* x5i = sk_X509_INFO_value(sx5i, i);
					if (x5i->x_pkey && x5i->x509) {	  // a personal cert (like PKCS12)
						KSSLCertificate *xc = KSSLCertificate::fromX509(x5i->x509);
						new KX509Item(_sideList, xc);
					} else if (x5i->x509) {	  // something else - maybe a CA file
						KSSLCertificate *xc = KSSLCertificate::fromX509(x5i->x509);
						if (xc->x509V3Extensions().certTypeCA())
							new KX509Item(_parentCA, xc);
						else new KX509Item(_sideList, xc);
					} else if (x5i->crl) {	 // a crl
						kDebug() << "Found a CRL...";
					}
				}

				sk_X509_INFO_free(sx5i);

#undef sk_free
#undef sk_num
#undef sk_value

				fclose(fp);
				return true;
				/////////////////////////////////////////////////////////////////////////////
					//		 Dunno how to load this
					/////////////////////////////////////////////////////////////////////////////
					} else {
			QString emsg = i18n("I do not know how to handle this type of file.") + '\n' + whatType;
			KMessageBox::sorry(_frame, emsg, i18n("Certificate Import"));
			return false;
		}
#endif
}


void KCertPart::displayPKCS12() {
	KSSLCertificate *xc = _p12->getCertificate();
	_p12_filenameLabel->setText(localFilePath());
	displayPKCS12Cert(xc);
	_p12_certState->setText(KSSLCertificate::verifyText(_p12->validate()));

	// Set the chain if it's there
	if (xc->chain().depth() > 1) {
		const QList<KSSLCertificate *> cl = xc->chain().getChain();
		int cnt = 0;
		_p12_chain->setEnabled(true);
		_p12_chain->clear();
		_p12_chain->addItem(i18n("0 - Site Certificate"));
		foreach (KSSLCertificate *c, cl) {
			KSSLX509Map map(c->getSubject());
			_p12_chain->addItem(QString::number(++cnt)+" - "+map.getValue("CN"));
		}
                qDeleteAll(cl);
		_p12_chain->setCurrentIndex(0);
	} else {
		_p12_chain->clear();
		_p12_chain->setEnabled(false);
	}
}


void KCertPart::displayCACert(KSSLCertificate *c) {
	// We have the file, lets work with it.
//### 	_ca_subject->setValues(c->getSubject());
//### 	_ca_issuer->setValues(c->getIssuer());

	// Set the valid period
	QPalette cspl = _ca_validFrom->palette();
	if (QDateTime::currentDateTime() < c->getQDTNotBefore()) {
		cspl.setColor(QPalette::Foreground, QColor(196,33,21));
	} else {
		cspl.setColor(QPalette::Foreground, QColor(42,153,59));
	}
	_ca_validFrom->setPalette(cspl);
	_ca_validFrom->setText(c->getNotBefore());

	cspl = _ca_validUntil->palette();
	if (QDateTime::currentDateTime() > c->getQDTNotAfter()) {
		cspl.setColor(QPalette::Foreground, QColor(196,33,21));
	} else {
		cspl.setColor(QPalette::Foreground, QColor(42,153,59));
	}
	_ca_validUntil->setPalette(cspl);
	_ca_validUntil->setText(c->getNotAfter());

	_ca_serialNum->setText(c->getSerialNumber());
	cspl = _ca_certState->palette();
	if (!c->isValid()) {
		cspl.setColor(QPalette::Foreground, QColor(196,33,21));
	} else {
		cspl.setColor(QPalette::Foreground, QColor(42,153,59));
	}
	_ca_certState->setPalette(cspl);
	_ca_certState->setText(KSSLCertificate::verifyText(c->validate()));

	_ca_pubkey->setPlainText (c->getPublicKeyText());
	_ca_digest->setText(c->getMD5DigestText());
	_ca_sig->setPlainText(c->getSignatureText());
}



void KCertPart::displayPKCS12Cert(KSSLCertificate *c) {
	// We have the file, lets work with it.
//### 	_p12_subject->setValues(c->getSubject());
//### 	_p12_issuer->setValues(c->getIssuer());

	// Set the valid period
	QPalette cspl = _p12_validFrom->palette();
	if (QDateTime::currentDateTime() < c->getQDTNotBefore()) {
		cspl.setColor(QPalette::Foreground, QColor(196,33,21));
	} else {
		cspl.setColor(QPalette::Foreground, QColor(42,153,59));
	}
	_p12_validFrom->setPalette(cspl);
	_p12_validFrom->setText(c->getNotBefore());

	cspl = _p12_validUntil->palette();
	if (QDateTime::currentDateTime() > c->getQDTNotAfter()) {
		cspl.setColor(QPalette::Foreground, QColor(196,33,21));
	} else {
		cspl.setColor(QPalette::Foreground, QColor(42,153,59));
	}
	_p12_validUntil->setPalette(cspl);
	_p12_validUntil->setText(c->getNotAfter());

	_p12_serialNum->setText(c->getSerialNumber());
	cspl = _p12_certState->palette();
	if (!c->isValid()) {
		cspl.setColor(QPalette::Foreground, QColor(196,33,21));
	} else {
		cspl.setColor(QPalette::Foreground, QColor(42,153,59));
	}
	_p12_certState->setPalette(cspl);
	_p12_certState->setText(KSSLCertificate::verifyText(c->validate()));

	_p12_pubkey->setPlainText(c->getPublicKeyText());
	_p12_digest->setText(c->getMD5DigestText());
	_p12_sig->setPlainText(c->getSignatureText());
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
		KConfig cfg("ksslcertificates", KConfig::SimpleConfig);

		if (cfg.hasGroup(_p12->getCertificate()->getSubject())) {
			QString msg = _curName + '\n' + i18n("A certificate with that name already exists. Are you sure that you wish to replace it?");
			int rc= KMessageBox::warningContinueCancel(_frame, msg, i18n("Certificate Import"),KGuiItem(i18n("Replace")));
			if (rc == KMessageBox::Cancel) {
				return;
			}
		}

                KConfigGroup cg(&cfg, _p12->getCertificate()->getSubject());
		cg.writeEntry("PKCS12Base64", _p12->toString());
		cg.writeEntry("Password", "");
		cg.sync();
		if (!_silentImport)
			KMessageBox::information(_frame, i18n("Certificate has been successfully imported into KDE.\nYou can manage your certificate settings from the KDE System Settings."), i18n("Certificate Import"));
	} else if (_ca) {
		KConfig cfg("ksslcalist", KConfig::NoGlobals);
		if (cfg.hasGroup(_ca->getSubject())) {
			QString msg = _curName + '\n' + i18n("A certificate with that name already exists. Are you sure that you wish to replace it?");
			int rc= KMessageBox::warningContinueCancel(_frame, msg, i18n("Certificate Import"),KGuiItem(i18n("Replace")));
			if (rc == KMessageBox::Cancel) {
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
			KMessageBox::information(_frame, i18n("Certificate has been successfully imported into KDE.\nYou can manage your certificate settings from the KDE System Settings."), i18n("Certificate Import"));
	}
}


void KCertPart::slotSave() {
	saveFile();
}


void KCertPart::slotDone() {
	KParts::BrowserInterface *iface = d->browserExtension->browserInterface();
	iface->callMethod("goHistory", -1);
}


void KCertPart::slotLaunch() {
	QProcess::startDetached("kcmshell4", QStringList() << "crypto");
}


void KCertPart::slotSelectionChanged() {
	// we assume that there is only one item selected...
	QTreeWidgetItem *x = _sideList->selectedItems().at(0);
	KX509Item *x5i = dynamic_cast<KX509Item*>(x);
	KPKCS12Item *p12i = dynamic_cast<KPKCS12Item*>(x);
	_p12 = NULL;
	_ca = NULL;
	if (x && x->parent() == _parentCA) {
		if (!x5i) {
			return;
		}
		x5i->cert->revalidate();
		_blankFrame->hide();
		_pkcsFrame->hide();
		_x509Frame->show();
		_ca = x5i->cert;
		_import->setEnabled(true);
		_save->setEnabled(true);
		_curName = x5i->_prettyName;
		displayCACert(_ca);
	} else if (x && x->parent() == NULL && x->type() == 1001) {
		if (!x5i) {
			return;
		}
		x5i->cert->revalidate();
		_blankFrame->hide();
		_pkcsFrame->hide();
		_x509Frame->show();
		_ca = x5i->cert;
		_import->setEnabled(false);
		_save->setEnabled(false);
		_curName = x5i->_prettyName;
		displayCACert(_ca);
	} else if (x && x->parent() == _parentP12) {
		if (!p12i) {
			return;
		}
		p12i->cert->revalidate();
		_blankFrame->hide();
		_x509Frame->hide();
		_pkcsFrame->show();
		_p12 = p12i->cert;
		_import->setEnabled(true);
		_save->setEnabled(true);
		_curName = p12i->_prettyName;
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

	QTreeWidgetItemIterator it(_parentP12);
	while (*it) {
			dynamic_cast<KPKCS12Item*>(*it)->cert;
			dynamic_cast<KPKCS12Item*>(*it)->_prettyName;
			slotImport();
			it++;
	}
	_p12 = NULL;

	it = QTreeWidgetItemIterator(_parentCA);
	while (*it) {
			dynamic_cast<KX509Item*>(*it)->cert;
			dynamic_cast<KX509Item*>(*it)->_prettyName;
			slotImport();
			it++;
	}
	_ca = NULL;

	_signers->regenerate();
	_silentImport = false;
	_p12 = p12Save;
	_ca = caSave;
	_curName = curNameSave;
	KMessageBox::information(_frame, i18n("Certificates have been successfully imported into KDE.\nYou can manage your certificate settings from the KDE System Settings."), i18n("Certificate Import"));
}


KAboutData *KCertPart::createAboutData()
{
	return new KAboutData("KCertPart", 0, ki18n("KDE Certificate Part"), "1.0");
}

#include "kcertpart.moc"
