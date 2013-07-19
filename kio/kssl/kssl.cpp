/* This file is part of the KDE project
 *
 * Copyright (C) 2000-2003 George Staikos <staikos@kde.org>
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

#include "kssl.h"

#include <ksslconfig.h>

// this hack provided by Malte Starostik to avoid glibc/openssl bug
// on some systems
#if KSSL_HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#undef crypt
#endif

#include <kopenssl.h>
#include <ksslx509v3.h>
#include <klocalizedstring.h>

#include <QAbstractSocket>
#include <QFile>

#ifdef __GNUC__
#warning "kssl.cpp contains temporary functions! Clean up"
#warning "kssl.cpp needs to be ported to QSslSocket"
#endif

#define sk_dup d->kossl->sk_dup

class KSSLPrivate {
public:
	KSSLPrivate() {
		kossl = KOpenSSLProxy::self();
	}

	~KSSLPrivate() {}

#if KSSL_HAVE_SSL
	SSL *m_ssl;
	SSL_CTX *m_ctx;
	SSL_METHOD *m_meth;
#endif
	KOSSL *kossl;
};


KSSL::KSSL(bool init) {
	d = new KSSLPrivate;
	m_bInit = false;
	m_bAutoReconfig = true;
	m_cfg = new KSSLSettings();
#if KSSL_HAVE_SSL
	d->m_ssl = 0L;
#endif

	if (init)
		initialize();
}


KSSL::~KSSL() {
	close();
	delete m_cfg;
	delete d;
}


int KSSL::seedWithEGD() {
    int rc = 0;
#if KSSL_HAVE_SSL
    if (m_cfg->useEGD() && !m_cfg->getEGDPath().isEmpty()) {
        rc = d->kossl->RAND_egd(QFile::encodeName(m_cfg->getEGDPath()).constData());
        if (rc < 0) {
            qWarning() << "KSSL: Error seeding PRNG with the EGD.";
        } else {
            //qDebug() << "KSSL: PRNG was seeded with" << rc << "bytes from the EGD.";
        }
    } else if (m_cfg->useEFile() && !m_cfg->getEGDPath().isEmpty()) {
        rc = d->kossl->RAND_load_file(QFile::encodeName(m_cfg->getEGDPath()).constData(), -1);
        if (rc < 0) {
            qWarning() << "KSSL: Error seeding PRNG with the entropy file.";
        } else {
           //qDebug() << "KSSL: PRNG was seeded with" << rc << "bytes from the entropy file.";
        }
    }
#endif
    return rc;
}


bool KSSL::initialize() {
#if KSSL_HAVE_SSL
    //qDebug() << "KSSL initialize";
	if (m_bInit)
		return false;

	if (m_bAutoReconfig)
		m_cfg->load();

	seedWithEGD();

	d->m_meth = d->kossl->SSLv23_client_method();
	d->m_ctx = d->kossl->SSL_CTX_new(d->m_meth);
	if (d->m_ctx == 0L) {
		return false;
	}

	// set cipher list
	QString clist = m_cfg->getCipherList();
	//qDebug() << "Cipher list: " << clist;
	if (!clist.isEmpty())
		d->kossl->SSL_CTX_set_cipher_list(d->m_ctx, const_cast<char *>(clist.toLatin1().constData()));

	m_bInit = true;
return true;
#else
return false;
#endif
}


void KSSL::close() {
#if KSSL_HAVE_SSL
    //qDebug() << "KSSL close";
	if (!m_bInit)
		return;

	if (d->m_ssl) {
		d->kossl->SSL_shutdown(d->m_ssl);
		d->kossl->SSL_free(d->m_ssl);
		d->m_ssl = 0L;
	}

	d->kossl->SSL_CTX_free(d->m_ctx);
	if (m_cfg->useEFile() && !m_cfg->getEGDPath().isEmpty()) {
		d->kossl->RAND_write_file(m_cfg->getEGDPath().toLatin1().constData());
	}

	m_bInit = false;
#endif
}


bool KSSL::reInitialize() {
	close();
return initialize();
}

// get the callback file - it's hidden away in here
//#include "ksslcallback.c"


bool KSSL::reconfig() {
	return reInitialize();
}


void KSSL::setAutoReconfig(bool ar) {
	m_bAutoReconfig = ar;
}


bool KSSL::setSettings(KSSLSettings *settings) {
	delete m_cfg;
	m_cfg = settings;
	return reconfig();
}

KSSLSettings * KSSL::settings()
{
    return m_cfg;
}


#if KSSL_HAVE_SSL
bool KSSL::m_bSSLWorks = true;
#else
bool KSSL::m_bSSLWorks = false;
#endif

bool KSSL::doesSSLWork() {
	return m_bSSLWorks;
}

#undef sk_dup

