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

#ifndef __KSMIMECRYPTO_H
#define __KSMIMECRYPTO_H

#include <q3ptrlist.h>
#include "ksslpkcs12.h"
#include "ksslcertificate.h"

class KOpenSSLProxy;
class KSMIMECryptoPrivate;

class KIO_EXPORT KSMIMECrypto {
 public:
    KSMIMECrypto();
    ~KSMIMECrypto();

    enum algo { KSC_C_DES3_CBC = 1,
		KSC_C_RC2_CBC_128,
		KSC_C_RC2_CBC_64,
		KSC_C_DES_CBC,
		KSC_C_RC2_CBC_40 };

    enum rc { KSC_R_OK,        /* everything ok */
	      KSC_R_OTHER,     /* unspecified error */
	      KSC_R_NO_SSL,    /* No crypto lib / compiled without SSL */
	      KSC_R_NOCIPHER,  /* encryption cipher n/a */
	      KSC_R_NOMEM,     /* out of memory */
	      KSC_R_FORMAT,    /* wrong input data format */
	      KSC_R_WRONGKEY,  /* wrong decryption/signature key */
	      KSC_R_VERIFY     /* data does not match signature */
    };

    /**
     * Sign a message
     * @param clearText MIME representation of the message (part) to sign
     * @param cipherText signature to append or signature block
     * @param privKey private key/certificate to sign with
     * @param certs additional certificates (may be empty)
     * @param detached create detached or opaque signature
     * @return 0 on success
     */
    rc signMessage(const QByteArray &clearText,
		   QByteArray &cipherText,
		   const KSSLPKCS12 &privKey,
		   const Q3PtrList<KSSLCertificate> &certs,
		   bool detached);

    /**
     * Check a detached message signature
     * Will check if messages matches signature and extract certificates
     * Does not check certificates for validity!
     * @param clearText MIME representation of signed message (without SIG)
     * @param signature signature
     * @param foundCerts certificates found in this message
     * @return 0 on success
     */
    rc checkDetachedSignature(const QByteArray &clearText,
			      const QByteArray &signature,
			      Q3PtrList<KSSLCertificate> &foundCerts);

    /**
     * Check an opaque signed message
     * Will check if signature matches and extract message
     * Does not check certificates for validity!
     * @param signedText signed message block
     * @param clearText cleartext of signed message
     * @foundCerts certificates found in this mesasge
     * @return 0 on success
     */
    rc checkOpaqueSignature(const QByteArray &signedText,
			    QByteArray &clearText,
			    Q3PtrList<KSSLCertificate> &foundCerts);
    
    /**
     * Encrypt a message
     * encrypts a message for the given list of recipients and the
     * selected algorithm. Note that any algorithm <128 bytes is
     * insecure and should never be used, even if SMIME-2 requires
     * only RC2-40
     * @param clearText MIME representation of message to encrypt
     * @param cipherText returned encrypted message
     * @param algorithm encryption algorithm
     * @recip recipient certificates
     * @return 0 on success
     */
    rc encryptMessage(const QByteArray &clearText,
		      QByteArray &cipherText,
		      algo algorithm,
		      const Q3PtrList<KSSLCertificate> &recip);

    /**
     * Decrypt a message
     * @param cipherText encrypted message block
     * @param clearText returns decrypted message
     * @param privKey private key to use
     * @return 0 on success
     */
    rc decryptMessage(const QByteArray &cipherText,
		      QByteArray &clearText,
		      const KSSLPKCS12 &privKey);

 private:
    KSMIMECryptoPrivate *priv;
    KOpenSSLProxy *kossl;
};

#endif
