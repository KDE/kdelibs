/* This file is part of the KDE libraries
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "ktcpsocket.h"

#include <kdebug.h>
#include <kurl.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <QtCore/QMutex>
#include <QtCore/QStringList>
#include <QtNetwork/QSslKey>
#include <QtNetwork/QSslCipher>
#include <QtNetwork/QNetworkProxy>

K_GLOBAL_STATIC(QMutex, ksslsocketInitMutex)
static QList<QSslCertificate> *kdeCaCertificateList;

static void initKSslSocket()
{
    static bool initialized = false;
    QMutexLocker locker(ksslsocketInitMutex);
    if (!initialized) {
        if (!kdeCaCertificateList) {
            kdeCaCertificateList = new QList<QSslCertificate>;
            QSslSocket::setDefaultCaCertificates(*kdeCaCertificateList); // set Qt's set to empty
        }

        if (!KGlobal::hasMainComponent())
            return;                 // we need KGlobal::dirs() available
        initialized = true;

        // set default CAs from KDE's own bundle
        QStringList bundles = KGlobal::dirs()->findAllResources("data", "kssl/ca-bundle.crt");
        foreach (const QString &bundle, bundles) {
            *kdeCaCertificateList += QSslCertificate::fromPath(bundle);
        }
        //kDebug(7029) << "Loading" << kdeCaCertificateList->count() << "CA certificates from" << bundles;
    }
}

static KTcpSocket::SslVersion kSslVersionFromQ(QSsl::SslProtocol protocol)
{
    switch (protocol) {
    case QSsl::SslV2:
        return KTcpSocket::SslV2;
    case QSsl::SslV3:
        return KTcpSocket::SslV3;
    case QSsl::TlsV1:
        return KTcpSocket::TlsV1;
    case QSsl::AnyProtocol:
        return KTcpSocket::AnySslVersion;
    default:
        return KTcpSocket::UnknownSslVersion;
    }
}


static QSsl::SslProtocol qSslProtocolFromK(KTcpSocket::SslVersion sslVersion)
{
    //### this lowlevel bit-banging is a little dangerous and a likely source of bugs
    if (sslVersion == KTcpSocket::AnySslVersion) {
        return QSsl::AnyProtocol;
    }
    //does it contain any valid protocol?
    if (!(sslVersion & (KTcpSocket::SslV2 | KTcpSocket::SslV2 | KTcpSocket::TlsV1))) {
        return QSsl::UnknownProtocol;
    }

    switch (sslVersion) {
    case KTcpSocket::SslV2:
        return QSsl::SslV2;
    case KTcpSocket::SslV3:
        return QSsl::SslV3;
    case KTcpSocket::TlsV1:
        return QSsl::TlsV1;
    default:
        //QSslSocket doesn't really take arbitrary combinations. It's one or all.
        return QSsl::AnyProtocol;
    }
}


//cipher class converter KSslCipher -> QSslCipher
class CipherCc
{
public:
    CipherCc()
    {
        foreach (const QSslCipher &c, QSslSocket::supportedCiphers()) {
            allCiphers.insert(c.name(), c);
        }
    }

    QSslCipher converted(const KSslCipher &ksc)
    {
        return allCiphers.value(ksc.name());
    }

private:
    QHash<QString, QSslCipher> allCiphers;
};


class KSslErrorPrivate
{
public:
    KSslError::Error errorFromQSslError(QSslError::SslError e)
    {
        switch (e) {
        case QSslError::NoError:
            return KSslError::NoError;
        case QSslError::UnableToGetLocalIssuerCertificate:
        case QSslError::InvalidCaCertificate:
            return KSslError::InvalidCertificateAuthority;
        case QSslError::InvalidNotBeforeField:
        case QSslError::InvalidNotAfterField:
        case QSslError::CertificateNotYetValid:
        case QSslError::CertificateExpired:
            return KSslError::ExpiredCertificate;
        case QSslError::UnableToDecodeIssuerPublicKey:
        case QSslError::SubjectIssuerMismatch:
        case QSslError::AuthorityIssuerSerialNumberMismatch:
            return KSslError::InvalidCertificate;
        case QSslError::SelfSignedCertificate:
        case QSslError::SelfSignedCertificateInChain:
            return KSslError::SelfSignedCertificate;
        case QSslError::CertificateRevoked:
            return KSslError::RevokedCertificate;
        case QSslError::InvalidPurpose:
            return KSslError::InvalidCertificatePurpose;
        case QSslError::CertificateUntrusted:
            return KSslError::UntrustedCertificate;
        case QSslError::CertificateRejected:
            return KSslError::RejectedCertificate;
        case QSslError::NoPeerCertificate:
            return KSslError::NoPeerCertificate;
        case QSslError::HostNameMismatch:
            return KSslError::HostNameMismatch;
        case QSslError::UnableToVerifyFirstCertificate:
        case QSslError::UnableToDecryptCertificateSignature:
        case QSslError::UnableToGetIssuerCertificate:
        case QSslError::CertificateSignatureFailed:
            return KSslError::CertificateSignatureFailed;
        case QSslError::PathLengthExceeded:
            return KSslError::PathLengthExceeded;
        case QSslError::UnspecifiedError:
        case QSslError::NoSslSupport:
        default:
            return KSslError::UnknownError;
        }
    }

    KSslError::Error error;
    QString errorString;
    QSslCertificate certificate;
};


KSslError::KSslError(Error errorCode, const QSslCertificate &certificate)
 : d(new KSslErrorPrivate())
{
    d->error = errorCode;
    d->certificate = certificate;
    //TODO do *something* about the error string
}


KSslError::KSslError(const QSslError &other)
 : d(new KSslErrorPrivate())
{
    d->error = d->errorFromQSslError(other.error());
    d->errorString = other.errorString();
    d->certificate = other.certificate();
}


KSslError::KSslError(const KSslError &other)
 : d(new KSslErrorPrivate())
{
    *d = *other.d;
}


KSslError::~KSslError()
{
    delete d;
}


KSslError &KSslError::operator=(const KSslError &other)
{
    *d = *other.d;
    return *this;
}


KSslError::Error KSslError::error() const
{
    return d->error;
}


QString KSslError::errorString() const
{
    return d->errorString;
}


QSslCertificate KSslError::certificate() const
{
    return d->certificate;
}


class KTcpSocketPrivate
{
public:
    KTcpSocketPrivate(KTcpSocket *qq)
     : q(qq),
       emittedReadyRead(false)
    {
        initKSslSocket();

        Q_ASSERT(kdeCaCertificateList);
        sock.setCaCertificates(*kdeCaCertificateList);
    }

    KTcpSocket::State state(QAbstractSocket::SocketState s)
    {
        switch (s) {
        case QAbstractSocket::UnconnectedState:
            return KTcpSocket::UnconnectedState;
        case QAbstractSocket::HostLookupState:
            return KTcpSocket::HostLookupState;
        case QAbstractSocket::ConnectingState:
            return KTcpSocket::ConnectingState;
        case QAbstractSocket::ConnectedState:
            return KTcpSocket::ConnectedState;
        case QAbstractSocket::ClosingState:
            return KTcpSocket::ClosingState;
        case QAbstractSocket::BoundState:
        case QAbstractSocket::ListeningState:
            //### these two are not relevant as long as this can't be a server socket
        default:
            return KTcpSocket::UnconnectedState; //the closest to "error"
        }
    }

    KTcpSocket::EncryptionMode encryptionMode(QSslSocket::SslMode mode)
    {
        switch (mode) {
        case QSslSocket::SslClientMode:
            return KTcpSocket::SslClientMode;
        case QSslSocket::SslServerMode:
            return KTcpSocket::SslServerMode;
        default:
            return KTcpSocket::UnencryptedMode;
        }
    }

    KTcpSocket::Error errorFromAbsSocket(QAbstractSocket::SocketError e)
    {
        switch (e) {
        case QAbstractSocket::ConnectionRefusedError:
            return KTcpSocket::ConnectionRefusedError;
        case QAbstractSocket::RemoteHostClosedError:
            return KTcpSocket::RemoteHostClosedError;
        case QAbstractSocket::HostNotFoundError:
            return KTcpSocket::HostNotFoundError;
        case QAbstractSocket::SocketAccessError:
            return KTcpSocket::SocketAccessError;
        case QAbstractSocket::SocketResourceError:
            return KTcpSocket::SocketResourceError;
        case QAbstractSocket::SocketTimeoutError:
            return KTcpSocket::SocketTimeoutError;
        case QAbstractSocket::NetworkError:
            return KTcpSocket::NetworkError;
        case QAbstractSocket::UnsupportedSocketOperationError:
            return KTcpSocket::UnsupportedSocketOperationError;
        case QAbstractSocket::DatagramTooLargeError:
            //we don't do UDP
        case QAbstractSocket::AddressInUseError:
        case QAbstractSocket::SocketAddressNotAvailableError:
            //### own values if/when we ever get server socket support
        case QAbstractSocket::ProxyAuthenticationRequiredError:
            //### maybe we need an enum value for this
        case QAbstractSocket::UnknownSocketError:
        default:
            return KTcpSocket::UnknownError;
        }
    }

    //private slots
    void reemitSocketError(QAbstractSocket::SocketError e)
    {
        emit q->error(errorFromAbsSocket(e));
    }

    void reemitSslErrors(const QList<QSslError> &errors)
    {
        q->showSslErrors(); //H4X
        emit q->sslErrors(errors);
    }

    void reemitStateChanged(QAbstractSocket::SocketState s)
    {
        emit q->stateChanged(state(s));
    }

    void reemitModeChanged(QSslSocket::SslMode m)
    {
        emit q->encryptionModeChanged(encryptionMode(m));
    }

    // This method is needed because we might emit readyRead() due to this QIODevice
    // having some data buffered, so we need to care about blocking, too.
    //### useless ATM as readyRead() now just calls d->sock.readyRead().
    void reemitReadyRead()
    {
        if (!emittedReadyRead) {
            emittedReadyRead = true;
            emit q->readyRead();
            emittedReadyRead = false;
        }
    }

    KTcpSocket *const q;
    bool emittedReadyRead;
    QSslSocket sock;
    QList<KSslCipher> ciphers;
    KTcpSocket::SslVersion advertisedSslVersion;
    CipherCc ccc;
};


KTcpSocket::KTcpSocket(QObject *parent)
 : QIODevice(parent),
   d(new KTcpSocketPrivate(this))
{
    d->advertisedSslVersion = SslV3;

    connect(&d->sock, SIGNAL(aboutToClose()), this, SIGNAL(aboutToClose()));
    connect(&d->sock, SIGNAL(bytesWritten(qint64)), this, SIGNAL(bytesWritten(qint64)));
    connect(&d->sock, SIGNAL(readyRead()), this, SLOT(reemitReadyRead()));
    connect(&d->sock, SIGNAL(connected()), this, SIGNAL(connected()));
    connect(&d->sock, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
    connect(&d->sock, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)),
            this, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
    connect(&d->sock, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(reemitSocketError(QAbstractSocket::SocketError)));
    connect(&d->sock, SIGNAL(sslErrors(const QList<QSslError> &)),
            this, SLOT(reemitSslErrors(const QList<QSslError> &)));
    connect(&d->sock, SIGNAL(hostFound()), this, SIGNAL(hostFound()));
    connect(&d->sock, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(reemitStateChanged(QAbstractSocket::SocketState)));
    connect(&d->sock, SIGNAL(modeChanged(QSslSocket::SslMode)),
            this, SLOT(reemitModeChanged(QSslSocket::SslMode)));
}


KTcpSocket::~KTcpSocket()
{
    delete d;
}

////////////////////////////// (mostly) virtuals from QIODevice

bool KTcpSocket::atEnd() const
{
    return d->sock.atEnd() && QIODevice::atEnd();
}


qint64 KTcpSocket::bytesAvailable() const
{
    return d->sock.bytesAvailable() + QIODevice::bytesAvailable();
}


qint64 KTcpSocket::bytesToWrite() const
{
    return d->sock.bytesToWrite();
}


bool KTcpSocket::canReadLine() const
{
    return d->sock.canReadLine() || QIODevice::canReadLine();
}


void KTcpSocket::close()
{
    d->sock.close();
    QIODevice::close();
}


bool KTcpSocket::isSequential() const
{
    return true;
}


bool KTcpSocket::open(QIODevice::OpenMode open)
{
    bool ret = d->sock.open(open);
    setOpenMode(d->sock.openMode());
    return ret;
}


bool KTcpSocket::waitForBytesWritten(int msecs)
{
    return d->sock.waitForBytesWritten(msecs);
}


bool KTcpSocket::waitForReadyRead(int msecs)
{
    return d->sock.waitForReadyRead(msecs);
}


qint64 KTcpSocket::readData(char *data, qint64 maxSize)
{
    return d->sock.read(data, maxSize);
}


qint64 KTcpSocket::writeData(const char *data, qint64 maxSize)
{
    return d->sock.write(data, maxSize);
}

////////////////////////////// public methods from QAbstractSocket

void KTcpSocket::abort()
{
    d->sock.abort();
}


void KTcpSocket::connectToHost(const QString &hostName, quint16 port, ProxyPolicy policy)
{
    if (policy == AutoProxy) {
        //###
    }
    d->sock.connectToHost(hostName, port);
    setOpenMode(d->sock.openMode());
}


void KTcpSocket::connectToHost(const QHostAddress &hostAddress, quint16 port, ProxyPolicy policy)
{
    if (policy == AutoProxy) {
        //###
    }
    d->sock.connectToHost(hostAddress, port);
    setOpenMode(d->sock.openMode());
}


void KTcpSocket::connectToHost(const KUrl &url, ProxyPolicy policy)
{
    if (policy == AutoProxy) {
        //###
    }
    d->sock.connectToHost(url.host(), url.port());
    setOpenMode(d->sock.openMode());
}


void KTcpSocket::disconnectFromHost()
{
    d->sock.disconnectFromHost();
    setOpenMode(d->sock.openMode());
}


KTcpSocket::Error KTcpSocket::error() const
{
    return d->errorFromAbsSocket(d->sock.error());
}


QList<KSslError> KTcpSocket::sslErrors() const
{
    //### pretty slow; also consider throwing out duplicate error codes. We may get
    //    duplicates even though there were none in the original list because KSslError
    //    has a smallest common denominator range of SSL error codes.
    QList<KSslError> ret;
    foreach (const QSslError &e, d->sock.sslErrors())
        ret.append(KSslError(e));
    return ret;
}


bool KTcpSocket::flush()
{
    return d->sock.flush();
}


bool KTcpSocket::isValid() const
{
    return d->sock.isValid();
}


QHostAddress KTcpSocket::localAddress() const
{
    return d->sock.localAddress();
}


QHostAddress KTcpSocket::peerAddress() const
{
    return d->sock.peerAddress();
}


QString KTcpSocket::peerName() const
{
    return d->sock.peerName();
}


quint16 KTcpSocket::peerPort() const
{
    return d->sock.peerPort();
}


QNetworkProxy KTcpSocket::proxy() const
{
    return d->sock.proxy();
}


qint64 KTcpSocket::readBufferSize() const
{
    return d->sock.readBufferSize();
}


void KTcpSocket::setProxy(const QNetworkProxy &proxy)
{
    d->sock.setProxy(proxy);
}


void KTcpSocket::setReadBufferSize(qint64 size)
{
    d->sock.setReadBufferSize(size);
}


KTcpSocket::State KTcpSocket::state() const
{
    return d->state(d->sock.state());
}


bool KTcpSocket::waitForConnected(int msecs)
{
    bool ret = d->sock.waitForConnected(msecs);
    setOpenMode(d->sock.openMode());
    return ret;
}


bool KTcpSocket::waitForDisconnected(int msecs)
{
    bool ret = d->sock.waitForDisconnected(msecs);
    setOpenMode(d->sock.openMode());
    return ret;
}

////////////////////////////// public methods from QSslSocket

void KTcpSocket::addCaCertificate(const QSslCertificate &certificate)
{
    d->sock.addCaCertificate(certificate);
}


/*
bool KTcpSocket::addCaCertificates(const QString &path, QSsl::EncodingFormat format,
                                   QRegExp::PatternSyntax syntax)
{
    return d->sock.addCaCertificates(path, format, syntax);
}
*/


