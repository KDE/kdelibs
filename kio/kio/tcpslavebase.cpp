/*
 * Copyright (C) 2000 Alex Zepeda <zipzippy@sonic.net
 * Copyright (C) 2001-2003 George Staikos <staikos@kde.org>
 * Copyright (C) 2001 Dawit Alemayehu <adawit@kde.org>
 *
 * This file is part of the KDE project
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

#include "tcpslavebase.h"

#include <config.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <time.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#include <kdebug.h>
#include <ksslcertificatemanager.h>
#include <ksslsettings.h>
#include <kmessagebox.h>
#include <network/ktcpsocket.h>

#include <klocale.h>
#include <QtCore/QDataStream>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostInfo>
#include <QtDBus/QtDBus>

#include <kapplication.h>
#include <ktoolinvocation.h>
#include <ksocketfactory.h>
#include <kprotocolmanager.h>

using namespace KIO;
//using namespace KNetwork;

typedef QMap<QString, QString> StringStringMap;
Q_DECLARE_METATYPE(StringStringMap)

//TODO Proxy support whichever way works; KPAC reportedly does *not* work.
//NOTE kded_proxyscout may or may not be interesting

//TODO resurrect SSL session recycling; this means save the session on disconnect and look
//for a reusable session on connect. Consider how HTTP persistent connections interact with that.

//TODO in case we support SSL-lessness we need static KTcpSocket::sslAvailable() and check it
//in most places we ATM check for d->isSSL.

//TODO check if d->isBlocking is honored everywhere it makes sense

//TODO fold KSSLSetting and KSSLCertificateHome into KSslSettings, and figure out how to manipulate it.

//TODO recognize partially encrypted websites as "somewhat safe"

//### isAutoSsl() - the name is pretty stupid. isSslFromStart()? isSslAfterConnect()?

/* List of dialogs/messageboxes we need to use (current code location in parentheses)
   Can the design of the "dontAskAgainName" thing be improved?
   Ugh, there *is* some code for application side messageboxes. Use it.

 - "SSLCertDialog" [select client cert] (SlaveInterface)
 - Enter password for client certificate (inline)
 - Password for client cert was wrong. Please reenter. (inline)
 - Setting client cert failed. [doesn't give reason] (inline)
 - "SSLInfoDialog" [mostly server cert info] (SlaveInterface)
 - You are about to enter secure mode. Security information/Display SSL information/Connect (inline)
 - You are about to leave secure mode. Security information/Continue loading/Abort (inline)
 - Hostname mismatch: Continue/Details/Cancel (inline)
 - IP address mismatch: Continue/Details/Cancel (inline)
 - Certificate failed authenticity check: Continue/Details/Cancel (inline)
 - Would you like to accept this certificate forever: Yes/No/Current sessions only (inline)
 */


/** @internal */
class TCPSlaveBase::TcpSlaveBasePrivate
{
public:

    TcpSlaveBasePrivate()
     : autoSSL(false),
       sslNoUi(false)
    {}
    ~TcpSlaveBasePrivate() {}

    QList<KSslError> nonIgnorableErrors(const QList<KSslError> &e) const
    {
        QList<KSslError> ret;
        //TODO :)
        return ret;
    }

    QString host;
    QString ip;
    KTcpSocket socket;

    int timeout;
    bool isBlocking;

    quint16 port;
    QByteArray serviceName;

    KSSLSettings sslSettings;
    bool usingSSL;
    bool autoSSL;
    bool sslNoUi; // If true, we just drop the connection silently
                  // if SSL certificate check fails in some way.
};


//### uh, is this a good idea??
KTcpSocket *TCPSlaveBase::socket() const
{
    return &d->socket;
}


TCPSlaveBase::TCPSlaveBase(const QByteArray &protocol,
                           const QByteArray &poolSocket,
                           const QByteArray &appSocket,
                           bool autoSSL)
 : SlaveBase(protocol, poolSocket, appSocket),
   d(new TcpSlaveBasePrivate)
{
    d->serviceName = protocol;
    d->usingSSL = false;
    d->timeout = KProtocolManager::connectTimeout();
    d->isBlocking = false;
    d->autoSSL = autoSSL;
}


