/* This file is part of the KDE project
 *
 * Copyright (C) 2000,2001 George Staikos <staikos@kde.org>
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif



#include <unistd.h>
#include <qstring.h>
#include <qfile.h>

#include "kssldefs.h"
#include "ksslcertificate.h"
#include "ksslcertchain.h"
#include "ksslutils.h"

#include <kstandarddirs.h>
#include <kmdcodec.h>
#include <klocale.h>
#include <qdatetime.h>
#include <ktempfile.h>

#include <sys/types.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

// this hack provided by Malte Starostik to avoid glibc/openssl bug
// on some systems
#ifdef HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/x509_vfy.h>
#include <openssl/pem.h>
#undef crypt
#endif

#include <kopenssl.h>
#include <qcstring.h>
#include <kdebug.h>
#include "ksslx509v3.h"



static char hv[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};


class KSSLCertificatePrivate {
public:
	KSSLCertificatePrivate() {
		kossl = KOSSL::self();
	}

	~KSSLCertificatePrivate() {
	}

	KSSLCertificate::KSSLValidation m_stateCache;
	bool m_stateCached;
	#ifdef HAVE_SSL
		X509 *m_cert;
	#endif
	KOSSL *kossl;
	KSSLCertChain _chain;
	KSSLX509V3 _extensions;
};

KSSLCertificate::KSSLCertificate() {
	d = new KSSLCertificatePrivate;
	d->m_stateCached = false;
	KGlobal::dirs()->addResourceType("kssl", KStandardDirs::kde_default("data") + "kssl");
	#ifdef HAVE_SSL
		d->m_cert = NULL;
	#endif
}


KSSLCertificate::KSSLCertificate(const KSSLCertificate& x) {
	d = new KSSLCertificatePrivate;
	d->m_stateCached = false;
	KGlobal::dirs()->addResourceType("kssl", KStandardDirs::kde_default("data") + "kssl");
	#ifdef HAVE_SSL
		d->m_cert = NULL;
		setCert(KOSSL::self()->X509_dup(const_cast<KSSLCertificate&>(x).getCert()));
		KSSLCertChain *c = x.d->_chain.replicate();
		setChain(c->rawChain());
		delete c;
	#endif
}



KSSLCertificate::~KSSLCertificate() {
#ifdef HAVE_SSL
	if (d->m_cert)
		d->kossl->X509_free(d->m_cert);
#endif
	delete d;
}


KSSLCertChain& KSSLCertificate::chain() {
	return d->_chain;
}


KSSLCertificate *KSSLCertificate::fromX509(X509 *x5) {
KSSLCertificate *n = NULL;
#ifdef HAVE_SSL
	if (x5) {
		n = new KSSLCertificate;
		n->setCert(KOSSL::self()->X509_dup(x5));
	}
#endif
return n;
}


KSSLCertificate *KSSLCertificate::fromString(QCString cert) {
KSSLCertificate *n = NULL;
#ifdef HAVE_SSL
	if (cert.length() == 0)
		return NULL;

	QByteArray qba, qbb = cert.copy();
	KCodecs::base64Decode(qbb, qba);
	unsigned char *qbap = reinterpret_cast<unsigned char *>(qba.data());
	X509 *x5c = KOSSL::self()->d2i_X509(NULL, &qbap, qba.size());
	if (!x5c) {
		return NULL;
	}

	n = new KSSLCertificate;
	n->setCert(x5c);
#endif
return n;
}



QString KSSLCertificate::getSubject() const {
QString rc = "";

#ifdef HAVE_SSL
	char *t = d->kossl->X509_NAME_oneline(d->kossl->X509_get_subject_name(d->m_cert), 0, 0);
	if (!t)
		return rc;
	rc = t;
	d->kossl->OPENSSL_free(t);
#endif
return rc;
}


QString KSSLCertificate::getSerialNumber() const {
QString rc = "";

#ifdef HAVE_SSL
	ASN1_INTEGER *aint = d->kossl->X509_get_serialNumber(d->m_cert);
	if (aint) {
		rc = ASN1_INTEGER_QString(aint);
		// d->kossl->ASN1_INTEGER_free(aint);   this makes the sig test fail
	}
#endif
return rc;
}


QString KSSLCertificate::getSignatureText() const {
QString rc = "";

#ifdef HAVE_SSL
char *s;
int n, i;

	i = d->kossl->OBJ_obj2nid(d->m_cert->sig_alg->algorithm);
	rc = i18n("Signature Algorithm: ");
	rc += (i == NID_undef)?i18n("Unknown"):d->kossl->OBJ_nid2ln(i);

	rc += "\n";
	rc += i18n("Signature Contents:");
	n = d->m_cert->signature->length;
	s = (char *)d->m_cert->signature->data;
	for (i = 0; i < n; i++) {
		if (i%20 != 0) rc += ":";
		else rc += "\n";
		rc.append(hv[(s[i]&0xf0)>>4]);
		rc.append(hv[s[i]&0x0f]);
	}

#endif

return rc;
}


QString KSSLCertificate::getMD5DigestText() const {
QString rc = "";

#ifdef HAVE_SSL
	unsigned int n;
	unsigned char md[EVP_MAX_MD_SIZE];

	if (!d->kossl->X509_digest(d->m_cert, d->kossl->EVP_md5(), md, &n)) {
		return rc;
	}

	for (unsigned int j = 0; j < n; j++) {
		if (j > 0)
			rc += ":";
		rc.append(hv[(md[j]&0xf0)>>4]);
		rc.append(hv[md[j]&0x0f]);
	}

#endif

return rc;
}



QString KSSLCertificate::getKeyType() const {
QString rc = "";

#ifdef HAVE_SSL
	EVP_PKEY *pkey = d->kossl->X509_get_pubkey(d->m_cert);
	if (pkey) {
		#ifndef NO_RSA
			if (pkey->type == EVP_PKEY_RSA)
				rc = "RSA";
			else
		#endif
		#ifndef NO_DSA
			if (pkey->type == EVP_PKEY_DSA)
				rc = "DSA";
			else
		#endif
				rc = "Unknown";
		d->kossl->EVP_PKEY_free(pkey);
	}
#endif

return rc;
}



QString KSSLCertificate::getPublicKeyText() const {
QString rc = "";
char *x = NULL;

#ifdef HAVE_SSL
	EVP_PKEY *pkey = d->kossl->X509_get_pubkey(d->m_cert);
	if (pkey) {
		rc = i18n("Unknown", "Unknown key algorithm");
		#ifndef NO_RSA
			if (pkey->type == EVP_PKEY_RSA) {
				rc = i18n("Key type: RSA (%1 bit)") + "\n";

				x = d->kossl->BN_bn2hex(pkey->pkey.rsa->n);
				rc += i18n("Modulus: ");
				rc = rc.arg(strlen(x)*4);
				for (unsigned int i = 0; i < strlen(x); i++) {
					if (i%40 != 0 && i%2 == 0)
						rc += ":";
					else if (i%40 == 0)
						rc += "\n";
					rc += x[i];
				}
				rc += "\n";
				d->kossl->OPENSSL_free(x);

				x = d->kossl->BN_bn2hex(pkey->pkey.rsa->e);
				rc += i18n("Exponent: 0x") + x + "\n";
				d->kossl->OPENSSL_free(x);
			}
		#endif
		#ifndef NO_DSA
			if (pkey->type == EVP_PKEY_DSA) {
				rc = i18n("Key type: DSA (%1 bit)") + "\n";

				x = d->kossl->BN_bn2hex(pkey->pkey.dsa->p);
				rc += i18n("Prime: ");
				// hack - this may not be always accurate
				rc = rc.arg(strlen(x)*4) ;
				for (unsigned int i = 0; i < strlen(x); i++) {
					if (i%40 != 0 && i%2 == 0)
						rc += ":";
					else if (i%40 == 0)
						rc += "\n";
					rc += x[i];
				}
				rc += "\n";
				d->kossl->OPENSSL_free(x);

				x = d->kossl->BN_bn2hex(pkey->pkey.dsa->q);
				rc += i18n("160 bit Prime Factor: ");
				for (unsigned int i = 0; i < strlen(x); i++) {
					if (i%40 != 0 && i%2 == 0)
						rc += ":";
					else if (i%40 == 0)
						rc += "\n";
					rc += x[i];
				}
				rc += "\n";
				d->kossl->OPENSSL_free(x);
	
				x = d->kossl->BN_bn2hex(pkey->pkey.dsa->g);
				rc += QString("g: ");
				for (unsigned int i = 0; i < strlen(x); i++) {
					if (i%40 != 0 && i%2 == 0)
						rc += ":";
					else if (i%40 == 0)
						rc += "\n";
					rc += x[i];
				}
				rc += "\n";
				d->kossl->OPENSSL_free(x);
	
				x = d->kossl->BN_bn2hex(pkey->pkey.dsa->pub_key);
				rc += i18n("Public Key: ");
				for (unsigned int i = 0; i < strlen(x); i++) {
					if (i%40 != 0 && i%2 == 0)
						rc += ":";
					else if (i%40 == 0)
						rc += "\n";
					rc += x[i];
				}
				rc += "\n";
				d->kossl->OPENSSL_free(x);
			}
		#endif
		d->kossl->EVP_PKEY_free(pkey);
	}
#endif

return rc;
}



QString KSSLCertificate::getIssuer() const {
QString rc = "";

#ifdef HAVE_SSL
	char *t = d->kossl->X509_NAME_oneline(d->kossl->X509_get_issuer_name(d->m_cert), 0, 0);

	if (!t)
		return rc;

	rc = t;
	d->kossl->OPENSSL_free(t);
#endif

return rc;
}

void KSSLCertificate::setChain(void *c) {
#ifdef HAVE_SSL
	d->_chain.setChain(c);
#endif
	d->m_stateCached = false;
	d->m_stateCache = KSSLCertificate::Unknown;
}

void KSSLCertificate::setCert(X509 *c) {
#ifdef HAVE_SSL
d->m_cert = c;
if (c) {
  	d->_extensions.flags = 0;
	d->kossl->X509_check_purpose(c, -1, 0);    // setup the fields (!!)

#if 0
	kdDebug(7029) << "---------------- Certificate ------------------" 
		      << endl;
	kdDebug(7029) << getSubject() << endl;
#endif

	for (int j = 0; j < d->kossl->X509_PURPOSE_get_count(); j++) {
		X509_PURPOSE *ptmp = d->kossl->X509_PURPOSE_get0(j);
		int id = d->kossl->X509_PURPOSE_get_id(ptmp);
		for (int ca = 0; ca < 2; ca++) {
			int idret = d->kossl->X509_check_purpose(c, id, ca);
			if (idret == 1 || idret == 2) {   // have it
//				kdDebug() << "PURPOSE: " << id << (ca?" CA":"") << endl;
				if (!ca)
					d->_extensions.flags |= (1L <<(id-1));
				else d->_extensions.flags |= (1L <<(16+id-1));
			} else {
				if (!ca)
					d->_extensions.flags &= ~(1L <<(id-1));
				else d->_extensions.flags &= ~(1L <<(16+id-1));
			}
		}
	}

#if 0
	kdDebug(7029) << "flags: " << QString::number(c->ex_flags, 2)
		      << "\nkeyusage: " << QString::number(c->ex_kusage, 2)
		      << "\nxkeyusage: " << QString::number(c->ex_xkusage, 2)
		      << "\nnscert: " << QString::number(c->ex_nscert, 2)
		      << endl;
	if (c->ex_flags & EXFLAG_KUSAGE)
		kdDebug(7029) << "     --- Key Usage extensions found" << endl;
        else
		kdDebug(7029) << "     --- Key Usage extensions NOT found" << endl;
	if (c->ex_flags & EXFLAG_XKUSAGE)
		kdDebug(7029) << "     --- Extended key usage extensions found" << endl;
        else
		kdDebug(7029) << "     --- Extended key usage extensions NOT found" << endl;
	if (c->ex_flags & EXFLAG_NSCERT)
		kdDebug(7029) << "     --- NS extensions found" << endl;
        else
		kdDebug(7029) << "     --- NS extensions NOT found" << endl;

        if (d->_extensions.certTypeSSLCA())
                kdDebug(7029) << "NOTE: this is an SSL CA file." << endl;
        else
                kdDebug(7029) << "NOTE: this is NOT an SSL CA file." << endl;
        if (d->_extensions.certTypeEmailCA())
                kdDebug(7029) << "NOTE: this is an EMAIL CA file." << endl;
        else
                kdDebug(7029) << "NOTE: this is NOT an EMAIL CA file." << endl;
        if (d->_extensions.certTypeCodeCA())
                kdDebug(7029) << "NOTE: this is a CODE CA file." << endl;
        else
                kdDebug(7029) << "NOTE: this is NOT a CODE CA file." << endl;
        if (d->_extensions.certTypeSSLClient())
                kdDebug(7029) << "NOTE: this is an SSL client." << endl;
        else
                kdDebug(7029) << "NOTE: this is NOT an SSL client." << endl;
        if (d->_extensions.certTypeSSLServer())
                kdDebug(7029) << "NOTE: this is an SSL server." << endl;
        else
                kdDebug(7029) << "NOTE: this is NOT an SSL server." << endl;
        if (d->_extensions.certTypeNSSSLServer())
                kdDebug(7029) << "NOTE: this is a NETSCAPE SSL server." << endl;
        else
                kdDebug(7029) << "NOTE: this is NOT a NETSCAPE SSL server." << endl;
        if (d->_extensions.certTypeSMIME())
                kdDebug(7029) << "NOTE: this is an SMIME certificate." << endl;
        else
                kdDebug(7029) << "NOTE: this is NOT an SMIME certificate." << endl;
        if (d->_extensions.certTypeSMIMEEncrypt())
                kdDebug(7029) << "NOTE: this is an SMIME encrypt cert." << endl;
        else
                kdDebug(7029) << "NOTE: this is NOT an SMIME encrypt cert." << endl;
        if (d->_extensions.certTypeSMIMESign())
                kdDebug(7029) << "NOTE: this is an SMIME sign cert." << endl;
        else
                kdDebug(7029) << "NOTE: this is NOT an SMIME sign cert." << endl;
        if (d->_extensions.certTypeCRLSign())
                kdDebug(7029) << "NOTE: this is a CRL signer." << endl;
        else
                kdDebug(7029) << "NOTE: this is NOT a CRL signer." << endl;

	kdDebug(7029) << "-----------------------------------------------" 
		      << endl;
#endif
}
#endif
d->m_stateCached = false;
d->m_stateCache = KSSLCertificate::Unknown;
}

X509 *KSSLCertificate::getCert() {
#ifdef HAVE_SSL
	return d->m_cert;
#endif
return 0;
}

// pull in the callback.  It's common across multiple files but we want
// it to be hidden.

#include "ksslcallback.c"


bool KSSLCertificate::isValid() {
	return (validate() == KSSLCertificate::Ok);
}


int KSSLCertificate::purposeToOpenSSL(KSSLCertificate::KSSLPurpose p) const {
int rc = 0;

	if (p & KSSLCertificate::SSLServer || p == KSSLCertificate::Any) {
		rc |= X509_PURPOSE_SSL_SERVER;
		rc |= X509_PURPOSE_NS_SSL_SERVER;
	}

	if (p & KSSLCertificate::SSLClient || p == KSSLCertificate::Any) {
		rc |= X509_PURPOSE_SSL_CLIENT;
	}

	if (p & KSSLCertificate::SMIMEEncrypt || p == KSSLCertificate::Any) {
		rc |= X509_PURPOSE_SMIME_ENCRYPT;
	}

	if (p & KSSLCertificate::SMIMESign || p == KSSLCertificate::Any) {
		rc |= X509_PURPOSE_SMIME_SIGN;
	}

return rc;	
}


// For backward compatibility
KSSLCertificate::KSSLValidation KSSLCertificate::validate() {
	return validate(KSSLCertificate::SSLServer);
}


//
// See apps/verify.c in OpenSSL for the source of most of this logic.
//

// CRL files?  we don't do that yet

KSSLCertificate::KSSLValidation KSSLCertificate::validate(KSSLCertificate::KSSLPurpose purpose) {

#ifdef HAVE_SSL
X509_STORE *certStore;
X509_LOOKUP *certLookup;
X509_STORE_CTX *certStoreCTX;
int rc = 0;

	if (!d->m_cert)
		return KSSLCertificate::Unknown;

	if (d->m_stateCached) {
		return d->m_stateCache;
	}

	QStringList qsl = KGlobal::dirs()->resourceDirs("kssl");

	if (qsl.isEmpty()) {
		return KSSLCertificate::NoCARoot;
	}

	KSSLCertificate::KSSLValidation ksslv = Unknown;

	for (QStringList::Iterator j = qsl.begin(); j != qsl.end(); ++j) {
		struct stat sb;
		QString _j = (*j)+"ca-bundle.crt";
		if (-1 == stat(_j.ascii(), &sb)) continue;

		certStore = d->kossl->X509_STORE_new();
		if (!certStore)
			return KSSLCertificate::Unknown;

		X509_STORE_set_verify_cb_func(certStore, X509Callback);

		certLookup = d->kossl->X509_STORE_add_lookup(certStore, d->kossl->X509_LOOKUP_file());
		if (!certLookup) {
			ksslv = KSSLCertificate::Unknown;
			d->kossl->X509_STORE_free(certStore);
			continue;
		}

		if (!d->kossl->X509_LOOKUP_load_file(certLookup, _j.ascii(), X509_FILETYPE_PEM)) {
			// error accessing directory and loading pems
			kdDebug(7029) << "KSSL couldn't read CA root: " 
					<< _j << endl;
			ksslv = KSSLCertificate::ErrorReadingRoot;
			d->kossl->X509_STORE_free(certStore);
			continue;
		}

		// This is the checking code
		certStoreCTX = d->kossl->X509_STORE_CTX_new();

		// this is a bad error - could mean no free memory.
		// This may be the wrong thing to do here
		if (!certStoreCTX) {
			kdDebug(7029) << "KSSL couldn't create an X509 store context." << endl;
			d->kossl->X509_STORE_free(certStore);
			continue;
		}

		d->kossl->X509_STORE_CTX_init(certStoreCTX, certStore, d->m_cert, NULL);
		if (d->_chain.isValid())
			d->kossl->X509_STORE_CTX_set_chain(certStoreCTX, (STACK_OF(X509)*)d->_chain.rawChain());

		//kdDebug(7029) << "KSSL setting CRL.............." << endl;
		// int X509_STORE_add_crl(X509_STORE *ctx, X509_CRL *x);

		d->kossl->X509_STORE_CTX_set_purpose(certStoreCTX, purposeToOpenSSL(purpose));

		certStoreCTX->error = X509_V_OK;
		rc = d->kossl->X509_verify_cert(certStoreCTX);
		int errcode = certStoreCTX->error;
		d->kossl->X509_STORE_CTX_free(certStoreCTX);
		d->kossl->X509_STORE_free(certStore);
		// end of checking code
		//

		ksslv = processError(errcode);

		//kdDebug(7029) << "KSSL Validation procedure RC: " 
		//		<< rc << endl;
		//kdDebug(7029) << "KSSL Validation procedure errcode: "
		//		<< errcode << endl;
		//kdDebug(7029) << "KSSL Validation procedure RESULTS: "
		//		<< ksslv << endl;

		if (ksslv != NoCARoot && ksslv != InvalidCA) {
			d->m_stateCached = true;
			d->m_stateCache = ksslv;
			break;
		}
	}

return (d->m_stateCache);

#endif
return NoSSL;
}



KSSLCertificate::KSSLValidation KSSLCertificate::revalidate() {
	d->m_stateCached = false;
	return validate();
}


KSSLCertificate::KSSLValidation KSSLCertificate::processError(int ec) {
KSSLCertificate::KSSLValidation rc;

rc = KSSLCertificate::Unknown;
#ifdef HAVE_SSL
	switch (ec) {
	case X509_V_OK:       // OK
		rc = KSSLCertificate::Ok;
	break;


	case X509_V_ERR_CERT_REJECTED:
		rc = KSSLCertificate::Rejected;
	break;


	case X509_V_ERR_CERT_UNTRUSTED:
		rc = KSSLCertificate::Untrusted;
	break;


	case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
	case X509_V_ERR_CERT_SIGNATURE_FAILURE:
	case X509_V_ERR_CRL_SIGNATURE_FAILURE:
	case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
	case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
		rc = KSSLCertificate::SignatureFailed;
	break;

	case X509_V_ERR_INVALID_CA:
	case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
	case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
	case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
		rc = KSSLCertificate::InvalidCA;
	break;


	case X509_V_ERR_INVALID_PURPOSE:
		rc = KSSLCertificate::InvalidPurpose;
	break;


	case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
	case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
		rc = KSSLCertificate::SelfSigned;
	break;


	case X509_V_ERR_CERT_REVOKED:
		rc = KSSLCertificate::Revoked;
	break;

	case X509_V_ERR_PATH_LENGTH_EXCEEDED:
		rc = KSSLCertificate::PathLengthExceeded;
	break;

	case X509_V_ERR_CERT_NOT_YET_VALID:
	case X509_V_ERR_CERT_HAS_EXPIRED:
	case X509_V_ERR_CRL_NOT_YET_VALID:
	case X509_V_ERR_CRL_HAS_EXPIRED:
	case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
	case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
	case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
	case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
		rc = KSSLCertificate::Expired;
		kdDebug(7029) << "KSSL apparently this is expired.  Not after: "
				<< getNotAfter() << endl;
	break;

	//case 1:
	case X509_V_ERR_APPLICATION_VERIFICATION:
	case X509_V_ERR_OUT_OF_MEM:
	case X509_V_ERR_UNABLE_TO_GET_CRL:
	case X509_V_ERR_CERT_CHAIN_TOO_LONG:
	default:
		rc = KSSLCertificate::Unknown;
	break;
}

d->m_stateCache = rc;
d->m_stateCached = true;
#endif
return rc;
}


QString KSSLCertificate::getNotBefore() const {
#ifdef HAVE_SSL
return ASN1_UTCTIME_QString(X509_get_notBefore(d->m_cert));
#else
return QString::null;
#endif
}


QString KSSLCertificate::getNotAfter() const {
#ifdef HAVE_SSL
return ASN1_UTCTIME_QString(X509_get_notAfter(d->m_cert));
#else
return QString::null;
#endif
}


QDateTime KSSLCertificate::getQDTNotBefore() const {
#ifdef HAVE_SSL
return ASN1_UTCTIME_QDateTime(X509_get_notBefore(d->m_cert), NULL);
#else
return QDateTime::currentDateTime();
#endif
}


QDateTime KSSLCertificate::getQDTNotAfter() const {
#ifdef HAVE_SSL
return ASN1_UTCTIME_QDateTime(X509_get_notAfter(d->m_cert), NULL);
#else
return QDateTime::currentDateTime();
#endif
}


int operator==(KSSLCertificate &x, KSSLCertificate &y) {
#ifndef HAVE_SSL
  return 1;
#else
  if (!KOSSL::self()->X509_cmp(x.getCert(), y.getCert())) return 1;
  return 0;
#endif
}


KSSLCertificate *KSSLCertificate::replicate() {
// The new certificate doesn't have the cached value.  It's probably
// better this way.  We can't anticipate every reason for doing this.
KSSLCertificate *newOne = new KSSLCertificate();
#ifdef HAVE_SSL
	newOne->setCert(d->kossl->X509_dup(getCert()));
	KSSLCertChain *c = d->_chain.replicate();
	newOne->setChain(c->rawChain());
	delete c;
#endif
return newOne;
}


QString KSSLCertificate::toString() {
return KCodecs::base64Encode(toDer());
}


QString KSSLCertificate::verifyText(KSSLValidation x) {
switch (x) {
case KSSLCertificate::Ok:
	return i18n("The certificate is valid.");
case KSSLCertificate::PathLengthExceeded:
case KSSLCertificate::ErrorReadingRoot:
case KSSLCertificate::NoCARoot:
	return i18n("Certificate signing authority root files could not be found so the certificate is not verified.");
case KSSLCertificate::InvalidCA:
	return i18n("Certificate signing authority is unknown or invalid.");
case KSSLCertificate::SelfSigned:
	return i18n("Certificate is self-signed and thus may not be trustworthy.");
case KSSLCertificate::Expired:
	return i18n("Certificate has expired.");
case KSSLCertificate::Revoked:
	return i18n("Certificate has been revoked.");
case KSSLCertificate::NoSSL:
	return i18n("SSL support was not found.");
case KSSLCertificate::Untrusted:
	return i18n("Signature is untrusted.");
case KSSLCertificate::SignatureFailed:
	return i18n("Signature test failed.");
case KSSLCertificate::Rejected:
case KSSLCertificate::InvalidPurpose:
	return i18n("Rejected, possibly due to an invalid purpose.");
case KSSLCertificate::PrivateKeyFailed:
	return i18n("Private key test failed.");
default:
break;
}

return i18n("The certificate is invalid.");
}


QByteArray KSSLCertificate::toDer() {
QByteArray qba;
#ifdef HAVE_SSL
unsigned int certlen = d->kossl->i2d_X509(getCert(), NULL);
// These should technically be unsigned char * but it doesn't matter
// for our purposes
char *cert = new char[certlen];
char *p = cert;
	// FIXME: return code!
	d->kossl->i2d_X509(getCert(), (unsigned char **)&p);

	// encode it into a QString
	qba.duplicate(cert, certlen);
	delete[] cert;
#endif
return qba;
}



QByteArray KSSLCertificate::toPem() {
QByteArray qba;
QString thecert = toString();
const char *header = "-----BEGIN CERTIFICATE-----\n";
const char *footer = "-----END CERTIFICATE-----\n";

	// We just do base64 on the ASN1
	//  64 character lines  (unpadded)
	unsigned int xx = thecert.length() - 1;
	for (unsigned int i = 0; i < xx/64; i++) {
		thecert.insert(64*(i+1)+i, '\n');
	}

	thecert.prepend(header);

	if (thecert[thecert.length()-1] != '\n')
		thecert += "\n";

	thecert.append(footer);

	qba.duplicate(thecert.local8Bit(), thecert.length());
return qba;
}


#define NETSCAPE_CERT_HDR     "certificate"

// what a piece of crap this is
QByteArray KSSLCertificate::toNetscape() {
QByteArray qba;
#ifdef HAVE_SSL
ASN1_HEADER ah;
ASN1_OCTET_STRING os;
KTempFile ktf;

	os.data = (unsigned char *)NETSCAPE_CERT_HDR;
	os.length = strlen(NETSCAPE_CERT_HDR);
	ah.header = &os;
	ah.data = (char *)getCert();
	ah.meth = d->kossl->X509_asn1_meth();

	d->kossl->ASN1_i2d_fp(ktf.fstream(),(unsigned char *)&ah);

	ktf.close();

	QFile qf(ktf.name());
	qf.open(IO_ReadOnly);
	char *buf = new char[qf.size()];
	qf.readBlock(buf, qf.size());
	qba.duplicate(buf, qf.size());
	qf.close();
	delete[] buf;

	ktf.unlink();

#endif
return qba;
}



QString KSSLCertificate::toText() {
QString text;
#ifdef HAVE_SSL
KTempFile ktf;

	d->kossl->X509_print(ktf.fstream(), getCert());
	ktf.close();

	QFile qf(ktf.name());
	qf.open(IO_ReadOnly);
	char *buf = new char[qf.size()+1];
	qf.readBlock(buf, qf.size());
	buf[qf.size()] = 0;
	text = buf;
	delete[] buf;
	qf.close();
	ktf.unlink();
#endif
return text;
}


bool KSSLCertificate::setCert(QString& cert) {
#ifdef HAVE_SSL
QByteArray qba, qbb = cert.local8Bit().copy();
	KCodecs::base64Decode(qbb, qba);
	unsigned char *qbap = reinterpret_cast<unsigned char *>(qba.data());
	X509 *x5c = KOSSL::self()->d2i_X509(NULL, &qbap, qba.size());
	if (x5c) {
		setCert(x5c);
		return true;
	}
#endif
return false;
}


KSSLX509V3& KSSLCertificate::x509V3Extensions() {
return d->_extensions;
}


bool KSSLCertificate::isSigner() {
return d->_extensions.certTypeCA();
}


QDataStream& operator<<(QDataStream& s, const KSSLCertificate& r) {
QStringList qsl;
QPtrList<KSSLCertificate> cl = const_cast<KSSLCertificate&>(r).chain().getChain();

	for (KSSLCertificate *c = cl.first(); c != 0; c = cl.next()) {
		qsl << c->toString();
	}

	cl.setAutoDelete(true);

	s << const_cast<KSSLCertificate&>(r).toString() << qsl;

return s;
}


QDataStream& operator>>(QDataStream& s, KSSLCertificate& r) {
QStringList qsl;
QString cert;

s >> cert >> qsl;

	if (r.setCert(cert) && !qsl.isEmpty())
		r.chain().setChain(qsl);

return s;
}



