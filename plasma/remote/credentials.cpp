/*
 *   Copyright © 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "credentials.h"
#include "config-plasma.h"

#include <QCryptographicHash>
#include <QObject>

#if ENABLE_REMOTE_WIDGETS
#include <QtCrypto>
#endif

#include <kdebug.h>
#include <kstandarddirs.h>

#include "authorizationmanager.h"

#define REQUIRED_FEATURES "rsa,sha1,pkey"

namespace Plasma {

class CredentialsPrivate {
public:
    CredentialsPrivate()
    {
    }

    CredentialsPrivate(const QString &id, const QString &name,
                    const QString &pemKey, bool isPrivateKey)
        : id(id),
          name(name)
    {
    #if ENABLE_REMOTE_WIDGETS
        if (!QCA::isSupported(REQUIRED_FEATURES)) {
            kWarning() << "QCA doesn't support " << REQUIRED_FEATURES;
            return;
        }

        if (isPrivateKey) {
            privateKey = QCA::PrivateKey::fromPEM(pemKey);
            publicKey = privateKey.toPublicKey();
        } else {
            publicKey = QCA::PublicKey::fromPEM(pemKey);
        }
    #endif
    }

    ~CredentialsPrivate()
    {
    }

    QString id;
    QString name;

#if ENABLE_REMOTE_WIDGETS
    QCA::PublicKey publicKey;
    QCA::PrivateKey privateKey;
#endif
};

Credentials::Credentials(const QString &id, const QString &name,
                   const QString &key, bool isPrivateKey)
         : d(new CredentialsPrivate(id, name, key, isPrivateKey))
{
}

Credentials::Credentials()
         : d(new CredentialsPrivate())
{
}

Credentials::Credentials(const Credentials &other)
        : d(new CredentialsPrivate())
{
    *d = *other.d;
}

Credentials::~Credentials()
{
    delete d;
}

Credentials &Credentials::operator=(const Credentials &other)
{
    *d = *other.d;
    return *this;
}

Credentials Credentials::createCredentials(const QString &name)
{
#if ENABLE_REMOTE_WIDGETS
    if (!QCA::isSupported(REQUIRED_FEATURES)) {
        kWarning() << "QCA doesn't support " << REQUIRED_FEATURES;
        return Credentials();
    }

    QCA::KeyGenerator generator;
    QCA::PrivateKey key = generator.createRSA(2048);
    QString pemKey(key.toPublicKey().toPEM());
    QString id = QCryptographicHash::hash(pemKey.toAscii(), QCryptographicHash::Sha1);
    return Credentials(id, name, key.toPEM(), true);
#else
    return Credentials();
#endif
}

TrustLevel Credentials::trustLevel() const
{
    /**
    QString pemFile = KStandardDirs::locate("trustedkeys", id());

    if (!pemFile.isEmpty()) {
        QCA::PublicKey pubKey = QCA::PublicKey::fromPEMFile(pemFile);
        if (pubKey == d->publicKey) {
            return true;
        }
    }
    */
    //Trust no one ;)
    return UnknownTrusted;
}

bool Credentials::isValid() const
{
#if ENABLE_REMOTE_WIDGETS
    if (!QCA::isSupported(REQUIRED_FEATURES)) {
        kWarning() << "QCA doesn't support " << REQUIRED_FEATURES;
        return false;
    }

    if (d->publicKey.isNull()) {
        return false;
    } else {
        QString id = QCryptographicHash::hash(d->publicKey.toPEM().toAscii(), QCryptographicHash::Sha1);
        return (id == d->id);
    }
#else
#ifndef NDEBUG
    kDebug() << "libplasma is compiled without support for remote widgets. Key invalid.";
#endif
    return false;
#endif
}

QString Credentials::name() const
{
    return d->name;
}

QString Credentials::id() const
{
    return d->id;
}

