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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// this hack provided by Malte Starostik to avoid glibc/openssl bug
// on some systems
#ifdef KSSL_HAVE_SSL
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#undef crypt
#endif

#include "kssl.h"

#include <kdebug.h>
#include <kstandarddirs.h>
#include <ksock.h>
#include <ksockaddr.h>

#include <kopenssl.h>
#include <ksslx509v3.h>
#include <ksslpkcs12.h>
#include <ksslsession.h>
#include <klocale.h>
#include <ksocks.h>

#define sk_dup d->kossl->sk_dup

class KSSLPrivate {
public:
	KSSLPrivate() {
		lastInitTLS = false;
		kossl = KOpenSSLProxy::self();
		session = 0L;
	}

	~KSSLPrivate() {
		delete session;
		session = 0L;
	}

	bool lastInitTLS;
	KSSLCertificate::KSSLValidation m_cert_vfy_res;
	QString proxyPeer;

#ifdef KSSL_HAVE_SSL
	SSL *m_ssl;
	SSL_CTX *m_ctx;
	SSL_METHOD *m_meth;
#endif
	KSSLSession *session;
	KOSSL *kossl;
};


KSSL::KSSL(bool init) {
	d = new KSSLPrivate;
	m_bInit = false;
	m_bAutoReconfig = true;
	m_cfg = new KSSLSettings();
#ifdef KSSL_HAVE_SSL  
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
#ifdef KSSL_HAVE_SSL
	if (m_cfg->useEGD() && !m_cfg->getEGDPath().isEmpty()) {
		rc = d->kossl->RAND_egd(m_cfg->getEGDPath().latin1());
		if (rc < 0) 
			kdDebug(7029) << "KSSL: Error seeding PRNG with the EGD." << endl;
		else kdDebug(7029) << "KSSL: PRNG was seeded with " << rc 
				   << " bytes from the EGD." << endl;
	} else if (m_cfg->useEFile() && !m_cfg->getEGDPath().isEmpty()) {
		rc = d->kossl->RAND_load_file(m_cfg->getEGDPath().latin1(), -1);
		if (rc < 0) 
			kdDebug(7029) << "KSSL: Error seeding PRNG with the entropy file." << endl;
		else kdDebug(7029) << "KSSL: PRNG was seeded with " << rc 
				   << " bytes from the entropy file." << endl;
	}
#endif
return rc;
}


bool KSSL::TLSInit() {
#ifdef KSSL_HAVE_SSL
// kdDebug(7029) << "KSSL TLS initialize" << endl;
	if (m_bInit)
		return false;

	if (m_bAutoReconfig)
	m_cfg->load();

	if (!m_cfg->tlsv1())
		return false;

	seedWithEGD();
	d->m_meth = d->kossl->TLSv1_client_method();
	d->lastInitTLS = true;

	d->m_ctx = d->kossl->SSL_CTX_new(d->m_meth);
	if (d->m_ctx == 0L) {
		return false;
	}

	// set cipher list
	QString clist = m_cfg->getCipherList();
	//kdDebug(7029) << "Cipher list: " << clist << endl;
	if (!clist.isEmpty())
		d->kossl->SSL_CTX_set_cipher_list(d->m_ctx, const_cast<char *>(clist.ascii()));

	m_bInit = true;
return true;
#else
return false;
#endif
}


bool KSSL::initialize() {
#ifdef KSSL_HAVE_SSL
	kdDebug(7029) << "KSSL initialize" << endl;
	if (m_bInit)
		return false;

	if (m_bAutoReconfig)
		m_cfg->load();

	seedWithEGD();
	// FIXME: we should be able to force SSL off entirely.
	d->lastInitTLS = false;

	m_pi.reset();

	if (m_cfg->sslv2() && !m_cfg->sslv3())
		d->m_meth = d->kossl->SSLv2_client_method();
	else if (m_cfg->sslv3() && !m_cfg->sslv2())
		d->m_meth = d->kossl->SSLv3_client_method();
	else d->m_meth = d->kossl->SSLv23_client_method();

/*
if (m_cfg->sslv2() && m_cfg->sslv3()) kdDebug(7029) << "Double method" << endl;
else if (m_cfg->sslv2()) kdDebug(7029) << "SSL2 method" << endl;
else if (m_cfg->sslv3()) kdDebug(7029) << "SSL3 method" << endl;
*/

	d->m_ctx = d->kossl->SSL_CTX_new(d->m_meth);
	if (d->m_ctx == 0L) {
		return false;
	}

	// set cipher list
	QString clist = m_cfg->getCipherList();
	kdDebug(7029) << "Cipher list: " << clist << endl;
	if (!clist.isEmpty())
		d->kossl->SSL_CTX_set_cipher_list(d->m_ctx, const_cast<char *>(clist.ascii()));

	m_bInit = true;
return true;
#else
return false;
#endif
}


