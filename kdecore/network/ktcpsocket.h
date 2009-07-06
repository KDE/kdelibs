/* This file is part of the KDE libraries
    Copyright (C) 2007 Thiago Macieira <thiago@kde.org>
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

#ifndef KTCPSOCKET_H
#define KTCPSOCKET_H

#include <QtNetwork/QSslSocket>
//#include <QtCore/QRegExp>

#include "kdecore_export.h"

/*
  Notes on QCA::TLS compatibility
  In order to check for all validation problems as far as possible we need to use:
  Validity QCA::TLS::peerCertificateValidity()
  TLS::IdentityResult QCA::TLS::peerIdentityResult()
  CertificateChain QCA::TLS::peerCertificateChain().validate() - to find the failing cert!
  TLS::Error QCA::TLS::errorCode() - for more generic (but stil SSL) errors
 */


class KSslKeyPrivate;

class KDECORE_EXPORT KSslKey {
public:
    enum Algorithm {
        Rsa = 0,
        Dsa,
        Dh
    };
    enum KeySecrecy {
        PublicKey,
        PrivateKey
    };

    KSslKey();
    KSslKey(const KSslKey &other);
    KSslKey(const QSslKey &sslKey);
    ~KSslKey();
    KSslKey &operator=(const KSslKey &other);

    Algorithm algorithm() const;
    bool isExportable() const;
    KeySecrecy secrecy() const;
    QByteArray toDer() const;
private:
    KSslKeyPrivate *const d;
};


class KSslCipherPrivate;

class KDECORE_EXPORT KSslCipher {
public:
    KSslCipher();
    KSslCipher(const KSslCipher &other);
    KSslCipher(const QSslCipher &);
    ~KSslCipher();
    KSslCipher &operator=(const KSslCipher &other);

    bool isNull() const;
    QString authenticationMethod() const;
    QString encryptionMethod() const;
    QString keyExchangeMethod() const;
    QString digestMethod() const;
    /* mainly for internal use */
    QString name() const;
    int supportedBits() const;
    int usedBits() const;

    static QList<KSslCipher> supportedCiphers();

private:
    KSslCipherPrivate *const d;
};


class KSslErrorPrivate;
class KTcpSocket;

class KDECORE_EXPORT KSslError
{
public:
    enum Error {
        NoError = 0,
        UnknownError,
        InvalidCertificateAuthorityCertificate,
        InvalidCertificate,
        CertificateSignatureFailed,
        SelfSignedCertificate,
        ExpiredCertificate,
        RevokedCertificate,
        InvalidCertificatePurpose,
        RejectedCertificate,
        UntrustedCertificate,
        NoPeerCertificate,
        HostNameMismatch,
        PathLengthExceeded
    };
    KSslError(KSslError::Error error = NoError, const QSslCertificate &cert = QSslCertificate());
    KSslError(const QSslError &error);  //### explicit yes or no?
    KSslError(const KSslError &other);
    ~KSslError();
    KSslError &operator=(const KSslError &other);

    Error error() const;
    QString errorString() const;
    QSslCertificate certificate() const;
private:
    KSslErrorPrivate *const d;
};


//consider killing more convenience functions with huge signatures
//### do we need setSession() / session() ?

//BIG FAT TODO: do we keep openMode() up to date everywhere it can change?

//other TODO: limit possible error strings?, SSL key stuff

//TODO protocol (or maybe even application?) dependent automatic proxy choice

class KTcpSocketPrivate;
class QHostAddress;
class KUrl;