bool Credentials::isValidSignature(const QByteArray &signature, const QByteArray &payload) 
{
#if ENABLE_REMOTE_WIDGETS
    if (!QCA::isSupported(REQUIRED_FEATURES)) {
        kWarning() << "QCA doesn't support " << REQUIRED_FEATURES;
        return false;
    }

    if (d->publicKey.canVerify()) {
        if (!isValid()) {
#ifndef NDEBUG
            kDebug() << "Key is null?";
#endif
        }
        QCA::PublicKey publicKey = QCA::PublicKey::fromPEM(d->publicKey.toPEM());
        publicKey.startVerify( QCA::EMSA3_MD5 );
        publicKey.update(payload);
        return ( publicKey.validSignature( signature ) );
    } else {
#ifndef NDEBUG
        kDebug() << "Can't verify?";
#endif
        return false;
    }
#else
    return false;
#endif
}

bool Credentials::canSign() const
{
#if ENABLE_REMOTE_WIDGETS
    if (!QCA::isSupported(REQUIRED_FEATURES)) {
        kWarning() << "QCA doesn't support " << REQUIRED_FEATURES;
        return false;
    }

    return d->privateKey.canSign();
#else
    return false;
#endif
}

QByteArray Credentials::signMessage(const QByteArray &message)
{
#if ENABLE_REMOTE_WIDGETS
    if(!QCA::isSupported(REQUIRED_FEATURES)) {
#ifndef NDEBUG
        kDebug() << "RSA not supported";
#endif
        return QByteArray();
    } else if (canSign()) {
        //QCA::PrivateKey privateKey = QCA::PrivateKey::fromPEM(d->privateKey.toPEM());
        d->privateKey.startSign( QCA::EMSA3_MD5 );
        d->privateKey.update( message );
        QByteArray signature = d->privateKey.signature();
        return signature;
    } else {
        return QByteArray();
    }
#else
    return QByteArray();
#endif
}

Credentials Credentials::toPublicCredentials() const
{
#if ENABLE_REMOTE_WIDGETS
    Credentials result(*this);
    result.d->privateKey = QCA::PrivateKey();
    return result;
#else
    return Credentials();
#endif
}

QDataStream &operator<<(QDataStream &out, const Credentials &myObj)
{
#if ENABLE_REMOTE_WIDGETS
    if (!QCA::isSupported(REQUIRED_FEATURES)) {
        kWarning() << "QCA doesn't support " << REQUIRED_FEATURES;
        return out;
    }

    QString privateKeyPem;
    QString publicKeyPem;

    if (!myObj.d->privateKey.isNull()) {
        privateKeyPem = myObj.d->privateKey.toPEM();
    }
    if (!myObj.d->publicKey.isNull()) {
        publicKeyPem = myObj.d->publicKey.toPEM();
    }

    out << 1 << myObj.d->id << myObj.d->name << privateKeyPem << publicKeyPem;
#endif

    return out;
}

QDataStream &operator>>(QDataStream &in, Credentials &myObj)
{
#if ENABLE_REMOTE_WIDGETS
    if (!QCA::isSupported(REQUIRED_FEATURES)) {
        kWarning() << "QCA doesn't support " << REQUIRED_FEATURES;
        return in;
    }

    QString privateKeyString;
    QString publicKeyString;
    uint version;

    in >> version >> myObj.d->id >> myObj.d->name >> privateKeyString >> publicKeyString;
    QCA::ConvertResult conversionResult;

    if (!privateKeyString.isEmpty()) {
        myObj.d->privateKey = QCA::PrivateKey::fromPEM(privateKeyString,
                                                    QByteArray(), &conversionResult);
    }

    if (!publicKeyString.isEmpty()) {
        myObj.d->publicKey = QCA::PublicKey::fromPEM(publicKeyString, &conversionResult);
    }

    if (conversionResult != QCA::ConvertGood) {
#ifndef NDEBUG
        kDebug() << "Unsuccessfull conversion of key?";
#endif
    }
#endif

    return in;
}

}
