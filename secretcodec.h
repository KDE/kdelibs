/* This file is part of the KDE project
 *
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
#ifndef SECRETCODEC_H
#define SECRETCODEC_H

#include <QtCrypto/QtCrypto>

class SecretCodec {
public:
    SecretCodec();
    ~SecretCodec();

    static const char* AlgorithmPlain;
    
    bool initServer( const QString &algorithm, const QVariant &input, QVariant &output );
    bool initClient( const QString &algorithm, const QVariant serverOutput );
    bool encryptServer( const QCA::SecureArray &value, QCA::SecureArray &encrypted, QByteArray &encryptedParams );
    bool decryptServer( const QCA::SecureArray &encrypted, const QByteArray &encryptedParams, QCA::SecureArray &value );
    bool encryptClient( const QCA::SecureArray &value, QCA::SecureArray &encrypted, QByteArray &encryptedParams );
    bool decryptClient( const QCA::SecureArray &encrypted, const QByteArray &encryptedParams, QCA::SecureArray &value );
    
private:
    enum Mode { 
        ModeUnitialized =0,
        ModeServer, 
        ModeClient 
    };
    QCA::Cipher         *m_cipher;
    QCA::SymmetricKey   m_symmetricKey;
    Mode                m_mode;
};

#endif // SECRETCODEC_H