bool KSSL::setSession(const KSSLSession *session) {
#ifdef KSSL_HAVE_SSL
	if (!session) {
		delete d->session;
		d->session = 0L;
		return true;
	}

	// Obtain a reference by incrementing the reference count.  Yuck.
	static_cast<SSL_SESSION*>(session->_session)->references++;

	d->session = new KSSLSession;
	d->session->_session = session->_session;

	return true;
#else
	return false;
#endif
}


void KSSL::close() {
#ifdef KSSL_HAVE_SSL
//kdDebug(7029) << "KSSL close" << endl;
	if (!m_bInit)
		return;

	delete d->session;
	d->session = 0L;

	if (d->m_ssl) {
		d->kossl->SSL_shutdown(d->m_ssl);
		d->kossl->SSL_free(d->m_ssl);
		d->m_ssl = 0L;
	}

	d->kossl->SSL_CTX_free(d->m_ctx);
	if (m_cfg->useEFile() && !m_cfg->getEGDPath().isEmpty()) {
		d->kossl->RAND_write_file(m_cfg->getEGDPath().latin1());
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


bool KSSL::setVerificationLogic() {
#if 0
#ifdef KSSL_HAVE_SSL
  //  SSL_set_verify_result(d->m_ssl, X509_V_OK);
  //  SSL_CTX_set_verify(d->m_ctx, SSL_VERIFY_PEER, X509Callback);
#endif
#endif
return true;
}


int KSSL::accept(int sock) {
#ifdef KSSL_HAVE_SSL
// kdDebug(7029) << "KSSL accept" << endl;
int rc;
	if (!m_bInit)
		return -1;
	d->m_ssl = d->kossl->SSL_new(d->m_ctx);
	if (!d->m_ssl)
		return -1;

	if (d->session) {
		if (static_cast<SSL_SESSION*>(d->session->_session)->sess_cert == 0)
		{
			kdDebug(7029) << "Can't reuse session, no certificate." << endl;
			delete d->session;
			d->session = 0;
		} else if (1 == d->kossl->SSL_set_session(d->m_ssl,
			static_cast<SSL_SESSION*>(d->session->_session))) {
			kdDebug(7029) << "Session ID is being reused." << endl;
		} else {
			kdDebug(7029) << "Error attempting to reuse session." << endl;
			delete d->session;
			d->session = 0;
		}
	}

/*
	if (!setVerificationLogic()) {
		d->kossl->SSL_shutdown(d->m_ssl);
		d->kossl->SSL_free(d->m_ssl);
		d->m_ssl = 0;
		return -1;
	}
*/

	if (!d->lastInitTLS)
		d->kossl->SSL_set_options(d->m_ssl, SSL_OP_NO_TLSv1);

	d->kossl->SSL_set_options(d->m_ssl, SSL_OP_ALL);

	rc = d->kossl->SSL_set_fd(d->m_ssl, sock);
	if (rc == 0) {
		d->kossl->SSL_shutdown(d->m_ssl);
		d->kossl->SSL_free(d->m_ssl);
		d->m_ssl = 0;
		return rc;
	}

	rc = d->kossl->SSL_accept(d->m_ssl);
	if (rc == 1) {
		setConnectionInfo();
		setPeerInfo();
		kdDebug(7029) << "KSSL connected OK" << endl;
	} else {
		kdDebug(7029) << "KSSL accept failed - rc = " << rc << endl;
		kdDebug(7029) << "                      ERROR = "
			      << d->kossl->SSL_get_error(d->m_ssl, rc) << endl;
		d->kossl->SSL_shutdown(d->m_ssl);
		d->kossl->SSL_free(d->m_ssl);
		d->m_ssl = 0;
		return -1;
	}

	if (!d->kossl->SSL_session_reused(d->m_ssl)) {
		if (d->session) {
			kdDebug(7029) << "Session reuse failed.  New session used instead." << endl;
			delete d->session;
			d->session = 0L;
		}
	}

	if (!d->session) {
		SSL_SESSION *sess = d->kossl->SSL_get1_session(d->m_ssl);
		if (sess) {
			d->session = new KSSLSession;
			d->session->_session = sess;
		}
	}

return rc;
#else
return -1;
#endif
}


int KSSL::connect(int sock) {
#ifdef KSSL_HAVE_SSL
// kdDebug(7029) << "KSSL connect" << endl;
int rc;
	if (!m_bInit)
		return -1;
	d->m_ssl = d->kossl->SSL_new(d->m_ctx);
	if (!d->m_ssl)
		return -1;

	if (d->session) {
		if (static_cast<SSL_SESSION*>(d->session->_session)->sess_cert == 0)
		{
			kdDebug(7029) << "Can't reuse session, no certificate." << endl;
			delete d->session;
			d->session = 0;
		} else if (1 == d->kossl->SSL_set_session(d->m_ssl,
			static_cast<SSL_SESSION*>(d->session->_session))) {
			kdDebug(7029) << "Session ID is being reused." << endl;
		} else {
			kdDebug(7029) << "Error attempting to reuse session." << endl;
			delete d->session;
			d->session = 0;
		}
	}

/*
	if (!setVerificationLogic()) {
		d->kossl->SSL_shutdown(d->m_ssl);
		d->kossl->SSL_free(d->m_ssl);
		d->m_ssl = 0;
		return -1;
	}
*/

	if (!d->lastInitTLS)
		d->kossl->SSL_set_options(d->m_ssl, SSL_OP_NO_TLSv1);

	d->kossl->SSL_set_options(d->m_ssl, SSL_OP_ALL);

	rc = d->kossl->SSL_set_fd(d->m_ssl, sock);
	if (rc == 0) {
		d->kossl->SSL_shutdown(d->m_ssl);
		d->kossl->SSL_free(d->m_ssl);
		d->m_ssl = 0;
		return rc;
	}

connect_again:
	rc = d->kossl->SSL_connect(d->m_ssl);
	if (rc == 1) {
		setConnectionInfo();
		setPeerInfo();
		kdDebug(7029) << "KSSL connected OK" << endl;
	} else {
		int err = d->kossl->SSL_get_error(d->m_ssl, rc);
		if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
			// nonblocking - but we block anyways in connect() :)
			goto connect_again;
		} else {
			kdDebug(7029) << "KSSL connect failed - rc = "
				<< rc << endl;
			kdDebug(7029) << "                   ERROR = "
				<< err << endl;
			d->kossl->ERR_print_errors_fp(stderr);
			d->kossl->SSL_shutdown(d->m_ssl);
			d->kossl->SSL_free(d->m_ssl);
			d->m_ssl = 0;
			return -1;
		}
	}

	if (!d->kossl->SSL_session_reused(d->m_ssl)) {
		if (d->session) {
			kdDebug(7029) << "Session reuse failed.  New session used instead." << endl;
			delete d->session;
			d->session = 0L;
		}
	}

	if (!d->session) {
		SSL_SESSION *sess = d->kossl->SSL_get1_session(d->m_ssl);
		if (sess) {
			d->session = new KSSLSession;
			d->session->_session = sess;
		}
	}

return rc;
#else
return -1;
#endif
}


int KSSL::pending() {
#ifdef KSSL_HAVE_SSL
	if (!m_bInit)
		return -1;
return d->kossl->SSL_pending(d->m_ssl);
#else
return -1;
#endif
}


int KSSL::peek(void *buf, int len) {
#ifdef KSSL_HAVE_SSL
	if (!m_bInit)
		return -1;
	// FIXME: enhance to work the way read() does below, handling errors
return d->kossl->SSL_peek(d->m_ssl, buf, len);
#else
return -1;
#endif
}


int KSSL::read(void *buf, int len) {
#ifdef KSSL_HAVE_SSL
	int rc = 0;
	int maxIters = 10;

	if (!m_bInit)
		return -1;

read_again:
	rc = d->kossl->SSL_read(d->m_ssl, (char *)buf, len);
	if (rc <= 0) {
		int err = d->kossl->SSL_get_error(d->m_ssl, rc);

		if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
			kdDebug(7029) << "SSL read() returning 0: " << err << endl;
			if (maxIters-- > 0) {
				::usleep(20000); // 20ms sleep
				goto read_again;
			}
			return 0;
		}

		kdDebug(7029) << "SSL READ ERROR: " << err << endl;
		if (err != SSL_ERROR_NONE &&
		    err != SSL_ERROR_ZERO_RETURN && err != SSL_ERROR_SYSCALL) {
			rc = -1;      // OpenSSL returns 0 on error too
		}

//		else if (err == SSL_ERROR_ZERO_RETURN)
//			rc = 0;
	}
return rc;
#else
return -1;
#endif
}