void KTcpSocket::addCaCertificates(const QList<QSslCertificate> &certificates)
{
    d->sock.addCaCertificates(certificates);
}


QList<QSslCertificate> KTcpSocket::caCertificates() const
{
    return d->sock.caCertificates();
}


QList<KSslCipher> KTcpSocket::ciphers() const
{
    return d->ciphers;
}


void KTcpSocket::connectToHostEncrypted(const QString &hostName, quint16 port, OpenMode openMode)
{
    d->sock.setProtocol(qSslProtocolFromK(d->advertisedSslVersion));
    d->sock.connectToHostEncrypted(hostName, port, openMode);
    setOpenMode(d->sock.openMode());
}


QSslCertificate KTcpSocket::localCertificate() const
{
    return d->sock.localCertificate();
}


QList<QSslCertificate> KTcpSocket::peerCertificateChain() const
{
    return d->sock.peerCertificateChain();
}


KSslKey KTcpSocket::privateKey() const
{
    return KSslKey(d->sock.privateKey());
}


KSslCipher KTcpSocket::sessionCipher() const
{
    return KSslCipher(d->sock.sessionCipher());
}


void KTcpSocket::setCaCertificates(const QList<QSslCertificate> &certificates)
{
    d->sock.setCaCertificates(certificates);
}


