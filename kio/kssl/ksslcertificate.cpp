/* This file is part of the KDE project
 *
 * Copyright (C) 2000-2003 George Staikos <staikos@kde.org>
 *               2008 Richard Hartmann <richih-kde@net.in.tum.de>
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

#include "ksslcertificate.h"

#include <config.h>
#include <ksslconfig.h>



#include <unistd.h>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QFile>

#include "kssldefs.h"
#include "ksslcertchain.h"
#include "ksslutils.h"

#include <kstandarddirs.h>
#include <kcodecs.h>
#include <kde_file.h>
#include <klocale.h>
#include <QtCore/QDate>
#include <ktemporaryfile.h>

#include <sys/types.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

// this hack provided by Malte Starostik to avoid glibc/openssl bug
// on some systems
#ifdef KSSL_HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/x509_vfy.h>
#include <openssl/pem.h>
#undef crypt
#endif

#include <kopenssl.h>
#include <kdebug.h>
#include "ksslx509v3.h"



static char hv[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};


class KSSLCertificatePrivate {
public:
    KSSLCertificatePrivate() {
        kossl = KOSSL::self();
        _lastPurpose = KSSLCertificate::None;
    }

    ~KSSLCertificatePrivate() {
    }

    KSSLCertificate::KSSLValidation m_stateCache;
    bool m_stateCached;
    #ifdef KSSL_HAVE_SSL
        X509 *m_cert;
    #endif
    KOSSL *kossl;
    KSSLCertChain _chain;
    KSSLX509V3 _extensions;
    KSSLCertificate::KSSLPurpose _lastPurpose;
};

KSSLCertificate::KSSLCertificate() {
    d = new KSSLCertificatePrivate;
    d->m_stateCached = false;
    KGlobal::dirs()->addResourceType("kssl", "data", "kssl");
    #ifdef KSSL_HAVE_SSL
        d->m_cert = NULL;
    #endif
}


KSSLCertificate::KSSLCertificate(const KSSLCertificate& x) {
    d = new KSSLCertificatePrivate;
    d->m_stateCached = false;
    KGlobal::dirs()->addResourceType("kssl", "data", "kssl");
    #ifdef KSSL_HAVE_SSL
        d->m_cert = NULL;
        setCert(KOSSL::self()->X509_dup(const_cast<KSSLCertificate&>(x).getCert()));
        KSSLCertChain *c = x.d->_chain.replicate();
        setChain(c->rawChain());
        delete c;
    #endif
}



KSSLCertificate::~KSSLCertificate() {
#ifdef KSSL_HAVE_SSL
    if (d->m_cert) {
        d->kossl->X509_free(d->m_cert);
    }
#endif
    delete d;
}


KSSLCertChain& KSSLCertificate::chain() {
    return d->_chain;
}


KSSLCertificate *KSSLCertificate::fromX509(X509 *x5) {
    KSSLCertificate *n = NULL;
#ifdef KSSL_HAVE_SSL
    if (x5) {
        n = new KSSLCertificate;
        n->setCert(KOSSL::self()->X509_dup(x5));
    }
#endif
    return n;
}


KSSLCertificate *KSSLCertificate::fromString(const QByteArray &cert) {
    KSSLCertificate *n = NULL;
#ifdef KSSL_HAVE_SSL
    if (cert.isEmpty()) {
        return NULL;
    }

    QByteArray qba = QByteArray::fromBase64(cert);
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

#ifdef KSSL_HAVE_SSL
    char *t = d->kossl->X509_NAME_oneline(d->kossl->X509_get_subject_name(d->m_cert), 0, 0);
    if (!t) {
        return rc;
    }
    rc = t;
    d->kossl->OPENSSL_free(t);
#endif
    return rc;
}


QString KSSLCertificate::getSerialNumber() const {
    QString rc = "";

#ifdef KSSL_HAVE_SSL
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

#ifdef KSSL_HAVE_SSL
    char *s;
    int n, i;

    i = d->kossl->OBJ_obj2nid(d->m_cert->sig_alg->algorithm);
    rc = i18n("Signature Algorithm: ");
    rc += (i == NID_undef)?i18n("Unknown"):QString(d->kossl->OBJ_nid2ln(i));

    rc += '\n';
    rc += i18n("Signature Contents:");
    n = d->m_cert->signature->length;
    s = (char *)d->m_cert->signature->data;
    for (i = 0; i < n; ++i) {
        if (i%20 != 0) {
            rc += ':';
        }
        else {
            rc += '\n';
        }
        rc.append(QChar(hv[(s[i]&0xf0)>>4]));
        rc.append(QChar(hv[s[i]&0x0f]));
    }

#endif

    return rc;
}


void KSSLCertificate::getEmails(QStringList &to) const {
    to.clear();
#ifdef KSSL_HAVE_SSL
    if (!d->m_cert) {
        return;
    }

    STACK *s = d->kossl->X509_get1_email(d->m_cert);
    if (s) {
        for(int n=0; n < s->num; n++) {
            to.append(d->kossl->sk_value(s,n));
        }
        d->kossl->X509_email_free(s);
    }
#endif
}


QString KSSLCertificate::getKDEKey() const {
    return getSubject() + " (" + getMD5DigestText() + ')';
}


QString KSSLCertificate::getMD5DigestFromKDEKey(const QString &k) {
    QString rc;
    int pos = k.lastIndexOf('(');
    if (pos != -1) {
        unsigned int len = k.length();
        if (k.at(len-1) == ')') {
            rc = k.mid(pos+1, len-pos-2);
        }
    }
    return rc;
}


QString KSSLCertificate::getMD5DigestText() const {
QString rc = "";

#ifdef KSSL_HAVE_SSL
    unsigned int n;
    unsigned char md[EVP_MAX_MD_SIZE];

    if (!d->kossl->X509_digest(d->m_cert, d->kossl->EVP_md5(), md, &n)) {
        return rc;
    }

    for (unsigned int j = 0; j < n; j++) {
        if (j > 0) {
            rc += ':';
        }
        rc.append(QChar(hv[(md[j]&0xf0)>>4]));
        rc.append(QChar(hv[md[j]&0x0f]));
    }

#endif

    return rc;
}



QString KSSLCertificate::getMD5Digest() const {
QString rc = "";

#ifdef KSSL_HAVE_SSL
    unsigned int n;
    unsigned char md[EVP_MAX_MD_SIZE];

    if (!d->kossl->X509_digest(d->m_cert, d->kossl->EVP_md5(), md, &n)) {
        return rc;
    }

    for (unsigned int j = 0; j < n; j++) {
        rc.append(QLatin1Char(hv[(md[j]&0xf0)>>4]));
        rc.append(QLatin1Char(hv[md[j]&0x0f]));
    }

#endif

    return rc;
}



QString KSSLCertificate::getKeyType() const {
QString rc = "";

#ifdef KSSL_HAVE_SSL
    EVP_PKEY *pkey = d->kossl->X509_get_pubkey(d->m_cert);
    if (pkey) {
        #ifndef NO_RSA
            if (pkey->type == EVP_PKEY_RSA) {
                rc = "RSA";
            }
            else
        #endif
        #ifndef NO_DSA
            if (pkey->type == EVP_PKEY_DSA) {
                rc = "DSA";
            }
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

#ifdef KSSL_HAVE_SSL
    EVP_PKEY *pkey = d->kossl->X509_get_pubkey(d->m_cert);
    if (pkey) {
        rc = i18nc("Unknown", "Unknown key algorithm");
        #ifndef NO_RSA
            if (pkey->type == EVP_PKEY_RSA) {
                x = d->kossl->BN_bn2hex(pkey->pkey.rsa->n);
                rc = i18n("Key type: RSA (%1 bit)", strlen(x)*4) + '\n';

                rc += i18n("Modulus: ");
                for (unsigned int i = 0; i < strlen(x); i++) {
                    if (i%40 != 0 && i%2 == 0) {
                        rc += ':';
                    }
                    else if (i%40 == 0) {
                        rc += '\n';
                    }
                    rc += x[i];
                }
                rc += '\n';
                d->kossl->OPENSSL_free(x);

                x = d->kossl->BN_bn2hex(pkey->pkey.rsa->e);
                rc += i18n("Exponent: 0x") + QLatin1String(x) +
                  QLatin1String("\n");
                d->kossl->OPENSSL_free(x);
            }
        #endif
        #ifndef NO_DSA
            if (pkey->type == EVP_PKEY_DSA) {
                x = d->kossl->BN_bn2hex(pkey->pkey.dsa->p);
                // hack - this may not be always accurate
                rc = i18n("Key type: DSA (%1 bit)", strlen(x)*4) + '\n';

                rc += i18n("Prime: ");
                for (unsigned int i = 0; i < strlen(x); i++) {
                    if (i%40 != 0 && i%2 == 0) {
                        rc += ':';
                    }
                    else if (i%40 == 0) {
                        rc += '\n';
                    }
                    rc += x[i];
                }
                rc += '\n';
                d->kossl->OPENSSL_free(x);

                x = d->kossl->BN_bn2hex(pkey->pkey.dsa->q);
                rc += i18n("160 bit prime factor: ");
                for (unsigned int i = 0; i < strlen(x); i++) {
                    if (i%40 != 0 && i%2 == 0) {
                        rc += ':';
                    }
                    else if (i%40 == 0) {
                        rc += '\n';
                    }
                    rc += x[i];
                }
                rc += '\n';
                d->kossl->OPENSSL_free(x);

                x = d->kossl->BN_bn2hex(pkey->pkey.dsa->g);
                rc += QString("g: ");
                for (unsigned int i = 0; i < strlen(x); i++) {
                    if (i%40 != 0 && i%2 == 0) {
                        rc += ':';
                    }
                    else if (i%40 == 0) {
                        rc += '\n';
                    }
                    rc += x[i];
                }
                rc += '\n';
                d->kossl->OPENSSL_free(x);

                x = d->kossl->BN_bn2hex(pkey->pkey.dsa->pub_key);
                rc += i18n("Public key: ");
                for (unsigned int i = 0; i < strlen(x); i++) {
                    if (i%40 != 0 && i%2 == 0) {
                        rc += ':';
                    }
                    else if (i%40 == 0) {
                        rc += '\n';
                    }
                    rc += x[i];
                }
                rc += '\n';
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

#ifdef KSSL_HAVE_SSL
    char *t = d->kossl->X509_NAME_oneline(d->kossl->X509_get_issuer_name(d->m_cert), 0, 0);

    if (!t) {
        return rc;
    }

    rc = t;
    d->kossl->OPENSSL_free(t);
#endif

    return rc;
}

void KSSLCertificate::setChain(void *c) {
#ifdef KSSL_HAVE_SSL
    d->_chain.setChain(c);
#endif
    d->m_stateCached = false;
    d->m_stateCache = KSSLCertificate::Unknown;
}

void KSSLCertificate::setCert(X509 *c) {
#ifdef KSSL_HAVE_SSL
    d->m_cert = c;
    if (c) {
        d->_extensions.flags = 0;
        d->kossl->X509_check_purpose(c, -1, 0);    // setup the fields (!!)

#if 0
        kDebug(7029) << "---------------- Certificate ------------------"
                  << endl;
        kDebug(7029) << getSubject();
#endif

        for (int j = 0; j < d->kossl->X509_PURPOSE_get_count(); j++) {
            X509_PURPOSE *ptmp = d->kossl->X509_PURPOSE_get0(j);
            int id = d->kossl->X509_PURPOSE_get_id(ptmp);
            for (int ca = 0; ca < 2; ca++) {
                int idret = d->kossl->X509_check_purpose(c, id, ca);
                if (idret == 1 || idret == 2) {   // have it
                    // kDebug() << "PURPOSE: " << id << (ca?" CA":"");
                    if (!ca) {
                        d->_extensions.flags |= (1L <<(id-1));
                    }
                    else d->_extensions.flags |= (1L <<(16+id-1));
                } else {
                    if (!ca) {
                        d->_extensions.flags &= ~(1L <<(id-1));
                    }
                    else d->_extensions.flags &= ~(1L <<(16+id-1));
                }
            }
        }

#if 0
        kDebug(7029) << "flags: " << QString::number(c->ex_flags, 2)
                  << "\nkeyusage: " << QString::number(c->ex_kusage, 2)
                  << "\nxkeyusage: " << QString::number(c->ex_xkusage, 2)
                  << "\nnscert: " << QString::number(c->ex_nscert, 2)
                  << endl;
        if (c->ex_flags & EXFLAG_KUSAGE)
            kDebug(7029) << "     --- Key Usage extensions found";
        else kDebug(7029) << "     --- Key Usage extensions NOT found";

        if (c->ex_flags & EXFLAG_XKUSAGE)
            kDebug(7029) << "     --- Extended key usage extensions found";
        else kDebug(7029) << "     --- Extended key usage extensions NOT found";

        if (c->ex_flags & EXFLAG_NSCERT)
            kDebug(7029) << "     --- NS extensions found";
        else kDebug(7029) << "     --- NS extensions NOT found";

        if (d->_extensions.certTypeSSLCA())
            kDebug(7029) << "NOTE: this is an SSL CA file.";
        else kDebug(7029) << "NOTE: this is NOT an SSL CA file.";

        if (d->_extensions.certTypeEmailCA())
            kDebug(7029) << "NOTE: this is an EMAIL CA file.";
        else kDebug(7029) << "NOTE: this is NOT an EMAIL CA file.";

        if (d->_extensions.certTypeCodeCA())
            kDebug(7029) << "NOTE: this is a CODE CA file.";
        else kDebug(7029) << "NOTE: this is NOT a CODE CA file.";

        if (d->_extensions.certTypeSSLClient())
            kDebug(7029) << "NOTE: this is an SSL client.";
        else kDebug(7029) << "NOTE: this is NOT an SSL client.";

        if (d->_extensions.certTypeSSLServer())
            kDebug(7029) << "NOTE: this is an SSL server.";
        else kDebug(7029) << "NOTE: this is NOT an SSL server.";

        if (d->_extensions.certTypeNSSSLServer())
            kDebug(7029) << "NOTE: this is a NETSCAPE SSL server.";
        else kDebug(7029) << "NOTE: this is NOT a NETSCAPE SSL server.";

        if (d->_extensions.certTypeSMIME())
            kDebug(7029) << "NOTE: this is an SMIME certificate.";
        else kDebug(7029) << "NOTE: this is NOT an SMIME certificate.";

        if (d->_extensions.certTypeSMIMEEncrypt())
            kDebug(7029) << "NOTE: this is an SMIME encrypt cert.";
        else kDebug(7029) << "NOTE: this is NOT an SMIME encrypt cert.";

        if (d->_extensions.certTypeSMIMESign())
            kDebug(7029) << "NOTE: this is an SMIME sign cert.";
        else kDebug(7029) << "NOTE: this is NOT an SMIME sign cert.";

        if (d->_extensions.certTypeCRLSign())
            kDebug(7029) << "NOTE: this is a CRL signer.";
        else kDebug(7029) << "NOTE: this is NOT a CRL signer.";

        kDebug(7029) << "-----------------------------------------------"
                 << endl;
#endif
    }
#endif
    d->m_stateCached = false;
    d->m_stateCache = KSSLCertificate::Unknown;
}

X509 *KSSLCertificate::getCert() {
#ifdef KSSL_HAVE_SSL
    return d->m_cert;
#endif
    return 0;
}

// pull in the callback.  It's common across multiple files but we want
// it to be hidden.

#include "ksslcallback.c"


bool KSSLCertificate::isValid(KSSLCertificate::KSSLPurpose p) {
    return (validate(p) == KSSLCertificate::Ok);
}


bool KSSLCertificate::isValid() {
    return isValid(KSSLCertificate::SSLServer);
}


int KSSLCertificate::purposeToOpenSSL(KSSLCertificate::KSSLPurpose p) const {
    int rc = 0;
#ifdef KSSL_HAVE_SSL
    if (p == KSSLCertificate::SSLServer) {
        rc = X509_PURPOSE_SSL_SERVER;
    } else if (p == KSSLCertificate::SSLClient) {
        rc = X509_PURPOSE_SSL_CLIENT;
    } else if (p == KSSLCertificate::SMIMEEncrypt) {
        rc = X509_PURPOSE_SMIME_ENCRYPT;
    } else if (p == KSSLCertificate::SMIMESign) {
        rc = X509_PURPOSE_SMIME_SIGN;
    } else if (p == KSSLCertificate::Any) {
        rc = X509_PURPOSE_ANY;
    }
#endif
    return rc;
}


// For backward compatibility
KSSLCertificate::KSSLValidation KSSLCertificate::validate() {
    return validate(KSSLCertificate::SSLServer);
}

KSSLCertificate::KSSLValidation KSSLCertificate::validate(KSSLCertificate::KSSLPurpose purpose)
{
    KSSLValidationList result = validateVerbose(purpose);
    if (result.isEmpty()) {
        return KSSLCertificate::Ok;
    }
    else
        return result.first();
}

//
// See apps/verify.c in OpenSSL for the source of most of this logic.
//

// CRL files?  we don't do that yet
KSSLCertificate::KSSLValidationList KSSLCertificate::validateVerbose(KSSLCertificate::KSSLPurpose purpose)
{
    return validateVerbose(purpose, 0);
}

KSSLCertificate::KSSLValidationList KSSLCertificate::validateVerbose(KSSLCertificate::KSSLPurpose purpose, KSSLCertificate *ca)
{
    KSSLValidationList errors;
    if (ca || (d->_lastPurpose != purpose)) {
        d->m_stateCached = false;
    }

    if (!d->m_stateCached) {
        d->_lastPurpose = purpose;
    }

#ifdef KSSL_HAVE_SSL
    X509_STORE *certStore;
    X509_LOOKUP *certLookup;
    X509_STORE_CTX *certStoreCTX;
    int rc = 0;

    if (!d->m_cert) {
        errors << KSSLCertificate::Unknown;
        return errors;
    }

    if (d->m_stateCached) {
        errors << d->m_stateCache;
        return errors;
    }

    const QStringList qsl = KGlobal::dirs()->resourceDirs("kssl");

    if (qsl.isEmpty()) {
        errors << KSSLCertificate::NoCARoot;
        return errors;
    }

    KSSLCertificate::KSSLValidation ksslv = Unknown;

    for (QStringList::ConstIterator j = qsl.begin(); j != qsl.end(); ++j) {
        KDE_struct_stat sb;
        QString _j = (*j) + "ca-bundle.crt";
        if (-1 == KDE_stat(_j.toAscii().constData(), &sb)) {
            continue;
        }

        certStore = d->kossl->X509_STORE_new();
        if (!certStore) {
            errors << KSSLCertificate::Unknown;
            return errors;
        }

        X509_STORE_set_verify_cb_func(certStore, X509Callback);

        certLookup = d->kossl->X509_STORE_add_lookup(certStore, d->kossl->X509_LOOKUP_file());
        if (!certLookup) {
            ksslv = KSSLCertificate::Unknown;
            d->kossl->X509_STORE_free(certStore);
            continue;
        }

        if (!d->kossl->X509_LOOKUP_load_file(certLookup, _j.toAscii().constData(), X509_FILETYPE_PEM)) {
            // error accessing directory and loading pems
            kDebug(7029) << "KSSL couldn't read CA root: "
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
            kDebug(7029) << "KSSL couldn't create an X509 store context.";
            d->kossl->X509_STORE_free(certStore);
            continue;
        }

        d->kossl->X509_STORE_CTX_init(certStoreCTX, certStore, d->m_cert, NULL);
        if (d->_chain.isValid()) {
            d->kossl->X509_STORE_CTX_set_chain(certStoreCTX, (STACK_OF(X509)*)d->_chain.rawChain());
        }

        //kDebug(7029) << "KSSL setting CRL..............";
        // int X509_STORE_add_crl(X509_STORE *ctx, X509_CRL *x);

        d->kossl->X509_STORE_CTX_set_purpose(certStoreCTX, purposeToOpenSSL(purpose));

        KSSL_X509CallBack_ca = ca ? ca->d->m_cert : 0;
        KSSL_X509CallBack_ca_found = false;

        certStoreCTX->error = X509_V_OK;
        rc = d->kossl->X509_verify_cert(certStoreCTX);
        int errcode = certStoreCTX->error;
        if (ca && !KSSL_X509CallBack_ca_found) {
            ksslv = KSSLCertificate::Irrelevant;
        } else {
            ksslv = processError(errcode);
        }
        // For servers, we can try NS_SSL_SERVER too
        if ((ksslv != KSSLCertificate::Ok) &&
            (ksslv != KSSLCertificate::Irrelevant) &&
            purpose == KSSLCertificate::SSLServer) {
            d->kossl->X509_STORE_CTX_set_purpose(certStoreCTX,
                                                 X509_PURPOSE_NS_SSL_SERVER);

            certStoreCTX->error = X509_V_OK;
            rc = d->kossl->X509_verify_cert(certStoreCTX);
            errcode = certStoreCTX->error;
            ksslv = processError(errcode);
        }
        d->kossl->X509_STORE_CTX_free(certStoreCTX);
        d->kossl->X509_STORE_free(certStore);
        // end of checking code
        //

        //kDebug(7029) << "KSSL Validation procedure RC: "
        //        << rc << endl;
        //kDebug(7029) << "KSSL Validation procedure errcode: "
        //        << errcode << endl;
        //kDebug(7029) << "KSSL Validation procedure RESULTS: "
        //        << ksslv << endl;

        if (ksslv != NoCARoot && ksslv != InvalidCA && ksslv != GetIssuerCertFailed && ksslv != DecodeIssuerPublicKeyFailed && ksslv != GetIssuerCertLocallyFailed ) {
            d->m_stateCached = true;
            d->m_stateCache = ksslv;
        }
        break;
    }

    if (ksslv != KSSLCertificate::Ok) {
        errors << ksslv;
    }
#else
    errors << KSSLCertificate::NoSSL;
#endif
    return errors;
}



KSSLCertificate::KSSLValidation KSSLCertificate::revalidate() {
    return revalidate(KSSLCertificate::SSLServer);
}


KSSLCertificate::KSSLValidation KSSLCertificate::revalidate(KSSLCertificate::KSSLPurpose p) {
    d->m_stateCached = false;
    return validate(p);
}


KSSLCertificate::KSSLValidation KSSLCertificate::processError(int ec) {
    KSSLCertificate::KSSLValidation rc;

    rc = KSSLCertificate::Unknown;
#ifdef KSSL_HAVE_SSL
    switch (ec) {

        // see man 1 verify for a detailed listing of all error codes

        // error 0
        case X509_V_OK:
            rc = KSSLCertificate::Ok;
        break;


        // error 2
        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
            rc = KSSLCertificate::GetIssuerCertFailed;
        break;

        // error 3
        case X509_V_ERR_UNABLE_TO_GET_CRL:
            rc = KSSLCertificate::GetCRLFailed;
        break;

        // error 4
        case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
            rc = KSSLCertificate::DecryptCertificateSignatureFailed;
        break;

        // error 5
        case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
            rc = KSSLCertificate::DecryptCRLSignatureFailed;
        break;

        // error 6
        case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
            rc = KSSLCertificate::DecodeIssuerPublicKeyFailed;
        break;

        // error 7
        case X509_V_ERR_CERT_SIGNATURE_FAILURE:
            rc = KSSLCertificate::CertificateSignatureFailed;
        break;

        // error 8
        case X509_V_ERR_CRL_SIGNATURE_FAILURE:
            rc = KSSLCertificate::CRLSignatureFailed;
        break;

        // error 9
        case X509_V_ERR_CERT_NOT_YET_VALID:
            rc = KSSLCertificate::CertificateNotYetValid;
        break;

        // error 10
        case X509_V_ERR_CERT_HAS_EXPIRED:
            rc = KSSLCertificate::CertificateHasExpired;
            kDebug(7029) << "KSSL apparently this is expired.  Not after: "
                    << getNotAfter() << endl;
        break;

        // error 11
        case X509_V_ERR_CRL_NOT_YET_VALID:
            rc = KSSLCertificate::CRLNotYetValid;
        break;

        // error 12
        case X509_V_ERR_CRL_HAS_EXPIRED:
            rc = KSSLCertificate::CRLHasExpired;
        break;

        // error 13
        case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
            rc = KSSLCertificate::CertificateFieldNotBeforeErroneous;
        break;

        // error 14
        case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
            rc = KSSLCertificate::CertificateFieldNotAfterErroneous;
        break;

        // error 15 - unused as of OpenSSL 0.9.8g
        case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
            rc = KSSLCertificate::CRLFieldLastUpdateErroneous;
        break;

        // error 16 - unused as of OpenSSL 0.9.8g
        case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
            rc = KSSLCertificate::CRLFieldNextUpdateErroneous;
        break;

        // error 17
        case X509_V_ERR_OUT_OF_MEM:
            rc = KSSLCertificate::OutOfMemory;
        break;

        // error 18
        case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
            rc = KSSLCertificate::SelfSigned;
        break;

        // error 19
        case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
            rc = KSSLCertificate::SelfSignedInChain;
        break;

        // error 20
        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
            rc = KSSLCertificate::GetIssuerCertLocallyFailed;
        break;

        // error 21
        case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
            rc = KSSLCertificate::VerifyLeafSignatureFailed;
        break;

        // error 22 - unused as of OpenSSL 0.9.8g
        case X509_V_ERR_CERT_CHAIN_TOO_LONG:
            rc = KSSLCertificate::CertificateChainTooLong;
        break;

        // error 23 - unused as of OpenSSL 0.9.8g
        case X509_V_ERR_CERT_REVOKED:
            rc = KSSLCertificate::CertificateRevoked;
        break;

        // error 24
        case X509_V_ERR_INVALID_CA:
            rc = KSSLCertificate::InvalidCA;
        break;

        // error 25
        case X509_V_ERR_PATH_LENGTH_EXCEEDED:
            rc = KSSLCertificate::PathLengthExceeded;
        break;

        // error 26
        case X509_V_ERR_INVALID_PURPOSE:
            rc = KSSLCertificate::InvalidPurpose;
        break;

        // error 27
        case X509_V_ERR_CERT_UNTRUSTED:
            rc = KSSLCertificate::CertificateUntrusted;
        break;

        // error 28
        case X509_V_ERR_CERT_REJECTED:
            rc = KSSLCertificate::CertificateRejected;
        break;

        // error 29 - only used with -issuer_checks
        case X509_V_ERR_SUBJECT_ISSUER_MISMATCH:
            rc = KSSLCertificate::IssuerSubjectMismatched;
        break;

        // error 30 - only used with -issuer_checks
        case X509_V_ERR_AKID_SKID_MISMATCH:
            rc = KSSLCertificate::AuthAndSubjectKeyIDMismatched;
        break;

        // error 31 - only used with -issuer_checks
        case X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH:
            rc = KSSLCertificate::AuthAndSubjectKeyIDAndNameMismatched;
        break;

        // error 32
        case X509_V_ERR_KEYUSAGE_NO_CERTSIGN:
            rc = KSSLCertificate::KeyMayNotSignCertificate;
        break;


        // error 50 - unused as of OpenSSL 0.9.8g
        case X509_V_ERR_APPLICATION_VERIFICATION:
            rc = KSSLCertificate::ApplicationVerificationFailed;
        break;


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
#ifdef KSSL_HAVE_SSL
    return ASN1_UTCTIME_QString(X509_get_notBefore(d->m_cert));
#else
    return QString();
#endif
}


QString KSSLCertificate::getNotAfter() const {
#ifdef KSSL_HAVE_SSL
    return ASN1_UTCTIME_QString(X509_get_notAfter(d->m_cert));
#else
    return QString();
#endif
}


QDateTime KSSLCertificate::getQDTNotBefore() const {
#ifdef KSSL_HAVE_SSL
    return ASN1_UTCTIME_QDateTime(X509_get_notBefore(d->m_cert), NULL);
#else
    return QDateTime::currentDateTime();
#endif
}


QDateTime KSSLCertificate::getQDTNotAfter() const {
#ifdef KSSL_HAVE_SSL
    return ASN1_UTCTIME_QDateTime(X509_get_notAfter(d->m_cert), NULL);
#else
    return QDateTime::currentDateTime();
#endif
}


int operator==(KSSLCertificate &x, KSSLCertificate &y) {
#ifndef KSSL_HAVE_SSL
    return 1;
#else
    if (!KOSSL::self()->X509_cmp(x.getCert(), y.getCert())) {
        return 1;
    }
    return 0;
#endif
}


KSSLCertificate *KSSLCertificate::replicate() {
    // The new certificate doesn't have the cached value.  It's probably
    // better this way.  We can't anticipate every reason for doing this.
    KSSLCertificate *newOne = new KSSLCertificate();
#ifdef KSSL_HAVE_SSL
    newOne->setCert(d->kossl->X509_dup(getCert()));
    KSSLCertChain *c = d->_chain.replicate();
    newOne->setChain(c->rawChain());
    delete c;
#endif
    return newOne;
}


QString KSSLCertificate::toString() 
{
    return toDer().toBase64();
}


QString KSSLCertificate::verifyText(KSSLValidation x) {
    switch (x) {
    // messages for errors defined in verify(1)
    case KSSLCertificate::Ok:
        return i18n("The certificate is valid.");
    case KSSLCertificate::GetIssuerCertFailed:
        return i18n("Retrieval of the issuer certificate failed. This means the CA's (Certificate Authority) certificate can not be found.");
    case KSSLCertificate::GetCRLFailed:
        return i18n("Retrieval of the CRL (Certificate Revocation List) failed. This means the CA's (Certificate Authority) CRL can not be found.");
    case KSSLCertificate::DecryptCertificateSignatureFailed:
        return i18n("The decryption of the certificate's signature failed. This means it could not even be calculated as opposed to just not matching the expected result.");
    case KSSLCertificate::DecryptCRLSignatureFailed:
        return i18n("The decryption of the CRL's (Certificate Revocation List) signature failed. This means it could not even be calculated as opposed to just not matching the expected result.");
    case KSSLCertificate::DecodeIssuerPublicKeyFailed:
        return i18n("The decoding of the public key of the issuer failed. This means that the CA's (Certificate Authority) certificate can not be used to verify the certificate you wanted to use.");
    case KSSLCertificate::CertificateSignatureFailed:
        return i18n("The certificate's signature is invalid. This means that the certificate can not be verified.");
    case KSSLCertificate::CRLSignatureFailed:
        return i18n("The CRL's (Certificate Revocation List) signature is invalid. This means that the CRL can not be verified.");
    case KSSLCertificate::CertificateNotYetValid:
        return i18n("The certificate is not valid, yet.");
    case KSSLCertificate::CertificateHasExpired:
        return i18n("The certificate is not valid, any more.");
    case KSSLCertificate::CRLNotYetValid:
        return i18n("The CRL (Certificate Revocation List) is not valid, yet.");
    case KSSLCertificate::CRLHasExpired:
        return i18n("The CRL (Certificate Revocation List) is not valid, yet.");
    case KSSLCertificate::CertificateFieldNotBeforeErroneous:
        return i18n("The time format of the certificate's 'notBefore' field is invalid.");
    case KSSLCertificate::CertificateFieldNotAfterErroneous:
        return i18n("The time format of the certificate's 'notAfter' field is invalid.");
    case KSSLCertificate::CRLFieldLastUpdateErroneous:
        return i18n("The time format of the CRL's (Certificate Revocation List) 'lastUpdate' field is invalid.");
    case KSSLCertificate::CRLFieldNextUpdateErroneous:
        return i18n("The time format of the CRL's (Certificate Revocation List) 'nextUpdate' field is invalid.");
    case KSSLCertificate::OutOfMemory:
        return i18n("The OpenSSL process ran out of memory.");
    case KSSLCertificate::SelfSigned:
        return i18n("The certificate is self-signed and not in the list of trusted certificates. If you want to accept this certificate, import it into the list of trusted certificates.");
    case KSSLCertificate::SelfSignedChain:      // this is obsolete and kept around for backwards compatibility, only
    case KSSLCertificate::SelfSignedInChain:
        return i18n("The certificate is self-signed. While the trust chain could be built up, the root CA's (Certificate Authority) certificate can not be found.");
    case KSSLCertificate::GetIssuerCertLocallyFailed:
        return i18n("The CA's (Certificate Authority) certificate can not be found. Most likely, your trust chain is broken.");
    case KSSLCertificate::VerifyLeafSignatureFailed:
        return i18n("The certificate can not be verified as it is the only certificate in the trust chain and not self-signed. If you self-sign the certificate, make sure to import it into the list of trusted certificates.");
    case KSSLCertificate::CertificateChainTooLong:
        return i18n("The certificate chain is longer than the maximum depth specified.");
    case KSSLCertificate::Revoked:     // this is obsolete and kept around for backwards compatibility, only
    case KSSLCertificate::CertificateRevoked:
        return i18n("The certificate has been revoked.");
    case KSSLCertificate::InvalidCA:
        return i18n("The certificate's CA (Certificate Authority) is invalid.");
    case KSSLCertificate::PathLengthExceeded:
        return i18n("The length of the trust chain exceeded one of the CA's (Certificate Authority) 'pathlength' parameters, making all subsequent signatures invalid.");
    case KSSLCertificate::InvalidPurpose:
        return i18n("The certificate has not been signed for the purpose you tried to use it for. This means the CA (Certificate Authority) does not allow this usage.");
    case KSSLCertificate::Untrusted:     // this is obsolete and kept around for backwards compatibility, only
    case KSSLCertificate::CertificateUntrusted:
        return i18n("The root CA (Certificate Authority) is not trusted for the purpose you tried to use this certificate for.");
    case KSSLCertificate::Rejected:     // this is obsolete and kept around for backwards compatibility, only     // this is obsolete and kept around for backwards compatibility, onle
    case KSSLCertificate::CertificateRejected:
        return i18n("The root CA (Certificate Authority) has been marked to be rejected for the purpose you tried to use it for.");
    case KSSLCertificate::IssuerSubjectMismatched:
        return i18n("The certificate's CA (Certificate Authority) does not match the CA name of the certificate.");
    case KSSLCertificate::AuthAndSubjectKeyIDMismatched:
        return i18n("The CA (Certificate Authority) certificate's key ID does not match the key ID in the 'Issuer' section of the certificate you are trying to use.");
    case KSSLCertificate::AuthAndSubjectKeyIDAndNameMismatched:
        return i18n("The CA (Certificate Authority) certificate's key ID and name do not match the key ID and name in the 'Issuer' section of the certificate you are trying to use.");
    case KSSLCertificate::KeyMayNotSignCertificate:
        return i18n("The certificate's CA (Certificate Authority) is not allowed to sign certificates.");
    case KSSLCertificate::ApplicationVerificationFailed:
        return i18n("OpenSSL could not be verified.");


    // this is obsolete and kept around for backwards compatibility, only
    case KSSLCertificate::SignatureFailed:
        return i18n("The signature test for this certificate failed. This could mean that the signature of this certificate or any in its trust path are invalid, could not be decoded or that the CRL (Certificate Revocation List) could not be verified. If you see this message, please let the author of the software you are using know that he or she should use the new, more specific error messages.");
    case KSSLCertificate::Expired:
        return i18n("This certificate, any in its trust path or its CA's (Certificate Authority) CRL (Certificate Revocation List) is not valid. Any of them could not be valid yet or not valid any more. If you see this message, please let the author of the software you are using know that he or she should use the new, more specific error messages.");
    // continue 'useful' messages

    // other error messages
    case KSSLCertificate::ErrorReadingRoot:
    case KSSLCertificate::NoCARoot:
        return i18n("Certificate signing authority root files could not be found so the certificate is not verified.");
    case KSSLCertificate::NoSSL:
        return i18n("SSL support was not found.");
    case KSSLCertificate::PrivateKeyFailed:
        return i18n("Private key test failed.");
    case KSSLCertificate::InvalidHost:
        return i18n("The certificate has not been issued for this host.");
    case KSSLCertificate::Irrelevant:
        return i18n("This certificate is not relevant.");
    default:
    break;
    }

    return i18n("The certificate is invalid.");
}


QByteArray KSSLCertificate::toDer() {
    QByteArray qba;
#ifdef KSSL_HAVE_SSL
    int certlen = d->kossl->i2d_X509(getCert(), NULL);
    if (certlen >= 0) {
        // These should technically be unsigned char * but it doesn't matter
        // for our purposes
        char *cert = new char[certlen];
        unsigned char *p = (unsigned char *)cert;
        // FIXME: return code!
        d->kossl->i2d_X509(getCert(), &p);

        // encode it into a QString
        qba = QByteArray(cert, certlen);
        delete[] cert;
    }
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

    if (thecert[thecert.length()-1] != '\n') {
        thecert += '\n';
    }

    thecert.append(footer);

    qba = thecert.toLocal8Bit();
    return qba;
}


#define NETSCAPE_CERT_HDR     "certificate"

#ifdef KSSL_HAVE_SSL
#if OPENSSL_VERSION_NUMBER < 0x00909000L

typedef struct NETSCAPE_X509_st
{
    ASN1_OCTET_STRING *header;
    X509 *cert;
} NETSCAPE_X509;
#endif
#endif

// what a piece of crap this is
QByteArray KSSLCertificate::toNetscape() {
    QByteArray qba;
#ifdef KSSL_HAVE_SSL
    NETSCAPE_X509 nx;
    ASN1_OCTET_STRING hdr;
    KTemporaryFile ktf;
    ktf.open();
    FILE *ktf_fs = fopen(ktf.fileName().toAscii(), "r+");

    hdr.data = (unsigned char *)NETSCAPE_CERT_HDR;
    hdr.length = strlen(NETSCAPE_CERT_HDR);
    nx.header = &hdr;
    nx.cert = getCert();

    d->kossl->ASN1_item_i2d_fp(ktf_fs,(unsigned char *)&nx);
    fclose(ktf_fs);

    QFile qf(ktf.fileName());
    qf.open(QIODevice::ReadOnly);
    char *buf = new char[qf.size()];
    qf.read(buf, qf.size());
    qba = QByteArray(buf, qf.size());
    qf.close();
    delete[] buf;

#endif
return qba;
}



QString KSSLCertificate::toText() {
    QString text;
#ifdef KSSL_HAVE_SSL
    KTemporaryFile ktf;
    ktf.open();
    FILE *ktf_fs = fopen(ktf.fileName().toAscii(), "r+");

    d->kossl->X509_print(ktf_fs, getCert());
    fclose(ktf_fs);

    QFile qf(ktf.fileName());
    qf.open(QIODevice::ReadOnly);
    char *buf = new char[qf.size()+1];
    qf.read(buf, qf.size());
    buf[qf.size()] = 0;
    text = buf;
    delete[] buf;
    qf.close();
#endif
return text;
}

bool KSSLCertificate::setCert(const QString& cert) {
#ifdef KSSL_HAVE_SSL
        QByteArray qba, qbb = cert.toLocal8Bit();
        qba = QByteArray::fromBase64(qbb);
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


QStringList KSSLCertificate::subjAltNames() const {
    QStringList rc;
#ifdef KSSL_HAVE_SSL
    STACK_OF(GENERAL_NAME) *names;
    names = (STACK_OF(GENERAL_NAME)*)d->kossl->X509_get_ext_d2i(d->m_cert, NID_subject_alt_name, 0, 0);

    if (!names) {
        return rc;
    }

    int cnt = d->kossl->sk_GENERAL_NAME_num(names);

    for (int i = 0; i < cnt; i++) {
        const GENERAL_NAME *val = (const GENERAL_NAME *)d->kossl->sk_value(names, i);
        if (val->type != GEN_DNS) {
            continue;
        }

        QString s = (const char *)d->kossl->ASN1_STRING_data(val->d.ia5);
        if (!s.isEmpty()) {
            rc += s;
        }
    }
    d->kossl->sk_free(names);
#endif
    return rc;
}


QDataStream& operator<<(QDataStream& s, const KSSLCertificate& r) {
    QStringList qsl;
    QList<KSSLCertificate *> cl = const_cast<KSSLCertificate&>(r).chain().getChain();

    foreach(KSSLCertificate *c, cl) {
        qsl << c->toString();
    }

    qDeleteAll(cl);
    s << const_cast<KSSLCertificate&>(r).toString() << qsl;

    return s;
}


QDataStream& operator>>(QDataStream& s, KSSLCertificate& r) {
    QStringList qsl;
    QString cert;

    s >> cert >> qsl;

        if (r.setCert(cert) && !qsl.isEmpty()) {
            r.chain().setCertChain(qsl);
        }

    return s;
}



