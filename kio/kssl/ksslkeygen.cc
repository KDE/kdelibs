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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#include "ksslkeygen.h"
#include <klocale.h>
#include <kdebug.h>
#include "keygenwizard.h"
#include "keygenwizard2.h"
#include <qlineedit.h>
#include <qpushbutton.h>
#include <kmessagebox.h>

#include <assert.h>

#include <kopenssl.h>



KSSLKeyGen::KSSLKeyGen(QWidget *parent, const char *name, bool modal) 
:KWizard(parent,name,modal) {
	_idx = -1;

#ifdef HAVE_SSL
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
	assert(_idx >= 0 && _idx < 3);   // for now

// FOR NOW, it's DISABLED

	KMessageBox::sorry(NULL, i18n("Certificate request generation has been disabled for this release due to incomplete code."), i18n("KDE SSL Information"));
	return;


	// Show a progress box

	// Generate the CSR
	int bits;
	switch (_idx) {
	case 0:
		bits = 1024;
		break;
	case 1:
		bits = 768;
		break;
	case 2:
		bits = 512;
		break;
	default:
		return;
	}

	generateCSR("This CSR", page2->_password1->text(), bits, 0x10001);
}


int KSSLKeyGen::generateCSR(QString name, QString pass, int bits, int e) {
#ifdef HAVE_SSL
KOSSL *kossl = KOSSL::self();
X509_REQ *req;
int rc;

	req = kossl->X509_REQ_new();
	if (!req)
		return -2;

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

	// We write it to the database and then the caller can obtain it
	// back from there.  Yes it's inefficient, but it doesn't happen
	// often and this way things are uniform.
  
	FILE *fp;
	fp = fopen("keygencsrtest.der", "w");

	kossl->i2d_X509_REQ_fp(fp, req);

	fclose(fp);

	// FIXME: private key!

	// FIXME: do we have to free "rsakey" ourself?  Small leak anyways..
	
	kossl->X509_REQ_free(req);

return 0;
#else
return -1;
#endif
}


QStringList KSSLKeyGen::supportedKeySizes() {
QStringList x;

#ifdef HAVE_SSL
   x << "1024"
     << "768"
     << "512";
#else
   x << i18n("No SSL support.");
#endif

return x;
}


#include "ksslkeygen.moc"