TCPSlaveBase::~TCPSlaveBase()
{
    delete d;
}


ssize_t TCPSlaveBase::write(const char *data, ssize_t len)
{
    ssize_t written = d->socket.write(data, len);
    if (written == -1) {
        kDebug(7029) << "d->socket.write() returned -1! Socket error is"
                     << d->socket.error() << ", Socket state is" << d->socket.state();
    }

    bool success = false;
    if (d->isBlocking) {
        // Drain the tx buffer
        success = d->socket.waitForBytesWritten(-1);
    } else {
        // ### I don't know how to make sure that all data does get written at some point
        // without doing it now. There is no event loop to do it behind the scenes.
        // Polling in the dispatch() loop? Something timeout based?
        success = d->socket.waitForBytesWritten(0);
    }

    d->socket.flush();  //this *ahem* might help to get the data on the wire faster

    if (d->socket.state() != KTcpSocket::ConnectedState && !success) {
        kDebug(7029) << "Write failed, will return -1! Socket error is"
                     << d->socket.error() << ", Socket state is" << d->socket.state()
                     << "Return value of waitForBytesWritten() is" << success;
        return -1;
    }

    return written;
}


ssize_t TCPSlaveBase::read(char* data, ssize_t len)
{
    if (d->usingSSL && (d->socket.encryptionMode() != KTcpSocket::SslClientMode)) {
        setMetaData("ssl_in_use", "FALSE");
        kDebug(7029) << "lost SSL connection.";
        return -1;
    }

    if (d->isBlocking) {
        d->socket.waitForReadyRead(-1);
    } else {
        d->socket.waitForReadyRead(0);
    }

    return d->socket.read(data, len);
}


ssize_t TCPSlaveBase::readLine(char *data, ssize_t len)
{
    if (d->usingSSL && (d->socket.encryptionMode() != KTcpSocket::SslClientMode)) {
        setMetaData("ssl_in_use", "FALSE");
        kDebug(7029) << "lost SSL connection.";
        return -1;
    }

    if (d->isBlocking) {
        d->socket.waitForReadyRead(-1);
    } else {
        d->socket.waitForReadyRead(0);
    }

    return d->socket.readLine(data, len);
}

// This function is simply a wrapper to establish the connection
// to the server.  It's a bit more complicated than ::connect
// because we first have to check to see if the user specified
// a port, and if so use it, otherwise we check to see if there
// is a port specified in /etc/services, and if so use that
// otherwise as a last resort use the supplied default port.
bool TCPSlaveBase::connectToHost(const QString &protocol,
                                 const QString &host,
                                 quint16 port)
{
    disconnectFromHost();  //Reset some state, even if we are already disconnected

    setMetaData("ssl_in_use", "FALSE"); //We have separate connection and SSL setup phases

    //  - leaving SSL - warn before we even connect
    //### see if it makes sense to move this into the HTTP ioslave which is the only
    //    user.
    if (metaData("main_frame_request") == "TRUE"  //### this looks *really* unreliable
          && metaData("ssl_activate_warnings") == "TRUE"
          && metaData("ssl_was_in_use") == "TRUE"
          && !d->autoSSL) {
        KSSLSettings kss;
        if (kss.warnOnLeave()) {
            int result = messageBox(i18n("You are about to leave secure "
                                         "mode. Transmissions will no "
                                         "longer be encrypted.\nThis "
                                         "means that a third party could "
                                         "observe your data in transit."),
                                    WarningContinueCancel,
                                    i18n("Security Information"),
                                    i18n("C&ontinue Loading"), QString(),
                                    "WarnOnLeaveSSLMode");

            if (result == KMessageBox::Cancel) {
                return false;
            }
        }
    }

    d->host = host;

    //FIXME! KTcpSocket doesn't know or care about protocol ports! Fix it there, then use it here.

    kDebug(7029) << "before connectToHost: Socket error is "
                 << d->socket.error() << ", Socket state is " << d->socket.state();
    d->socket.connectToHost(host, port);
    kDebug(7029) << "after connectToHost: Socket error is "
                 << d->socket.error() << ", Socket state is " << d->socket.state();

    bool connectOk = d->socket.waitForConnected(d->timeout > -1 ? d->timeout * 1000 : -1);
    kDebug(7029) << "after waitForConnected: Socket error is "
                 << d->socket.error() << ", Socket state is " << d->socket.state()
                 << ", waitForConnected returned " << connectOk;

    if (d->socket.state() != KTcpSocket::ConnectedState) {
        if (d->socket.error() == KTcpSocket::HostNotFoundError) {
            error(ERR_UNKNOWN_HOST,
                  host + QLatin1String(": ") + d->socket.errorString());
        } else {
            error(ERR_COULD_NOT_CONNECT,
                  host + QLatin1String(": ") + d->socket.errorString());
        }
        return false;
    }

    //### check for proxyAuthenticationRequiredError

    if (d->autoSSL) {
        SslResult res = startTLSInternal();
        if (res == ResultFailed) {
            //### more?
            return false;
        }
    }
    // store the IP for later
    d->ip = d->socket.peerAddress().toString();
    d->port = d->socket.peerPort();

    return true;
}