void KTcpSocket::setCiphers(const QList<KSslCipher> &ciphers)
{
    QList<QSslCipher> cl;
    foreach (const KSslCipher &c, d->ciphers) {
        cl.append(d->ccc.converted(c));
    }
    d->sock.setCiphers(cl);
}


void KTcpSocket::setLocalCertificate(const QSslCertificate &certificate)
{
    d->sock.setLocalCertificate(certificate);
}


void KTcpSocket::setLocalCertificate(const QString &fileName, QSsl::EncodingFormat format)
{
    d->sock.setLocalCertificate(fileName, format);
}


//TODO
void KTcpSocket::setPrivateKey(const KSslKey &key)
{
}


//TODO
void KTcpSocket::setPrivateKey(const QString &fileName, KSslKey::Algorithm algorithm,
                               QSsl::EncodingFormat format, const QByteArray &passPhrase)
{
}


bool KTcpSocket::waitForEncrypted(int msecs)
{
    return d->sock.waitForEncrypted(msecs);
}


KTcpSocket::EncryptionMode KTcpSocket::encryptionMode() const
{
    return d->encryptionMode(d->sock.mode());
}


//slot
void KTcpSocket::ignoreSslErrors()
{
    d->sock.ignoreSslErrors();
}


//slot
void KTcpSocket::startClientEncryption()
{
    d->sock.setProtocol(qSslProtocolFromK(d->advertisedSslVersion));
    d->sock.startClientEncryption();
}


