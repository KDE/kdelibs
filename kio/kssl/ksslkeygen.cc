/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#include "ksslkeygen.h"
#include "keygenwizard.h"
#include "keygenwizard2.h"

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kopenssl.h>
#include <kprogress.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kwallet.h>

#include <qlineedit.h>
#include <qpushbutton.h>

#include <assert.h>


KSSLKeyGen::KSSLKeyGen(QWidget *parent, const char *name, bool modal) 
:KWizard(parent,name,modal) {
	_idx = -1;

#ifdef KSSL_HAVE_SSL
	page1 = new KGWizardPage1(this, "Wizard Page 1");
	addPage(page1, i18n("KDE Certificate Request"));
	page2 = new KGWizardPage2(this, "Wizard Page 2");
	addPage(page2, i18n("KDE Certificate Request - Password"));
	setHelpEnabled(page1, false);
	setHelpEnabled(page2, false);
	setFinishEnabled(page2, false);
	connect(page2->_password1, SIGNAL(textChanged(const QString&)), this, SLOT(slotPassChanged()));
	connect(page2->_password2, SIGNAL(textChanged(const QString&)), this, SLOT(slotPassChanged()));
	connect(finishButton(), SIGNAL(clicked()), SLOT(slotGenerate()));
#else
	// tell him he doesn't have SSL
#endif
}


KSSLKeyGen::~KSSLKeyGen() {
	
}


void KSSLKeyGen::slotPassChanged() {
	setFinishEnabled(page2, page2->_password1->text() == page2->_password2->text() && page2->_password1->text().length() >= 4);
}


void KSSLKeyGen::slotGenerate() {
	assert(_idx >= 0 && _idx <= 3);   // for now


	// Generate the CSR
	int bits;
	switch (_idx) {
	case 0:
		bits = 2048;
		break;
	case 1:
		bits = 1024;
		break;
	case 2:
		bits = 768;
		break;
	case 3:
		bits = 512;
		break;
	default:
		KMessageBox::sorry(NULL, i18n("Unsupported key size."), i18n("KDE SSL Information"));
		return;
	}

	KProgressDialog *kpd = new KProgressDialog(this, "progress dialog", i18n("KDE"), i18n("Please wait while the encryption keys are generated..."));
	kpd->progressBar()->setProgress(0);
	kpd->show();
	// FIXME - progress dialog won't show this way

	int rc = generateCSR("This CSR" /*FIXME */, page2->_password1->text(), bits, 0x10001 /* This is the traditional exponent used */);
	kpd->progressBar()->setProgress(100);

#ifndef Q_OS_WIN //TODO: reenable for WIN32
	if (rc == 0 && KWallet::Wallet::isEnabled()) {
		rc = KMessageBox::questionYesNo(this, i18n("Do you wish to store the passphrase in your wallet file?"), QString::null, i18n("Store"), i18n("Do Not Store"));
		if (rc == KMessageBox::Yes) {
			KWallet::Wallet *w = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), winId());
			if (w) {
				// FIXME: store passphrase in wallet
				delete w;
			}
		}
	}
#endif

	kpd->deleteLater();
}


int KSSLKeyGen::generateCSR(const QString& name, const QString& pass, int bits, int e) {
#ifdef KSSL_HAVE_SSL
	KOSSL *kossl = KOSSL::self();
	int rc;

	X509_REQ *req = kossl->X509_REQ_new();
	if (!req) {
		return -2;
	}

	EVP_PKEY *pkey = kossl->EVP_PKEY_new();
	if (!pkey) {
		kossl->X509_REQ_free(req);
		return -4;
	}

	RSA *rsakey = kossl->RSA_generate_key(bits, e, NULL, NULL);
	if (!rsakey) {
		kossl->X509_REQ_free(req);
		kossl->EVP_PKEY_free(pkey);
		return -3;
	}

	rc = kossl->EVP_PKEY_assign(pkey, EVP_PKEY_RSA, (char *)rsakey);

	rc = kossl->X509_REQ_set_pubkey(req, pkey);

	// Set the subject
	X509_NAME *n = kossl->X509_NAME_new();

	kossl->X509_NAME_add_entry_by_txt(n, (char*)LN_countryName, MBSTRING_UTF8, (unsigned char*)name.local8Bit().data(), -1, -1, 0);
	kossl->X509_NAME_add_entry_by_txt(n, (char*)LN_organizationName, MBSTRING_UTF8, (unsigned char*)name.local8Bit().data(), -1, -1, 0);
	kossl->X509_NAME_add_entry_by_txt(n, (char*)LN_organizationalUnitName, MBSTRING_UTF8, (unsigned char*)name.local8Bit().data(), -1, -1, 0);
	kossl->X509_NAME_add_entry_by_txt(n, (char*)LN_localityName, MBSTRING_UTF8, (unsigned char*)name.local8Bit().data(), -1, -1, 0);
	kossl->X509_NAME_add_entry_by_txt(n, (char*)LN_stateOrProvinceName, MBSTRING_UTF8, (unsigned char*)name.local8Bit().data(), -1, -1, 0);
	kossl->X509_NAME_add_entry_by_txt(n, (char*)LN_commonName, MBSTRING_UTF8, (unsigned char*)name.local8Bit().data(), -1, -1, 0);
	kossl->X509_NAME_add_entry_by_txt(n, (char*)LN_pkcs9_emailAddress, MBSTRING_UTF8, (unsigned char*)name.local8Bit().data(), -1, -1, 0);
	
	rc = kossl->X509_REQ_set_subject_name(req, n);


	rc = kossl->X509_REQ_sign(req, pkey, kossl->EVP_md5());

	// We write it to the database and then the caller can obtain it
	// back from there.  Yes it's inefficient, but it doesn't happen
	// often and this way things are uniform.
  
	KGlobal::dirs()->addResourceType("kssl", KStandardDirs::kde_default("data") + "kssl");

	QString path = KGlobal::dirs()->saveLocation("kssl");
	KTempFile csrFile(path + "csr_", ".der");

	if (!csrFile.fstream()) {
		kossl->X509_REQ_free(req);
		kossl->EVP_PKEY_free(pkey);
		return -5;
	}

	KTempFile p8File(path + "pkey_", ".p8");

	if (!p8File.fstream()) {
		kossl->X509_REQ_free(req);
		kossl->EVP_PKEY_free(pkey);
		return -5;
	}

	kossl->i2d_X509_REQ_fp(csrFile.fstream(), req);

	kossl->i2d_PKCS8PrivateKey_fp(p8File.fstream(), pkey,
			kossl->EVP_bf_cbc(), pass.local8Bit().data(),
			pass.length(), 0L, 0L);

	// FIXME Write kconfig entry to store the filenames under the md5 hash

	kossl->X509_REQ_free(req);
	kossl->EVP_PKEY_free(pkey);

	return 0;
#else
	return -1;
#endif
}


QStringList KSSLKeyGen::supportedKeySizes() {
	QStringList x;

#ifdef KSSL_HAVE_SSL
	x	<< i18n("2048 (High Grade)")
		<< i18n("1024 (Medium Grade)")
		<< i18n("768  (Low Grade)")
		<< i18n("512  (Low Grade)");
#else
	x	<< i18n("No SSL support.");
#endif

	return x;
}


#include "ksslkeygen.moc"