int KSSL::write(const void *buf, int len) {
#ifdef KSSL_HAVE_SSL
	if (!m_bInit)
		return -1;

write_again:
	int rc = d->kossl->SSL_write(d->m_ssl, (const char *)buf, len);
	if (rc <= 0) {      // OpenSSL returns 0 on error too
		int err = d->kossl->SSL_get_error(d->m_ssl, rc);

		if (err == SSL_ERROR_WANT_WRITE) {
			::usleep(20000); // 20ms sleep
			goto write_again;
		}

		kdDebug(7029) << "SSL WRITE ERROR: " << err << endl;
		if (err != SSL_ERROR_NONE &&
		    err != SSL_ERROR_ZERO_RETURN && err != SSL_ERROR_SYSCALL)
			rc = -1;
	}

return rc;
#else
return -1;
#endif
}


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


#ifdef KSSL_HAVE_SSL
bool KSSL::m_bSSLWorks = true;
#else
bool KSSL::m_bSSLWorks = false;
#endif

bool KSSL::doesSSLWork() {
	return m_bSSLWorks;
}


void KSSL::setConnectionInfo() {
#ifdef KSSL_HAVE_SSL
SSL_CIPHER *sc;
char buf[1024];

	buf[0] = 0;  // for safety.
	sc = d->kossl->SSL_get_current_cipher(d->m_ssl);
	if (!sc) {
		kdDebug(7029) << "KSSL get current cipher failed - we're probably gonna crash!" << endl;
		return;
	}

	// set the number of bits, bits used
	m_ci.m_iCipherUsedBits = d->kossl->SSL_CIPHER_get_bits(sc, &(m_ci.m_iCipherBits));
	// set the cipher version
	m_ci.m_cipherVersion = d->kossl->SSL_CIPHER_get_version(sc);
	// set the cipher name
	m_ci.m_cipherName = d->kossl->SSL_CIPHER_get_name(sc);
	// set the cipher description
	m_ci.m_cipherDescription = d->kossl->SSL_CIPHER_description(sc, buf, 1023);

#endif
}