//debugging H4X
void KTcpSocket::showSslErrors()
{
	foreach (const QSslError &e, d->sock.sslErrors())
		kDebug(7029) << e.errorString();
}


void KTcpSocket::setAdvertisedSslVersion(KTcpSocket::SslVersion version)
{
    d->advertisedSslVersion = version;
}


KTcpSocket::SslVersion KTcpSocket::advertisedSslVersion() const
{
    return d->advertisedSslVersion;
}


KTcpSocket::SslVersion KTcpSocket::negotiatedSslVersion() const
{
    if (!d->sock.isEncrypted()) {
        return UnknownSslVersion;
    }
    return kSslVersionFromQ(d->sock.protocol());
}


QString KTcpSocket::negotiatedSslVersionName() const
{
    if (!d->sock.isEncrypted()) {
        return QString();
    }
    return d->sock.sessionCipher().protocolString();
}


////////////////////////////// KSslKey

class KSslKeyPrivate
{
public:
    KSslKey::Algorithm convertAlgorithm(QSsl::KeyAlgorithm a)
    {
        switch(a) {
        case QSsl::Dsa:
            return KSslKey::Dsa;
        default:
            return KSslKey::Rsa;
        }
    }

    KSslKey::Algorithm algorithm;
    KSslKey::KeySecrecy secrecy;
    bool isExportable;
    QByteArray der;
};