void TCPSlaveBase::disconnectFromHost()
{
    kDebug(7029);
    d->ip.clear();
    d->host.clear();
    d->usingSSL = false;

    if (d->socket.state() == KTcpSocket::UnconnectedState)
        return;

    //stopTLS(); ### this is a reminder to save a session for reuse or do whatever on SSL shutdown
    d->socket.disconnectFromHost();
    if (d->socket.state() != KTcpSocket::UnconnectedState)
        d->socket.waitForDisconnected(-1); // wait for unsent data to be sent
    d->socket.close(); //whatever that means on a socket
}

bool TCPSlaveBase::isAutoSsl() const
{
    return d->autoSSL;
}

bool TCPSlaveBase::isUsingSsl() const
{
    return d->usingSSL;
}

quint16 TCPSlaveBase::port() const
{
    return d->port;
}

bool TCPSlaveBase::atEnd() const
{
    return d->socket.atEnd();
}

bool TCPSlaveBase::startSsl()
{
    if (d->usingSSL)
        return false;
    return startTLSInternal() != ResultFailed;
}


TCPSlaveBase::SslResult TCPSlaveBase::startTLSInternal()
{
    selectClientCertificate();

    //setMetaData("ssl_session_id", d->kssl->session()->toString());
    //### we don't support session reuse for now...

    d->usingSSL = true;
    setMetaData("ssl_in_use", "TRUE");

    /* Usually ignoreSslErrors() would be called in the slot invoked by the sslErrors()
       signal but that would mess up the flow of control. We will check for errors
       anyway to decide if we want to continue connecting. Otherwise ignoreSslErrors()
       before connecting would be rather insecure. */
    d->socket.ignoreSslErrors();
    d->socket.startClientEncryption();
    d->socket.waitForEncrypted(-1);

    if (d->socket.encryptionMode() != KTcpSocket::SslClientMode) { //TODO user visible diagnostics
        d->usingSSL = false;
        setMetaData("ssl_in_use", "FALSE");
        kDebug(7029) << "initial SSL handshake failed.";
        return ResultFailed;
    }

    //Set metadata, among other things for the "SSL Details" dialog
    KSslCipher cipher = d->socket.sessionCipher();

    kDebug(7029) << "Cipher info - "
                 << " authenticationMethod:" << cipher.authenticationMethod()
                 << " encryptionMethod:" << cipher.encryptionMethod()
                 << " keyExchangeMethod:" << cipher.keyExchangeMethod()
                 << " name:" << cipher.name()
                 << " protocol:" << cipher.protocol()
                 << " protocolName:" << cipher.protocolName()
                 << " supportedBits:" << cipher.supportedBits()
                 << " usedBits:" << cipher.usedBits();

    setMetaData("ssl_protocol_version", cipher.protocolName());
    QString sslCipher = cipher.encryptionMethod() + '\n';
    sslCipher += cipher.authenticationMethod() + '\n';
    sslCipher += cipher.keyExchangeMethod() + '\n';
    sslCipher += cipher.digestMethod();
    setMetaData("ssl_cipher", sslCipher);
    setMetaData("ssl_cipher_used_bits", QString::number(cipher.usedBits()));
    setMetaData("ssl_cipher_bits", QString::number(cipher.supportedBits()));
    setMetaData("ssl_peer_ip", d->ip);

    //TODO Qt 4.4 actually has non-null certificates in QSslError::certificate() so
    //     we should use that then
    QString errorStr;
    foreach(const KSslError &se, d->socket.sslErrors())
        errorStr += QString::number(static_cast<int>(se.error())) + '\n';
    setMetaData("ssl_cert_errors", errorStr);

    QString peerCertChain;
    foreach (const QSslCertificate &cert, d->socket.peerCertificateChain()) {
        peerCertChain.append(cert.toPem());
        peerCertChain.append('\x01');
    }
    peerCertChain.chop(1);
    setMetaData("ssl_peer_chain", peerCertChain);
    //Do this before the other side needs the data, e.g. for the SSL info dialog
    sendMetaData();

    SslResult rc = verifyServerCertificate();
    if (rc == ResultFailed) {
        d->usingSSL = false;
        setMetaData("ssl_in_use", "FALSE");
        kDebug(7029) << "server certificate verification failed.";
        d->socket.disconnectFromHost();     //Make the connection fail (cf. ignoreSslErrors())
        return ResultFailed;
    } else if (rc == ResultOverridden) {
        kDebug(7029) << "server certificate verification failed but continuing at user's request.";
    }

    //"warn" when starting SSL/TLS
    if (metaData("ssl_activate_warnings") == "TRUE"
        && metaData("ssl_was_in_use") == "FALSE"
        && d->sslSettings.warnOnEnter()) {

        int msgResult = messageBox(i18n("You are about to enter secure mode. "
                                        "All transmissions will be encrypted "
                                        "unless otherwise noted.\nThis means "
                                        "that no third party will be able to "
                                        "easily observe your data in transit."),
                                   WarningYesNo,
                                   i18n("Security Information"),
                                   i18n("Display SSL &Information"),
                                   i18n("C&onnect"),
                                   "WarnOnEnterSSLMode");
        if (msgResult == KMessageBox::Yes) {
            messageBox(SSLMessageBox /*==the SSL info dialog*/, QString());
        }
    }

    return rc;
}


