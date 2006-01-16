/* This file is part of the KDE project
 *
 * Copyright (C) 2003 Stefan Rompf <sux@loplof.de>
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


#include <q3ptrlist.h>
#include <qstring.h>
#include <kdebug.h>

#include "kopenssl.h"
#include "ksslcertificate.h"
#include "ksslpkcs12.h"
#include "ksmimecrypto.h"

// this hack provided by Malte Starostik to avoid glibc/openssl bug
// on some systems
#ifdef KSSL_HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/err.h>
#undef crypt
#endif


// forward included macros to KOpenSSLProxy
#define sk_new kossl->sk_new
#define sk_free kossl->sk_free
#define sk_push kossl->sk_push
#define sk_value kossl->sk_value
#define sk_num kossl->sk_num
#define BIO_ctrl kossl->BIO_ctrl


#ifdef KSSL_HAVE_SSL
static const char eot = 0;

class KSMIMECryptoPrivate {
    KOpenSSLProxy *kossl;

public:
    KSMIMECryptoPrivate(KOpenSSLProxy *kossl);


    STACK_OF(X509) *certsToX509(Q3PtrList<KSSLCertificate> &certs);

    KSMIMECrypto::rc signMessage(BIO *clearText,
				 BIO *cipherText,
				 KSSLPKCS12 &privKey, Q3PtrList<KSSLCertificate> &certs,
				 bool detached);

    KSMIMECrypto::rc encryptMessage(BIO *clearText,
				    BIO *cipherText, KSMIMECrypto::algo algorithm,
				    Q3PtrList<KSSLCertificate> &recip);

    KSMIMECrypto::rc checkSignature(BIO *clearText,
				    BIO *signature, bool detached,
				    Q3PtrList<KSSLCertificate> &recip);
    
    KSMIMECrypto::rc decryptMessage(BIO *cipherText,
				    BIO *clearText,
				    KSSLPKCS12 &privKey);
    
    void MemBIOToQByteArray(BIO *src, QByteArray &dest);

    KSMIMECrypto::rc sslErrToRc(void);
};


KSMIMECryptoPrivate::KSMIMECryptoPrivate(KOpenSSLProxy *kossl): kossl(kossl) {
}


STACK_OF(X509) *KSMIMECryptoPrivate::certsToX509(Q3PtrList<KSSLCertificate> &certs) {
    STACK_OF(X509) *x509 = sk_new(NULL);
    KSSLCertificate *cert = certs.first();
    while(cert) {
	sk_X509_push(x509, cert->getCert());
	cert = certs.next();
    }
    return x509;
}


KSMIMECrypto::rc KSMIMECryptoPrivate::signMessage(BIO *clearText,
						  BIO *cipherText,
						  KSSLPKCS12 &privKey, Q3PtrList<KSSLCertificate> &certs,
						  bool detached) {

    STACK_OF(X509) *other = NULL;
    KSMIMECrypto::rc rc;
    int flags = detached?PKCS7_DETACHED:0;

    if (certs.count()) other = certsToX509(certs);

    PKCS7 *p7 = kossl->PKCS7_sign(privKey.getCertificate()->getCert(), privKey.getPrivateKey(),
				  other, clearText, flags);

    if (other) sk_X509_free(other);

    if (!p7) return sslErrToRc();

    if (kossl->i2d_PKCS7_bio(cipherText, p7)) {
	rc = KSMIMECrypto::KSC_R_OK;
    } else {
	rc = sslErrToRc();
    }

    kossl->PKCS7_free(p7);

    return rc;
}

KSMIMECrypto::rc KSMIMECryptoPrivate::encryptMessage(BIO *clearText,
						     BIO *cipherText, KSMIMECrypto::algo algorithm,
						     Q3PtrList<KSSLCertificate> &recip) {
    EVP_CIPHER *cipher = NULL;
    KSMIMECrypto::rc rc;
    switch(algorithm) {
	case KSMIMECrypto::KSC_C_DES3_CBC:
	    cipher = kossl->EVP_des_ede3_cbc();
	    break;
	case KSMIMECrypto::KSC_C_RC2_CBC_128:
	    cipher = kossl->EVP_rc2_cbc();
	    break;
	case KSMIMECrypto::KSC_C_RC2_CBC_64:
	    cipher = kossl->EVP_rc2_64_cbc();
	    break;
	case KSMIMECrypto::KSC_C_DES_CBC:
	    cipher = kossl->EVP_des_cbc();
	    break;
	case KSMIMECrypto::KSC_C_RC2_CBC_40:
	    cipher = kossl->EVP_rc2_40_cbc();
	    break;
    }
    if (!cipher) return KSMIMECrypto::KSC_R_NOCIPHER;

    STACK_OF(X509) *certs = certsToX509(recip);

    PKCS7 *p7 = kossl->PKCS7_encrypt(certs, clearText, cipher, 0);

    sk_X509_free(certs);

    if (!p7) return sslErrToRc();

    if (kossl->i2d_PKCS7_bio(cipherText, p7)) {
	rc = KSMIMECrypto::KSC_R_OK;
    } else {
	rc = sslErrToRc();
    }

    kossl->PKCS7_free(p7);

    return rc;
}


KSMIMECrypto::rc KSMIMECryptoPrivate::checkSignature(BIO *clearText,
						     BIO *signature, bool detached,
						     Q3PtrList<KSSLCertificate> &recip) {
    
    PKCS7 *p7 = kossl->d2i_PKCS7_bio(signature, NULL);
    KSMIMECrypto::rc rc = KSMIMECrypto::KSC_R_OTHER;

    if (!p7) return sslErrToRc();

    BIO *in;
    BIO *out;
    if (detached) {
	in = clearText;
	out = NULL;
    } else {
	in = NULL;
	out = clearText;
    }

    X509_STORE *dummystore = kossl->X509_STORE_new();
    if (kossl->PKCS7_verify(p7, NULL, dummystore, in, out, PKCS7_NOVERIFY)) {
	STACK_OF(X509) *signers = kossl->PKCS7_get0_signers(p7, 0, PKCS7_NOVERIFY);
	int num = sk_X509_num(signers);

	for(int n=0; n<num; n++) {
	    KSSLCertificate *signer = KSSLCertificate::fromX509(sk_X509_value(signers, n));
	    recip.append(signer);
	}

	sk_X509_free(signers);
	rc = KSMIMECrypto::KSC_R_OK;
    } else {
	rc = sslErrToRc();
    }

    kossl->X509_STORE_free(dummystore);
    kossl->PKCS7_free(p7);

    return rc;
}


KSMIMECrypto::rc KSMIMECryptoPrivate::decryptMessage(BIO *cipherText,
						     BIO *clearText,
						     KSSLPKCS12 &privKey) {
    
    PKCS7 *p7 = kossl->d2i_PKCS7_bio(cipherText, NULL);
    KSMIMECrypto::rc rc;

    if (!p7) return sslErrToRc();

    if (kossl->PKCS7_decrypt(p7, privKey.getPrivateKey(), privKey.getCertificate()->getCert(), 
			     clearText, 0)) {
	rc = KSMIMECrypto::KSC_R_OK;
    } else {
	rc = sslErrToRc();
    }

    kossl->PKCS7_free(p7);

    return rc;
}


void KSMIMECryptoPrivate::MemBIOToQByteArray(BIO *src, QByteArray &dest) {
    char *buf;
    long len = BIO_get_mem_data(src, &buf);
    dest = QByteArray(buf, len);
    /* Now this goes quite a bit into openssl internals.
       We assume that openssl uses malloc() (it does in
       default config) and rip out the buffer.
    */
    reinterpret_cast<BUF_MEM *>(src->ptr)->data = NULL;
}

    
KSMIMECrypto::rc KSMIMECryptoPrivate::sslErrToRc(void) {
    unsigned long cerr = kossl->ERR_get_error();

    // To be completed and possibly fixed

    switch(ERR_GET_REASON(cerr)) {
	case ERR_R_MALLOC_FAILURE:
	    return KSMIMECrypto::KSC_R_NOMEM;
    }

    switch(ERR_GET_LIB(cerr)) {
	case ERR_LIB_PKCS7:
	    switch(ERR_GET_REASON(cerr)) {	
		case PKCS7_R_WRONG_CONTENT_TYPE:
		case PKCS7_R_NO_CONTENT:
		case PKCS7_R_NO_SIGNATURES_ON_DATA:
		    return KSMIMECrypto::KSC_R_FORMAT;
		    break;
		case PKCS7_R_PRIVATE_KEY_DOES_NOT_MATCH_CERTIFICATE:
		case PKCS7_R_DECRYPT_ERROR: // Hmm?
		    return KSMIMECrypto::KSC_R_WRONGKEY;
		    break;
		case PKCS7_R_DIGEST_FAILURE:
		    return KSMIMECrypto::KSC_R_VERIFY;
		default:
		    break;
	    }
	    break;
	default:
	    break;
    }

    kdDebug(7029) <<"KSMIMECrypto: uncaught error " <<ERR_GET_LIB(cerr)
		  <<" " <<ERR_GET_REASON(cerr) <<endl;
    return KSMIMECrypto::KSC_R_OTHER;
}    
#endif