KSslKey::KSslKey()
 : d(new KSslKeyPrivate)
{
    d->algorithm = Rsa;
    d->secrecy = PublicKey;
    d->isExportable = true;
}


KSslKey::KSslKey(const KSslKey &other)
 : d(new KSslKeyPrivate)
{
    *d = *other.d;
}


KSslKey::KSslKey(const QSslKey &qsk)
 : d(new KSslKeyPrivate)
{
    d->algorithm = d->convertAlgorithm(qsk.algorithm());
    d->secrecy = (qsk.type() == QSsl::PrivateKey) ? PrivateKey : PublicKey;
    d->isExportable = true;
    d->der = qsk.toDer();
}


KSslKey::~KSslKey()
{
    delete d;
}


KSslKey &KSslKey::operator=(const KSslKey &other)
{
    *d = *other.d;
    return *this;
}


KSslKey::Algorithm KSslKey::algorithm() const
{
    return d->algorithm;
}


bool KSslKey::isExportable() const
{
    return d->isExportable;
}


KSslKey::KeySecrecy KSslKey::secrecy() const
{
    return d->secrecy;
}


QByteArray KSslKey::toDer() const
{
    return d->der;
}

////////////////////////////// KSslCipher

//nice-to-have: make implicitly shared
class KSslCipherPrivate
{
public:

