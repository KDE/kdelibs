/* This file is part of the KDE project
 *
 * Copyright 2010, Michael Leupold <lemma@confuego.org>
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
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

#include "secretcodec.h"

#include <qca_tools.h>
#include <kdebug.h>

const char* SecretCodec::AlgorithmPlain = "plain";

SecretCodec::SecretCodec() :
    m_cipher(0),
    m_mode(ModeUnitialized)
{
    static bool qcaInitialized = false;
    if ( !qcaInitialized ) {
        QCA::init();
        qcaInitialized = true;
    }
}

SecretCodec::~SecretCodec()
{
    delete m_cipher;
}

bool SecretCodec::initServer(const QString& algorithm, const QVariant& input, QVariant& output)
{
    if ( m_mode != ModeUnitialized ) {
        kDebug() << "ERROR the code is already initialized";
        Q_ASSERT(0);
        return false;
    }

    m_mode = ModeServer;
    
    if ( algorithm.compare( AlgorithmPlain ) == 0 ) {
        kDebug() << "Initializing a PLAIN (noencrypting) server codec";
    }
    else {
    
        static QRegExp rxAlgorithm("^dh-ietf(\\d+)-([^-]+)-([^-]+)-([^-]+)$",
                                Qt::CaseInsensitive);

        if(rxAlgorithm.exactMatch(algorithm) &&
                input.type() == QVariant::ByteArray) {
            QString encalgo = rxAlgorithm.cap(2).toLower();
            QString blockmode = rxAlgorithm.cap(3).toLower();
            QString padding = rxAlgorithm.cap(4).toLower();

            QCA::KeyGenerator keygen;

            // determine the discrete logarithm group to use
            QCA::DLGroupSet groupnum;
            switch(rxAlgorithm.cap(1).toInt()) {
            case 768:
                groupnum = QCA::IETF_768;
                break;
            case 1024:
                groupnum = QCA::IETF_1024;
                break;
            case 1536:
                groupnum = QCA::IETF_1536;
                break;
            case 2048:
                groupnum = QCA::IETF_2048;
                break;
            case 3072:
                groupnum = QCA::IETF_3072;
                break;
            case 4096:
                groupnum = QCA::IETF_4096;
                break;
            case 6144:
                groupnum = QCA::IETF_6144;
                break;
            case 8192:
                groupnum = QCA::IETF_8192;
                break;
            default:
                // no known discrete logarithm group
                return false;
            }
            QCA::DLGroup dlgroup(keygen.createDLGroup(groupnum));
            if(dlgroup.isNull()) {
                return false;
            }

            // determine if we support (or want to support)
            // the encryption algorithm.
            if((encalgo == "blowfish" || encalgo == "twofish" ||
                    encalgo == "aes128" || encalgo == "aes192" ||
                    encalgo == "aes256") &&
                    QCA::isSupported(QString("%1-%2-%3").arg(encalgo, blockmode, padding)
                                    .toLatin1().constData())) {

                // get client's public key
                QCA::DHPublicKey clientKey(dlgroup,
                                        QCA::BigInteger(QCA::SecureArray(input.toByteArray())));
                // generate own private key
                QCA::PrivateKey privKey(keygen.createDH(dlgroup));
                // generate the shared symmetric key
                QCA::SymmetricKey sharedKey(privKey.deriveKey(clientKey));

                QCA::Cipher::Mode cbm;
                if(blockmode == "cbc") {
                    cbm = QCA::Cipher::CBC;
                } else {
                    return false;
                }

                QCA::Cipher::Padding cp;
                if(padding == "pkcs7") {
                    cp = QCA::Cipher::PKCS7;
                } else if(padding == "default") {
                    cp = QCA::Cipher::DefaultPadding;
                } else {
                    return false;
                }

                QCA::Cipher *cipher = new QCA::Cipher(encalgo, cbm, cp);

                // check if creating the cipher worked and if our shared
                // key is longer than the minimum length required.
                if(sharedKey.size() >= cipher->keyLength().minimum()) {
                    output.setValue(privKey.toPublicKey().toDH().y().toArray().toByteArray());
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool SecretCodec::initClient(const QString& algorithm, const QVariant serverOutput)
{
    if ( m_mode != ModeUnitialized ) {
        kDebug() << "ERROR the code is already initialized";
        Q_ASSERT(0);
        return false;
    }
    
    m_mode = ModeClient;
    
    if ( algorithm.compare( AlgorithmPlain ) == 0 ) {
        kDebug() << "Initializing a PLAIN (noencrypting) client codec";
    }
    else {
        // TODO: implement other cases here
        Q_ASSERT(0);
    }
    
    return true;
}

bool SecretCodec::encryptServer(const QCA::SecureArray& value, QCA::SecureArray& encrypted, QByteArray &encryptedParams)
{
    Q_ASSERT( m_mode == ModeServer );
    if (m_cipher) {
        QCA::InitializationVector iv(m_cipher->blockSize());
        m_cipher->setup(QCA::Encode, m_symmetricKey, iv);
        QCA::SecureArray encval = m_cipher->update(value);
        if(!m_cipher->ok()) {
            return false;
        }
        encval += m_cipher->final();
        if(!m_cipher->ok()) {
            return false;
        }
        encrypted = encval.toByteArray();
        encryptedParams = iv.toByteArray();
    }
    else {
        // unencrypted session
        encrypted = value;
    }
    return true;
}

bool SecretCodec::decryptServer(const QCA::SecureArray& encrypted, const QByteArray &encryptedParams, QCA::SecureArray& value)
{
    Q_ASSERT( m_mode == ModeServer );
    if (m_cipher) {
        if(! encryptedParams.size() == m_cipher->blockSize()) {
            return false;
        }
        QCA::InitializationVector iv(encryptedParams);
        m_cipher->setup(QCA::Decode, m_symmetricKey, iv);
        value = m_cipher->update(encrypted);
        if(!m_cipher->ok()) {
            return false;
        }
        value += m_cipher->final();
        if(!m_cipher->ok()) {
            return false;
        }
    }
    else {
        value = encrypted;
    }
    return true;
}

bool SecretCodec::encryptClient(const QCA::SecureArray& value, QCA::SecureArray& encrypted, QByteArray& encryptedParams)
{
    if ( m_cipher ) {
        // TODO: implement client-side encryption
        Q_ASSERT(0);
    }
    else {
        encrypted = value;
    }
    return true;
}

bool SecretCodec::decryptClient(const QCA::SecureArray& encrypted, const QByteArray& encryptedParams, QCA::SecureArray& value)
{
    if ( m_cipher ) {
        // TODO: implement client-side decryption
        Q_ASSERT(0);
    }
    else {
        value = encrypted;
    }
    return true;
}