KSMIMECrypto::KSMIMECrypto() {
#ifdef KSSL_HAVE_SSL
    kossl = KOpenSSLProxy::self();
    priv = new KSMIMECryptoPrivate(kossl);
    if (!kossl->hasLibCrypto()) kossl = 0L;
#else
    kossl = 0L;
	priv = 0L;
#endif
}


KSMIMECrypto::~KSMIMECrypto() {
#ifdef KSSL_HAVE_SSL
    delete priv;
#endif
}


KSMIMECrypto::rc KSMIMECrypto::signMessage(const QByteArray &clearText,
					   QByteArray &cipherText,
					   const KSSLPKCS12 &privKey,
					   const Q3PtrList<KSSLCertificate> &certs,
					   bool detached) {
#ifdef KSSL_HAVE_SSL
    if (!kossl) return KSC_R_NO_SSL;
    BIO *in = kossl->BIO_new_mem_buf((char *)clearText.data(), clearText.size());
    BIO *out = kossl->BIO_new(kossl->BIO_s_mem());

    rc rc = priv->signMessage(in, out,
			      const_cast<KSSLPKCS12 &>(privKey),
			      const_cast<Q3PtrList<KSSLCertificate> &>(certs),
			      detached);

    if (!rc) priv->MemBIOToQByteArray(out, cipherText);

    kossl->BIO_free(out);
    kossl->BIO_free(in);

    return rc;
#else
    return KSC_R_NO_SSL;
#endif
}