class KDECORE_EXPORT KTcpSocket: public QIODevice
{
    Q_OBJECT
public:
    enum State {
        UnconnectedState = 0,
        HostLookupState,
        ConnectingState,
        ConnectedState,
        BoundState,
        ListeningState,
        ClosingState
        //hmmm, do we need an SslNegotiatingState?
    };
    enum SslVersion {
        UnknownSslVersion = 0x01,
        SslV2 = 0x02,
        SslV3 = 0x04,
        TlsV1 = 0x08,
        SslV3_1 = 0x08,
        AnySslVersion = SslV2 | SslV3 | TlsV1
    };
    Q_DECLARE_FLAGS(SslVersions, SslVersion)
    enum Error {
        UnknownError = 0,
        ConnectionRefusedError,
        RemoteHostClosedError,
        HostNotFoundError,
        SocketAccessError,
        SocketResourceError,
        SocketTimeoutError,
        NetworkError,
        UnsupportedSocketOperationError
    };
/*
The following is based on reading the OpenSSL interface code of both QSslSocket
and QCA::TLS. Barring oversights it should be accurate. The two cases with the
question marks apparently will never be emitted by QSslSocket so there is nothing
to compare.

QSslError::NoError                                  KTcpSocket::NoError
QSslError::UnableToGetIssuerCertificate             QCA::ErrorSignatureFailed
QSslError::UnableToDecryptCertificateSignature      QCA::ErrorSignatureFailed
QSslError::UnableToDecodeIssuerPublicKey            QCA::ErrorInvalidCA
QSslError::CertificateSignatureFailed               QCA::ErrorSignatureFailed
QSslError::CertificateNotYetValid                   QCA::ErrorExpired
QSslError::CertificateExpired                       QCA::ErrorExpired
QSslError::InvalidNotBeforeField                    QCA::ErrorExpired
QSslError::InvalidNotAfterField                     QCA::ErrorExpired
QSslError::SelfSignedCertificate                    QCA::ErrorSelfSigned
QSslError::SelfSignedCertificateInChain             QCA::ErrorSelfSigned
QSslError::UnableToGetLocalIssuerCertificate        QCA::ErrorInvalidCA
QSslError::UnableToVerifyFirstCertificate           QCA::ErrorSignatureFailed
QSslError::CertificateRevoked                       QCA::ErrorRevoked
QSslError::InvalidCaCertificate                     QCA::ErrorInvalidCA
QSslError::PathLengthExceeded                       QCA::ErrorPathLengthExceeded
QSslError::InvalidPurpose                           QCA::ErrorInvalidPurpose
QSslError::CertificateUntrusted                     QCA::ErrorUntrusted
QSslError::CertificateRejected                      QCA::ErrorRejected
QSslError::SubjectIssuerMismatch                    QCA::TLS::InvalidCertificate ?
QSslError::AuthorityIssuerSerialNumberMismatch      QCA::TLS::InvalidCertificate ?
QSslError::NoPeerCertificate                        QCA::TLS::NoCertificate
QSslError::HostNameMismatch                         QCA::TLS::HostMismatch
QSslError::UnspecifiedError                         KTcpSocket::UnknownError
QSslError::NoSslSupport                             Never happens :)
 */
    enum EncryptionMode {
        UnencryptedMode = 0,
        SslClientMode,
        SslServerMode //### not implemented
    };
    enum ProxyPolicy {
        /// Use the proxy that KProtocolManager suggests for the connection parameters given.
        AutoProxy = 0,
        /// Use the proxy set by setProxy(), if any; otherwise use no proxy.
        ManualProxy
    };

    KTcpSocket(QObject *parent = 0);
    ~KTcpSocket();

    //from QIODevice
    //reimplemented virtuals - the ones not reimplemented are OK for us
    virtual bool atEnd() const;
    virtual qint64 bytesAvailable() const;
    virtual qint64 bytesToWrite() const;
    virtual bool canReadLine() const;
    virtual void close();
    virtual bool isSequential() const;
    virtual bool open(QIODevice::OpenMode open);
    virtual bool waitForBytesWritten(int msecs);
    //### Document that this actually tries to read *more* data
    virtual bool waitForReadyRead(int msecs = 30000);
protected:
    virtual qint64 readData (char *data, qint64 maxSize);
    virtual qint64 writeData (const char *data, qint64 maxSize);
public:
    //from QAbstractSocket
    void abort();
    void connectToHost(const QString &hostName, quint16 port, ProxyPolicy policy = AutoProxy);
    void connectToHost(const QHostAddress &hostAddress, quint16 port, ProxyPolicy policy = AutoProxy);

    /**
     * Take the hostname and port from @p url and connect to them. The information from a
     * full URL enables the most accurate choice of proxy in case of proxy rules that
     * depend on high-level information like protocol or username.
     * @see KProtocolManager::proxyForUrl()
     */
    void connectToHost(const KUrl &url, ProxyPolicy policy = AutoProxy);
    void disconnectFromHost();
    Error error() const; //### QAbstractSocket's model is strange. error() should be related to the
                         //current state and *NOT* just report the last error if there was one.
    QList<KSslError> sslErrors() const; //### the errors returned can only have a subset of all
                                //possible QSslError::SslError enum values depending on backend
    bool flush();
    bool isValid() const;
    QHostAddress localAddress() const;
    QHostAddress peerAddress() const;
    QString peerName() const;
    quint16 peerPort() const;