void KSSL::setPeerInfo() {
#ifdef KSSL_HAVE_SSL
	m_pi.setPeerHost(d->proxyPeer);
	m_pi.m_cert.setCert(d->kossl->SSL_get_peer_certificate(d->m_ssl));
	STACK_OF(X509) *xs = d->kossl->SSL_get_peer_cert_chain(d->m_ssl);
	if (xs)
		xs = sk_X509_dup(xs);   // Leak? 
	m_pi.m_cert.setChain((void *)xs);
#endif
}


KSSLConnectionInfo& KSSL::connectionInfo() {
	return m_ci;
}


// KDE 4: Make it const QString &
void KSSL::setPeerHost(QString realHost) {
	d->proxyPeer = realHost;
}

// deprecated
void KSSL::setProxyUse(bool, QString, int, QString) {
}


KSSLPeerInfo& KSSL::peerInfo() {
	return m_pi;
}


bool KSSL::setClientCertificate(KSSLPKCS12 *pkcs) {
#ifdef KSSL_HAVE_SSL
	if (!pkcs || !pkcs->getCertificate())
		return false;

int rc;
X509 *x = pkcs->getCertificate()->getCert();
EVP_PKEY *k = pkcs->getPrivateKey();

	if (!x || !k) return false;

	if (!pkcs->getCertificate()->x509V3Extensions().certTypeSSLClient())
		return false;

	rc = d->kossl->SSL_CTX_use_certificate(d->m_ctx, x);
	if (rc <= 0) {
		kdDebug(7029) << "KSSL - SSL_CTX_use_certificate failed.  rc = " << rc << endl;
		return false;
	}

	rc = d->kossl->SSL_CTX_use_PrivateKey(d->m_ctx, k);
	if (rc <= 0) {
		kdDebug(7029) << "KSSL - SSL_CTX_use_PrivateKey failed.  rc = " << rc << endl;
		return false;
	}

	return true;
#else
	return false;
#endif
}

#undef sk_dup

const KSSLSession* KSSL::session() const {
	return d->session;
}

bool KSSL::reusingSession() const {
#ifdef KSSL_HAVE_SSL
	return (d->m_ssl && d->kossl->SSL_session_reused(d->m_ssl));
#else
	return false;
#endif
}