KSMIMECrypto::rc KSMIMECrypto::checkDetachedSignature(const QByteArray &clearText,
						      const QByteArray &signature,
						      Q3PtrList<KSSLCertificate> &foundCerts) {
#ifdef KSSL_HAVE_SSL
    if (!kossl) return KSC_R_NO_SSL;
    BIO *txt = kossl->BIO_new_mem_buf((char *)clearText.data(), clearText.length());
    BIO *sig = kossl->BIO_new_mem_buf((char *)signature.data(), signature.size());

    rc rc = priv->checkSignature(txt, sig, true, foundCerts);

    kossl->BIO_free(sig);
    kossl->BIO_free(txt);

    return rc;
#else
    return KSC_R_NO_SSL;
#endif
}


KSMIMECrypto::rc KSMIMECrypto::checkOpaqueSignature(const QByteArray &signedText,
						    QByteArray &clearText,
						    Q3PtrList<KSSLCertificate> &foundCerts) {
#ifdef KSSL_HAVE_SSL
    if (!kossl) return KSC_R_NO_SSL;

    BIO *in = kossl->BIO_new_mem_buf((char *)signedText.data(), signedText.size());
    BIO *out = kossl->BIO_new(kossl->BIO_s_mem());
   
    rc rc = priv->checkSignature(out, in, false, foundCerts);

    kossl->BIO_write(out, &eot, 1);
    priv->MemBIOToQByteArray(out, clearText);

    kossl->BIO_free(out);
    kossl->BIO_free(in);

    return rc;
#else
    return KSC_R_NO_SSL;
#endif
}


KSMIMECrypto::rc KSMIMECrypto::encryptMessage(const QByteArray &clearText,
					      QByteArray &cipherText,
					      algo algorithm,
					      const Q3PtrList<KSSLCertificate> &recip) {
#ifdef KSSL_HAVE_SSL
    if (!kossl) return KSC_R_NO_SSL;

    BIO *in = kossl->BIO_new_mem_buf((char *)clearText.data(), clearText.size());
    BIO *out = kossl->BIO_new(kossl->BIO_s_mem());

    rc rc = priv->encryptMessage(in,out,algorithm,
				 const_cast< Q3PtrList<KSSLCertificate> &>(recip));

    if (!rc) priv->MemBIOToQByteArray(out, cipherText);

    kossl->BIO_free(out);
    kossl->BIO_free(in);

    return rc;
#else
    return KSC_R_NO_SSL;
#endif
}


KSMIMECrypto::rc KSMIMECrypto::decryptMessage(const QByteArray &cipherText,
					      QByteArray &clearText,
					      const KSSLPKCS12 &privKey) {
#ifdef KSSL_HAVE_SSL
    if (!kossl) return KSC_R_NO_SSL;

    BIO *in = kossl->BIO_new_mem_buf((char *)cipherText.data(), cipherText.size());
    BIO *out = kossl->BIO_new(kossl->BIO_s_mem());

    rc rc = priv->decryptMessage(in,out,
				 const_cast<KSSLPKCS12 &>(privKey));

    kossl->BIO_write(out, &eot, 1);
    priv->MemBIOToQByteArray(out, clearText);

    kossl->BIO_free(out);
    kossl->BIO_free(in);

    return rc;
#else
    return KSC_R_NO_SSL;
#endif
}
