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

#include <kopenssl.h>



KSSLKeyGen::KSSLKeyGen(QWidget *parent, const char *name, bool modal) 
:KDialog(parent,name,modal) {
#ifdef HAVE_SSL
	kossl = KOSSL::self();
#endif
}


KSSLKeyGen::~KSSLKeyGen() {
	
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
  
	// FIXME: private key!

	

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
   x << i18n("Sorry, no SSL support.");
#endif

return x;
}


#include "ksslkeygen.moc"