    /**
     * @see: connectToHost()
     */
    QNetworkProxy proxy() const;
    qint64 readBufferSize() const; //probably hard to implement correctly

    /**
     * @see: connectToHost()
     */
    void setProxy(const QNetworkProxy &proxy); //people actually seem to need it
    void setReadBufferSize(qint64 size);
    State state() const;
    bool waitForConnected(int msecs = 30000);
    bool waitForDisconnected(int msecs = 30000);

    //from QSslSocket
    void addCaCertificate(const QSslCertificate &certificate);
//    bool addCaCertificates(const QString &path, QSsl::EncodingFormat format = QSsl::Pem,
//                           QRegExp::PatternSyntax syntax = QRegExp::FixedString);
    void addCaCertificates(const QList<QSslCertificate> &certificates);
    QList<QSslCertificate> caCertificates() const;
    QList<KSslCipher> ciphers() const;
    void connectToHostEncrypted(const QString &hostName, quint16 port, OpenMode openMode = ReadWrite);
    // bool isEncrypted() const { return encryptionMode() != UnencryptedMode }
    QSslCertificate localCertificate() const;
    QList<QSslCertificate> peerCertificateChain() const;
    KSslKey privateKey() const;
    KSslCipher sessionCipher() const;
    void setCaCertificates(const QList<QSslCertificate> &certificates);
    void setCiphers(const QList<KSslCipher> &ciphers);
    //### void setCiphers(const QString &ciphers); //what about i18n?
    void setLocalCertificate(const QSslCertificate &certificate);
    void setLocalCertificate(const QString &fileName, QSsl::EncodingFormat format = QSsl::Pem);
    void setPrivateKey(const KSslKey &key); //implement
    void setPrivateKey(const QString &fileName, KSslKey::Algorithm algorithm = KSslKey::Rsa,
                       QSsl::EncodingFormat format = QSsl::Pem,
                       const QByteArray &passPhrase = QByteArray()); //TODO
    void setAdvertisedSslVersion(SslVersion version);
    SslVersion advertisedSslVersion() const;    //always equal to last setSslAdvertisedVersion
    SslVersion negotiatedSslVersion() const;     //negotiated version; downgrades are possible.
    QString negotiatedSslVersionName() const;
    bool waitForEncrypted(int msecs = 30000);

    EncryptionMode encryptionMode() const;

Q_SIGNALS:
    //from QIODevice
    void aboutToClose();
    void bytesWritten(qint64 bytes);
    void readyRead();

    //from QAbstractSocket
    void connected();
    void disconnected();
    void error(KTcpSocket::Error);
    void hostFound();
    void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator);
    // only for raw socket state, SSL is separate
    void stateChanged(State);

    //from QSslSocket
    void encrypted();
    void encryptionModeChanged(EncryptionMode);
    void sslErrors(const QList<KSslError> &errors);

public Q_SLOTS:
    void ignoreSslErrors();
    void startClientEncryption();
    // void startServerEncryption(); //not implemented
private:
    Q_PRIVATE_SLOT(d, void reemitReadyRead())
    Q_PRIVATE_SLOT(d, void reemitSocketError(QAbstractSocket::SocketError))
    Q_PRIVATE_SLOT(d, void reemitSslErrors(const QList<QSslError> &))
    Q_PRIVATE_SLOT(d, void reemitStateChanged(QAbstractSocket::SocketState))
    Q_PRIVATE_SLOT(d, void reemitModeChanged(QSslSocket::SslMode))

//debugging H4X
    void showSslErrors();

    friend class KTcpSocketPrivate;
    KTcpSocketPrivate *const d;
};


/**
 * This class can hold all the necessary data from a KTcpSocket to ask the user
 * to continue connecting in the face of SSL errors.
 * It can be used to carry the data for the UI over time or over thread boundaries.
 *
 * @see: KSslCertificateManager::askIgnoreSslErrors()
 */
class KDECORE_EXPORT KSslErrorUiData
{
public:
    /**
     * Default construct an instance with no useful data.
     */
    KSslErrorUiData();
    /**
     * Create an instance and initialize it with SSL error data from @p socket.
     */
    KSslErrorUiData(const KTcpSocket *socket);
    KSslErrorUiData(const KSslErrorUiData &other);
    KSslErrorUiData &operator=(const KSslErrorUiData &);
    class Private;
private:
    friend class Private;
    Private *const d;
};


#endif // KTCPSOCKET_H