    QString authenticationMethod;
    QString encryptionMethod;
    QString keyExchangeMethod;
    QString name;
    bool isNull;
    int supportedBits;
    int usedBits;
};


KSslCipher::KSslCipher()
 : d(new KSslCipherPrivate)
{
    d->isNull = true;
    d->supportedBits = 0;
    d->usedBits = 0;
}


KSslCipher::KSslCipher(const KSslCipher &other)
 : d(new KSslCipherPrivate)
{
    *d = *other.d;
}


KSslCipher::KSslCipher(const QSslCipher &qsc)
 : d(new KSslCipherPrivate)
{
    d->authenticationMethod = qsc.authenticationMethod();
    d->encryptionMethod = qsc.encryptionMethod();
    //Qt likes to append the number of bits (usedBits?) to the algorithm,
    //for example "AES(256)". We only want the pure algorithm name, though.
    int parenIdx = d->encryptionMethod.indexOf('(');
    if (parenIdx > 0)
        d->encryptionMethod.truncate(parenIdx);
    d->keyExchangeMethod = qsc.keyExchangeMethod();
    d->name = qsc.name();
    d->isNull = qsc.isNull();
    d->supportedBits = qsc.supportedBits();
    d->usedBits = qsc.usedBits();
}


KSslCipher::~KSslCipher()
{
    delete d;
}


KSslCipher &KSslCipher::operator=(const KSslCipher &other)
{
    *d = *other.d;
    return *this;
}


bool KSslCipher::isNull() const
{
    return d->isNull;
}


QString KSslCipher::authenticationMethod() const
{
    return d->authenticationMethod;
}


QString KSslCipher::encryptionMethod() const
{
    return d->encryptionMethod;
}


QString KSslCipher::keyExchangeMethod() const
{
    return d->keyExchangeMethod;
}


QString KSslCipher::digestMethod() const
{
    //### This is not really backend neutral. It works for OpenSSL and
    //    for RFC compliant names, though.
    if (d->name.endsWith("SHA"))
        return "SHA-1";
    else if (d->name.endsWith("MD5"))
        return "MD5";
    else
        return "";
}


QString KSslCipher::name() const
{
    return d->name;
}


int KSslCipher::supportedBits() const
{
    return d->supportedBits;
}


int KSslCipher::usedBits() const
{
    return d->usedBits;
}


//static 
QList<KSslCipher> KSslCipher::supportedCiphers()
{
    QList<KSslCipher> ret;
    QList<QSslCipher> candidates = QSslSocket::supportedCiphers();
    foreach(const QSslCipher &c, candidates) {
        ret.append(KSslCipher(c));
    }
    return ret;
}


#include "ktcpsocket.moc"