void TCPSlaveBase::selectClientCertificate()
{
#if 0 //hehe
    QString certname;   // the cert to use this session
    bool send = false, prompt = false, save = false, forcePrompt = false;
    KSSLCertificateHome::KSSLAuthAction aa;

    setMetaData("ssl_using_client_cert", "FALSE"); // we change this if needed

    if (metaData("ssl_no_client_cert") == "TRUE") return;
    forcePrompt = (metaData("ssl_force_cert_prompt") == "TRUE");

    // Delete the old cert since we're certainly done with it now
    if (d->pkcs) {
        delete d->pkcs;
        d->pkcs = NULL;
    }

    if (!d->kssl) return;

    // Look for a general certificate
    if (!forcePrompt) {
        certname = KSSLCertificateHome::getDefaultCertificateName(&aa);
        switch (aa) {
        case KSSLCertificateHome::AuthSend:
            send = true; prompt = false;
            break;
        case KSSLCertificateHome::AuthDont:
            send = false; prompt = false;
            certname.clear();
            break;
        case KSSLCertificateHome::AuthPrompt:
            send = false; prompt = true;
            break;
        default:
            break;
        }
    }

    // Look for a certificate on a per-host basis as an override
    QString tmpcn = KSSLCertificateHome::getDefaultCertificateName(d->host, &aa);
    if (aa != KSSLCertificateHome::AuthNone) {   // we must override
        switch (aa) {
        case KSSLCertificateHome::AuthSend:
            send = true;
            prompt = false;
            certname = tmpcn;
            break;
        case KSSLCertificateHome::AuthDont:
            send = false;
            prompt = false;
            certname.clear();
            break;
        case KSSLCertificateHome::AuthPrompt:
            send = false;
            prompt = true;
            certname = tmpcn;
            break;
        default:
            break;
        }
    }

    // Finally, we allow the application to override anything.
    if (hasMetaData("ssl_demand_certificate")) {
        certname = metaData("ssl_demand_certificate");
        if (!certname.isEmpty()) {
            forcePrompt = false;
            prompt = false;
            send = true;
        }
    }

    if (certname.isEmpty() && !prompt && !forcePrompt) return;

    // Ok, we're supposed to prompt the user....
    if (prompt || forcePrompt) {
        QStringList certs = KSSLCertificateHome::getCertificateList();

        QStringList::Iterator it = certs.begin();
        while (it != certs.end()) {
            KSSLPKCS12 *pkcs = KSSLCertificateHome::getCertificateByName(*it);
            if (pkcs && (!pkcs->getCertificate() ||
                         !pkcs->getCertificate()->x509V3Extensions().certTypeSSLClient())) {
                it = certs.erase(it);
            } else {
                ++it;
            }
            delete pkcs;
        }

        if (certs.isEmpty()) return;  // we had nothing else, and prompt failed

        if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kio.uiserver")) {
            KToolInvocation::startServiceByDesktopPath("kuiserver.desktop",
                    QStringList());
        }

        QDBusInterface uis("org.kde.kio.uiserver", "/UIServer", "org.kde.KIO.UIServer");

        QDBusMessage retVal = uis.call("showSSLCertDialog", d->host, certs, metaData("window-id").toLongLong());
        if (retVal.type() == QDBusMessage::ReplyMessage) {
            if (retVal.arguments().at(0).toBool()) {
                send = retVal.arguments().at(1).toBool();
                save = retVal.arguments().at(2).toBool();
                certname = retVal.arguments().at(3).toString();
            }
        }
    }

    // The user may have said to not send the certificate,
    // but to save the choice
    if (!send) {
        if (save) {
            KSSLCertificateHome::setDefaultCertificate(certname, d->host,
                    false, false);
        }
        return;
    }

    // We're almost committed.  If we can read the cert, we'll send it now.
    KSSLPKCS12 *pkcs = KSSLCertificateHome::getCertificateByName(certname);
    if (!pkcs && KSSLCertificateHome::hasCertificateByName(certname)) {           // We need the password
        KIO::AuthInfo ai;
        bool first = true;
        do {
            ai.prompt = i18n("Enter the certificate password:");
            ai.caption = i18n("SSL Certificate Password");
            ai.url.setProtocol("kssl");
            ai.url.setHost(certname);
            ai.username = certname;
            ai.keepPassword = true;

            bool showprompt;
            if (first)
                showprompt = !checkCachedAuthentication(ai);
            else
                showprompt = true;
            if (showprompt) {
                if (!openPasswordDialog(ai, first ? QString() :
                                        i18n("Unable to open the certificate. Try a new password?")))
                    break;
            }

            first = false;
            pkcs = KSSLCertificateHome::getCertificateByName(certname, ai.password);
        } while (!pkcs);

    }

    // If we could open the certificate, let's send it
    if (pkcs) {
        if (!d->kssl->setClientCertificate(pkcs)) {
            messageBox(Information, i18n("The procedure to set the "
                                         "client certificate for the session "
                                         "failed."), i18n("SSL"));
            delete pkcs;  // we don't need this anymore
            pkcs = 0L;
        } else {
            kDebug(7029) << "Client SSL certificate is being used.";
            setMetaData("ssl_using_client_cert", "TRUE");
            if (save) {
                KSSLCertificateHome::setDefaultCertificate(certname, d->host,
                        true, false);
            }
        }
        d->pkcs = pkcs;
    }
#endif
}


TCPSlaveBase::SslResult TCPSlaveBase::verifyServerCertificate()
{
    d->sslNoUi = hasMetaData("ssl_no_ui") && (metaData("ssl_no_ui") != "FALSE");

    QList<KSslError> se = d->socket.sslErrors();
    if (se.isEmpty())
        return ResultOk;

    if (d->sslNoUi)
        return ResultFailed;

    QString message = i18n("The server failed the authenticity check (%1)\n\n",
                           d->host);

    foreach (const KSslError &err, se) {
        //### use our own wording that is "closer to the user"
        message.append(err.errorString());
        message.append('\n');
    }

    //### Consider that hostname mismatch and e.g. an expired certificate are very different.
    //    Maybe there should be no option to acceptForever a cert with bad hostname.

    //### the truth of the following comment is disputed
    //### The old code also checks if the CN (==domain name, here) matches the actual peer IP.
    //This is not a feature of the SSL infrastructure itself so we need to do some work.

    /* We need a list of ignorable errors. I don't think it makes sense to ignore
       malformed certificates, for example, as other environments probably don't do
       that either. It would be similar to a compiler trying to correct syntax errors.
       There are also hard errors that are just impossible to override.
      Ignorable so far, determined by watching real connection errors:
        -UnableToGetLocalIssuerCertificate
        -CertificateUntrusted
        -UnableToVerifyFirstCertificate
    */

    KSslCertificateManager *const cm = KSslCertificateManager::self();
    KSslCertificateRule rule = cm->rule(d->socket.peerCertificateChain().first(), d->host);

    //TODO put nonIgnorableErrors into the cert manager
    QList<KSslError> fatalErrors = d->nonIgnorableErrors(se);
    if (!fatalErrors.isEmpty()) {
        //TODO message "sorry, fatal error, you can't override it"
        return ResultFailed;
    }

    //throw out previously seen errors that are supposed to be ignored.
    se = rule.filterErrors(se);
    if (se.isEmpty()) {
        kDebug(7029) << "Error list empty after removing errors to be ignored. Continuing.";
        return ResultOverridden;
    }

    //### We don't present the option to permanently reject the certificate even though
    //    it would be technically possible.

    int msgResult;
    do {
        msgResult = messageBox(WarningYesNoCancel, message,
                               i18n("Server Authentication"),
                               i18n("&Details"), i18n("Co&ntinue"));
        if (msgResult == KMessageBox::Yes) {
            //Details was chosen - we pop up the details dialog and present the choices again
            messageBox(SSLMessageBox /*==the SSL info dialog*/, QString());
        } else if (msgResult == KMessageBox::Cancel) {
            return ResultFailed;
        }
        //fall through on KMessageBox::No
    } while (msgResult == KMessageBox::Yes);

    //Save the user's choice to ignore the SSL errors.

    msgResult = messageBox(WarningYesNo,
                            i18n("Would you like to accept this "
                                 "certificate forever without "
                                 "being prompted?"),
                            i18n("Server Authentication"),
                            i18n("&Forever"),
                            i18n("&Current Sessions Only"));
    QDateTime ruleExpiry = QDateTime::currentDateTime();
    if (msgResult == KMessageBox::Yes) {
        //accept forever (we interpret this as "for a very long time")
        ruleExpiry = ruleExpiry.addYears(1000);
    } else {
        //accept for a short time, say half an hour.
        ruleExpiry = ruleExpiry.addSecs(30*60);
    }

    //TODO special cases for wildcard domain name in the certificate!
    //rule = KSslCertificateRule(d->socket.peerCertificateChain().first(), whatever);

    rule.setExpiryDateTime(ruleExpiry);
    rule.setIgnoredErrors(se);
    cm->setRule(rule);

    return ResultOverridden;
#if 0 //### need to to do something like the old code about the main and subframe stuff
    kDebug(7029) << "SSL HTTP frame the parent? " << metaData("main_frame_request");
    if (!hasMetaData("main_frame_request") || metaData("main_frame_request") == "TRUE") {
        // Since we're the parent, we need to teach the child.
        setMetaData("ssl_parent_ip", d->ip);
        setMetaData("ssl_parent_cert", pc.toString());
        //  - Read from cache and see if there is a policy for this
        KSSLCertificateCache::KSSLCertificatePolicy cp =
            d->certCache->getPolicyByCertificate(pc);

        //  - validation code
        if (ksv != KSSLCertificate::Ok) {
            if (d->sslNoUi) {
                return -1;
            }

            if (cp == KSSLCertificateCache::Unknown ||
                    cp == KSSLCertificateCache::Ambiguous) {
                cp = KSSLCertificateCache::Prompt;
            } else {
                // A policy was already set so let's honor that.
                permacache = d->certCache->isPermanent(pc);
            }

            if (!_IPmatchesCN && cp == KSSLCertificateCache::Accept) {
                cp = KSSLCertificateCache::Prompt;
//            ksv = KSSLCertificate::Ok;
            }

////// SNIP SNIP //////////

        //  - cache the results
        d->certCache->addCertificate(pc, cp, permacache);
        if (doAddHost) d->certCache->addHost(pc, d->host);
    } else {    // Child frame
        //  - Read from cache and see if there is a policy for this
        KSSLCertificateCache::KSSLCertificatePolicy cp =
            d->certCache->getPolicyByCertificate(pc);
        isChild = true;

        // Check the cert and IP to make sure they're the same
        // as the parent frame
        bool certAndIPTheSame = (d->ip == metaData("ssl_parent_ip") &&
                                 pc.toString() == metaData("ssl_parent_cert"));

        if (ksv == KSSLCertificate::Ok) {
            if (certAndIPTheSame) {       // success
                rc = 1;
                setMetaData("ssl_action", "accept");
            } else {
                /*
                if (d->sslNoUi) {
                  return -1;
                }
                result = messageBox(WarningYesNo,
                                    i18n("The certificate is valid but does not appear to have been assigned to this server.  Do you wish to continue loading?"),
                                    i18n("Server Authentication"));
                if (result == KMessageBox::Yes) {     // success
                  rc = 1;
                  setMetaData("ssl_action", "accept");
                } else {    // fail
                  rc = -1;
                  setMetaData("ssl_action", "reject");
                }
                */
                setMetaData("ssl_action", "accept");
                rc = 1;   // Let's accept this now.  It's bad, but at least the user
                // will see potential attacks in KDE3 with the pseudo-lock
                // icon on the toolbar, and can investigate with the RMB
            }
        } else {
            if (d->sslNoUi) {
                return -1;
            }

            if (cp == KSSLCertificateCache::Accept) {
                if (certAndIPTheSame) {    // success
                    rc = 1;
                    setMetaData("ssl_action", "accept");
                } else {   // fail
                    result = messageBox(WarningYesNo,
                                        i18n("You have indicated that you wish to accept this certificate, but it is not issued to the server who is presenting it. Do you wish to continue loading?"),
                                        i18n("Server Authentication"));
                    if (result == KMessageBox::Yes) {
                        rc = 1;
                        setMetaData("ssl_action", "accept");
                        d->certCache->addHost(pc, d->host);
                    } else {
                        rc = -1;
                        setMetaData("ssl_action", "reject");
                    }
                }
            } else if (cp == KSSLCertificateCache::Reject) {      // fail
                messageBox(Information, i18n("SSL certificate is being rejected as requested. You can disable this in the KDE Control Center."),
                           i18n("Server Authentication"));
                rc = -1;
                setMetaData("ssl_action", "reject");
            } else {

//////// SNIP SNIP //////////

    return rc;
#endif //#if 0
    return ResultOk;
}


bool TCPSlaveBase::isConnected() const
{
    //QSslSocket::isValid() and therefore KTcpSocket::isValid() are shady...
    return d->socket.state() == KTcpSocket::ConnectedState;
}


bool TCPSlaveBase::waitForResponse(int t)
{
    return d->socket.waitForReadyRead(t * 1000);
}

void TCPSlaveBase::setBlocking(bool b)
{
    d->isBlocking = b;
}

void TCPSlaveBase::virtual_hook(int id, void* data)
{
    SlaveBase::virtual_hook(id, data);
}


#include "tcpslavebase.moc"
