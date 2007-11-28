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
#include <ksslall.h>
#include <ksslcertdialog.h>
#include <kmessagebox.h>

#include <klocale.h>
#include <QtCore/QDataStream>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostInfo>
#include <QtDBus/QtDBus>

#include <kapplication.h>
#include <ktoolinvocation.h>
#include <ksocketfactory.h>
#include <kprotocolmanager.h>
#include <k3streamsocket.h>

using namespace KIO;
using namespace KNetwork;

typedef QMap<QString, QString> StringStringMap;
Q_DECLARE_METATYPE(StringStringMap)

/** @internal */
class TCPSlaveBase::TcpSlaveBasePrivate
{
public:

    TcpSlaveBasePrivate()
        : socket(0),
        rblockSz(256),
        militantSSL(false),
        userAborted(false),
        isSSL(false) {}
    ~TcpSlaveBasePrivate() {}

    KSSL *kssl;
    bool usingTLS;
    KSSLCertificateCache *cc;
    QString host;
    QString realHost;
    QString ip;
#ifdef USE_SOCKETFACTORY
    QTcpSocket *socket;
#else
    KStreamSocket *socket;
#endif
    KSSLPKCS12 *pkcs;

    int status;
    int timeout;
    int rblockSz;      // Size for reading blocks in readLine()
    MetaData savedMetaData;
    bool block;
    bool useSSLTunneling;
    bool needSSLHandShake;
    bool militantSSL;              // If true, we just drop a connection silently
    // if SSL certificate check fails in any way.
    bool userAborted;

    bool isSSL;
    quint16 port;
    quint16 defaultPort;
    QByteArray serviceName;
};

QIODevice *TCPSlaveBase::socket() const
{
    return d->socket;
}


TCPSlaveBase::TCPSlaveBase(unsigned short int defaultPort,
                           const QByteArray &protocol,
                           const QByteArray &poolSocket,
                           const QByteArray &appSocket,
                           bool useSSL)
             :SlaveBase (protocol, poolSocket, appSocket),
              d(new TcpSlaveBasePrivate)
{
    d->defaultPort = defaultPort;
    d->serviceName = protocol;
    init();
    if (useSSL) {
        d->isSSL = true;
        d->isSSL = initializeSSL();
    }
}

// The constructor procedures go here
void TCPSlaveBase::init()
{
    d->kssl = 0L;
    d->ip = "";
    d->cc = 0L;
    d->usingTLS = false;
    d->pkcs = 0L;
    d->status = -1;
    d->timeout = KProtocolManager::connectTimeout();
    d->block = false;
    d->useSSLTunneling = false;
}

TCPSlaveBase::~TCPSlaveBase()
{
    cleanSSL();
    if (d->usingTLS) delete d->kssl;
    delete d->pkcs;
    delete d;
}

ssize_t TCPSlaveBase::write(const char *data, ssize_t len)
{
#ifdef Q_OS_UNIX
    if ( (d->isSSL || d->usingTLS) && !d->useSSLTunneling )
    {
        if ( d->needSSLHandShake )
            (void) doSSLHandShake( true );
        return d->kssl->write(data, len);
    }
#endif
    if (!d->socket)
        return -1;              // not connected

    ssize_t written = d->socket->write(data, len);
    d->socket->flush();

#ifdef USE_SOCKETFACTORY
    if (d->block)
    {
        // mimic blocking mode
        while (d->socket->state() == QAbstractSocket::ConnectedState &&
               d->socket->bytesToWrite())
            d->socket->waitForBytesWritten(-1);
        if (d->socket->state() != QAbstractSocket::ConnectedState)
            // connection closed due to error
            return -1;
    }
#endif
    return written;
}

ssize_t TCPSlaveBase::read(char* data, ssize_t len)
{
#ifdef Q_OS_UNIX
    if ( (d->isSSL || d->usingTLS) && !d->useSSLTunneling )
    {
        if ( d->needSSLHandShake )
            (void) doSSLHandShake( true );
        return d->kssl->read(data, len);
    }
#endif
    if (!d->socket)
        return -1;              // not connected

#ifdef USE_SOCKETFACTORY
    d->socket->waitForReadyRead(0); // read from the kernel
    if (d->block && d->socket->bytesAvailable() == 0)
        d->socket->waitForReadyRead(-1);
#endif
    return d->socket->read(data, len);
}


void TCPSlaveBase::setBlockSize(int sz)
{
  if (sz <= 0)
    sz = 1;

  d->rblockSz = sz;
}


ssize_t TCPSlaveBase::readLine(char *data, ssize_t len)
{
// Optimization:
//           It's small, but it probably results in a gain on very high
//   speed connections.  I moved 3 if statements out of the while loop
//   so that the while loop is as small as possible.  (GS)

  // let's not segfault!
  if (!data || !len)
    return -1;

#ifdef USE_SOCKETFACTORY
  return d->socket->readLine(data, len);

#else
  char tmpbuf[1024];   // 1kb temporary buffer for peeking
  *data = 0;
  ssize_t clen = 0;
  char *buf = data;
  int rc = 0;

if ((d->isSSL || d->usingTLS) && !d->useSSLTunneling) {       // SSL CASE
  if ( d->needSSLHandShake )
    (void) doSSLHandShake( true );

  while (clen < len-1) {
    rc = d->kssl->pending();
    if (rc > 0) {   // Read a chunk
      int bytes = rc;
      if (bytes > d->rblockSz)
         bytes = d->rblockSz;

      rc = d->kssl->peek(tmpbuf, bytes);
      if (rc <= 0) {
        // FIXME: this doesn't cover rc == 0 case
        return -1;
      }

      bytes = rc;   // in case it contains no \n
      for (int i = 0; i < rc; i++) {
        if (tmpbuf[i] == '\n') {
          bytes = i+1;
          break;
        }
      }

      if (bytes+clen >= len)   // don't read too much!
        bytes = len - clen - 1;

      rc = d->kssl->read(buf, bytes);
      if (rc > 0) {
        clen += rc;
        buf += (rc-1);
        if (*buf++ == '\n')
          break;
      } else {
        // FIXME: different case if rc == 0;
        return -1;
      }
    } else {        // Read a byte
      rc = d->kssl->read(buf, 1);
      if (rc <= 0) {
        return -1;
        // hm rc = 0 then
        // SSL_read says to call SSL_get_error to see if
        // this was an error.    FIXME
      } else {
        clen++;
        if (*buf++ == '\n')
          break;
      }
    }
  }
} else {                                                      // NON SSL CASE
  while (true) {
    rc = d->socket->bytesAvailable();

    if (rc == -1)		// error
      return -1;
    if (rc == 0) {
      // nothing to be read, wait for more
      rc = d->socket->waitForReadyRead(-1);
      if (rc == -1)
	// error
	return -1;
      if (rc == 0)
	// eof?
	rc = 1;
    }

    // peek to see if anything is available
    int bytes = rc;
    if ((ssize_t)bytes > len)
      bytes = len;

    rc = d->socket->peek(data, bytes);
    if (rc == -1)
      return -1;		// error
    if (rc == 0)
      return 0;			// eof

    for (int i = 0; i < rc; i++) {
      if (data[i] == '\n') {
	rc = d->socket->read(data, i + 1);
	return rc;
      }
    }
  }
}

  // Both cases fall through to here
  *buf = 0;
return clen;
#endif
}

// This function is simply a wrapper to establish the connection
// to the server.  It's a bit more complicated than ::connect
// because we first have to check to see if the user specified
// a port, and if so use it, otherwise we check to see if there
// is a port specified in /etc/services, and if so use that
// otherwise as a last resort use the supplied default port.
bool TCPSlaveBase::connectToHost( const QString &protocol,
                                  const QString &host,
                                  quint16 port,
                                  bool sendError )
{
    d->userAborted = false;
#ifdef Q_OS_UNIX

    //  - leaving SSL - warn before we even connect
    if (metaData("main_frame_request") == "TRUE" &&
        metaData("ssl_activate_warnings") == "TRUE" &&
               metaData("ssl_was_in_use") == "TRUE" &&
        !d->isSSL) {
       KSSLSettings kss;
       if (kss.warnOnLeave()) {
          int result = messageBox( i18n("You are about to leave secure "
                                        "mode. Transmissions will no "
                                        "longer be encrypted.\nThis "
                                        "means that a third party could "
                                        "observe your data in transit."),
                                   WarningContinueCancel,
                                   i18n("Security Information"),
                                   i18n("C&ontinue Loading"), QString(),
                                   "WarnOnLeaveSSLMode" );

           // Move this setting into KSSL instead
          KConfigGroup config = KSharedConfig::openConfig("kioslaverc")->group("Notification Messages");

          if (!config.readEntry("WarnOnLeaveSSLMode", true)) {
              config.deleteEntry("WarnOnLeaveSSLMode");
              config.sync();
              kss.setWarnOnLeave(false);
              kss.save();
          }

          if ( result == KMessageBox::Cancel ) {
             d->userAborted = true;
             return false;
          }
       }
    }
#endif
    d->status = -1;
    d->host = host;
    d->needSSLHandShake = d->isSSL;

#ifdef USE_SOCKETFACTORY
    d->socket = KSocketFactory::synchronousConnectToHost(protocol, host, port,
                                                         d->timeout > -1 ? d->timeout * 1000 : -1);
    if (d->socket->state() != QAbstractSocket::ConnectedState)
    {
        d->status = d->socket->error();
        if ( sendError )
        {
            if (d->status == QAbstractSocket::HostNotFoundError)
                error( ERR_UNKNOWN_HOST, host);
            else
                error( ERR_COULD_NOT_CONNECT, host + QLatin1String(": ") +
		       d->socket->errorString());
        }
        return false;
    }

    // store the IP for later
    d->ip = d->socket->peerAddress().toString();
    d->port = d->socket->peerPort();
#else
    Q_UNUSED(protocol);

    // set to blocking mode so that we can connect now
    d->socket = new KStreamSocket;
    d->socket->setBlocking(true);

    if ( d->timeout > -1 )
        d->socket->setTimeout( d->timeout * 1000 );
    if (!d->socket->connect(host, QString::number(port)))
    {
        d->status = d->socket->error();
        if ( sendError )
        {
            if (d->status == KNetwork::KSocketBase::LookupFailure)
                error( ERR_UNKNOWN_HOST, host);
            else if ( d->status != KNetwork::KSocketBase::NoError)
                error( ERR_COULD_NOT_CONNECT, host + QLatin1String(": ") +
		       d->socket->errorString());
        }
        return false;
    }

    // reset the blocking mode
    d->socket->setBlocking(d->block);

    // store the IP for later
    KNetwork::KSocketAddress sa = d->socket->peerAddress();
    d->ip = sa.nodeName();
    d->port = sa.asInet().port();
#endif

    if (d->isSSL && !d->useSSLTunneling) {
        if ( !doSSLHandShake( sendError ) )
            return false;
    }
    else
        setMetaData("ssl_in_use", "FALSE");

    return true;
}

void TCPSlaveBase::closeDescriptor()
{
    stopTLS();
    if (d->isSSL)
      d->kssl->close();
    if (d->socket) {
#ifdef USE_SOCKETFACTORY
        d->socket->disconnectFromHost();
        if (d->socket->state() != QAbstractSocket::UnconnectedState)
            d->socket->waitForDisconnected(); // wait for unsent data to be sent
#endif

        d->socket->close();
        delete d->socket;
        d->socket = 0;
    }
    d->ip.clear();
    d->host.clear();
}

bool TCPSlaveBase::initializeSSL()
{
    if (d->isSSL) {
        if (KSSL::doesSSLWork()) {
            d->kssl = new KSSL;
            return true;
        }
    }
    return false;
}

void TCPSlaveBase::cleanSSL()
{
    delete d->cc;

    if (d->isSSL) {
        delete d->kssl;
        d->kssl = 0;
    }
    d->militantSSL = false;
}

bool TCPSlaveBase::usingSSL() const
{
    return d->isSSL;
}

void TCPSlaveBase::setDefaultPort(quint16 port)
{
    d->defaultPort = port;
}

quint16 TCPSlaveBase::defaultPort() const
{
    return d->defaultPort;
}

quint16 TCPSlaveBase::port() const
{
    return d->port;
}

bool TCPSlaveBase::atEnd() const
{
    // this doesn't work!!
    kError(7029) << " called! It doesn't work.  Fix caller"
		  << endl << kBacktrace();
    return d->socket->atEnd();
}

int TCPSlaveBase::startTLS()
{
    if (d->usingTLS || d->useSSLTunneling || d->isSSL || !KSSL::doesSSLWork())
        return false;

    d->kssl = new KSSL(false);
    if (!d->kssl->TLSInit()) {
        delete d->kssl;
        return -1;
    }

    if ( !d->realHost.isEmpty() )
    {
      kDebug(7029) << "Setting real hostname: " << d->realHost;
      d->kssl->setPeerHost(d->realHost);
    } else {
      kDebug(7029) << "Setting real hostname: " << d->host;
      d->kssl->setPeerHost(d->host);
    }

    if (hasMetaData("ssl_session_id")) {
        KSSLSession *s = KSSLSession::fromString(metaData("ssl_session_id"));
        if (s) {
            d->kssl->setSession(s);
            delete s;
        }
    }
    certificatePrompt();

    int rc = d->kssl->connect(d->socket);
    if (rc < 0) {
        delete d->kssl;
        return -2;
    }

    setMetaData("ssl_session_id", d->kssl->session()->toString());

    d->usingTLS = true;
    setMetaData("ssl_in_use", "TRUE");

    if (!d->kssl->reusingSession()) {
        rc = verifyCertificate();
        if (rc != 1) {
            setMetaData("ssl_in_use", "FALSE");
            d->usingTLS = false;
            delete d->kssl;
            return -3;
        }
    }

    d->savedMetaData = mOutgoingMetaData;
    return (d->usingTLS ? 1 : 0);
}


void TCPSlaveBase::stopTLS()
{
    if (d->usingTLS) {
        delete d->kssl;
        d->usingTLS = false;
        setMetaData("ssl_in_use", "FALSE");
    }
}


void TCPSlaveBase::setSSLMetaData() {
  if (!(d->usingTLS || d->useSSLTunneling || d->isSSL))
    return;

  mOutgoingMetaData = d->savedMetaData;
}


bool TCPSlaveBase::canUseTLS() const
{
    return !(d->isSSL || d->needSSLHandShake || !KSSL::doesSSLWork());
}


void TCPSlaveBase::certificatePrompt()
{
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
        switch(aa) {
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

  QString ourHost;
  if (!d->realHost.isEmpty()) {
     ourHost = d->realHost;
  } else {
     ourHost = d->host;
  }

  // Look for a certificate on a per-host basis as an override
  QString tmpcn = KSSLCertificateHome::getDefaultCertificateName(ourHost, &aa);
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
    while ( it != certs.end() ) {
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
                                                   QStringList() );
    }

    QDBusInterface uis("org.kde.kio.uiserver", "/UIServer", "org.kde.KIO.UIServer");

    QDBusMessage retVal = uis.call("showSSLCertDialog", ourHost, certs, metaData("window-id").toLongLong());
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
       KSSLCertificateHome::setDefaultCertificate(certname, ourHost,
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
                KSSLCertificateHome::setDefaultCertificate(certname, ourHost,
                                                           true, false);
         }
      }
      d->pkcs = pkcs;
   }
}



bool TCPSlaveBase::usingTLS() const
{
    return d->usingTLS;
}

//  Returns 0 for failed verification, -1 for rejected cert and 1 for ok
int TCPSlaveBase::verifyCertificate()
{
    int rc = 0;
    bool permacache = false;
    bool isChild = false;
    bool _IPmatchesCN = false;
    int result;
    bool doAddHost = false;
    QString ourHost;

    if (!d->realHost.isEmpty())
        ourHost = d->realHost;
    else ourHost = d->host;

    QString theurl = QString(d->serviceName)+"://"+ourHost+':'+ QString::number(d->port);

   if (!hasMetaData("ssl_militant") || metaData("ssl_militant") == "FALSE")
     d->militantSSL = false;
   else if (metaData("ssl_militant") == "TRUE")
     d->militantSSL = true;

    if (!d->cc) d->cc = new KSSLCertificateCache;

    KSSLCertificate& pc = d->kssl->peerInfo().getPeerCertificate();

    KSSLCertificate::KSSLValidationList ksvl = pc.validateVerbose(KSSLCertificate::SSLServer);

   _IPmatchesCN = d->kssl->peerInfo().certMatchesAddress();
   if (!_IPmatchesCN) {
      // Check if switching to the canonical name helps matching
      QHostInfo res = QHostInfo::fromName(d->kssl->peerInfo().peerHost());
      if (!res.addresses().isEmpty()) {
         QString old = d->kssl->peerInfo().peerHost();
         d->kssl->peerInfo().setPeerHost(res.hostName());
         _IPmatchesCN = d->kssl->peerInfo().certMatchesAddress();
         if (!_IPmatchesCN) {
            d->kssl->peerInfo().setPeerHost(old);
         }
      }
      if (!_IPmatchesCN && !d->militantSSL) { // force this if the user wants it
         if (d->cc->getHostList(pc).contains(ourHost)) {
            _IPmatchesCN = true;
         }
      }
   }

   if (!_IPmatchesCN) {
      ksvl << KSSLCertificate::InvalidHost;
   }

   KSSLCertificate::KSSLValidation ksv = KSSLCertificate::Ok;
   if (!ksvl.isEmpty())
      ksv = ksvl.first();

    /* Setting the various bits of meta-info that will be needed. */
    setMetaData("ssl_cipher", d->kssl->connectionInfo().getCipher());
    setMetaData("ssl_cipher_desc",
                            d->kssl->connectionInfo().getCipherDescription());
    setMetaData("ssl_cipher_version",
                                d->kssl->connectionInfo().getCipherVersion());
    setMetaData("ssl_cipher_used_bits",
              QString::number(d->kssl->connectionInfo().getCipherUsedBits()));
    setMetaData("ssl_cipher_bits",
                  QString::number(d->kssl->connectionInfo().getCipherBits()));
    setMetaData("ssl_peer_ip", d->ip);
    if (!d->realHost.isEmpty()) {
       setMetaData("ssl_proxied", "true");
    }

    QString errorStr;
    for(KSSLCertificate::KSSLValidationList::ConstIterator it = ksvl.begin();
        it != ksvl.end(); ++it)
    {
       errorStr += QString::number(*it)+':';
    }
    setMetaData("ssl_cert_errors", errorStr);
    setMetaData("ssl_peer_certificate", pc.toString());

    if (pc.chain().isValid() && pc.chain().depth() > 1) {
       QString theChain;
       QList<KSSLCertificate *> chain = pc.chain().getChain();
       foreach (KSSLCertificate *c, chain) {
          theChain += c->toString();
          theChain += '\n';
       }
       qDeleteAll(chain);
       setMetaData("ssl_peer_chain", theChain);
    } else setMetaData("ssl_peer_chain", "");

   setMetaData("ssl_cert_state", QString::number(ksv));

   if (ksv == KSSLCertificate::Ok) {
      rc = 1;
      setMetaData("ssl_action", "accept");
   }

   kDebug(7029) << "SSL HTTP frame the parent? " << metaData("main_frame_request");
   if (!hasMetaData("main_frame_request") || metaData("main_frame_request") == "TRUE") {
      // Since we're the parent, we need to teach the child.
      setMetaData("ssl_parent_ip", d->ip);
      setMetaData("ssl_parent_cert", pc.toString());
      //  - Read from cache and see if there is a policy for this
      KSSLCertificateCache::KSSLCertificatePolicy cp =
                                         d->cc->getPolicyByCertificate(pc);

      //  - validation code
      if (ksv != KSSLCertificate::Ok) {
         if (d->militantSSL) {
            return -1;
         }

         if (cp == KSSLCertificateCache::Unknown ||
             cp == KSSLCertificateCache::Ambiguous) {
            cp = KSSLCertificateCache::Prompt;
         } else {
            // A policy was already set so let's honor that.
            permacache = d->cc->isPermanent(pc);
         }

         if (!_IPmatchesCN && cp == KSSLCertificateCache::Accept) {
            cp = KSSLCertificateCache::Prompt;
//            ksv = KSSLCertificate::Ok;
         }

         // Precondition: cp is one of Reject, Accept or Prompt
         switch (cp) {
         case KSSLCertificateCache::Accept:
           rc = 1;
           setMetaData("ssl_action", "accept");
          break;
         case KSSLCertificateCache::Reject:
           rc = -1;
           setMetaData("ssl_action", "reject");
          break;
         case KSSLCertificateCache::Prompt:
           {
             do {
                if (ksv == KSSLCertificate::InvalidHost) {
                   result = messageBox( WarningYesNoCancel,
                              i18n("The IP address of the host %1 "
                                   "does not match the one the "
                                   "certificate was issued to.",
                                   ourHost),
                              i18n("Server Authentication"),
                              i18n("&Details"),
                              i18n("Co&ntinue") );
                } else {
                   result = messageBox( WarningYesNoCancel,
                              i18n("The server certificate failed the "
                                   "authenticity test (%1).",
                                   ourHost),
                              i18n("Server Authentication"),
                              i18n("&Details"),
                              i18n("Co&ntinue") );
                }

                if (result == KMessageBox::Yes) {
                  if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kio.uiserver"))
                      KToolInvocation::startServiceByDesktopPath("kuiserver.desktop",
                                                                 QStringList() );

                  QDBusInterface uis("org.kde.kio.uiserver", "/UIServer", "org.kde.KIO.UIServer");
                  QMap<QString, QString> adjusted = mOutgoingMetaData;
                  qDBusRegisterMetaType<QMap<QString, QString> >();
                  uis.call("showSSLInfoDialog",
                      theurl, qVariantFromValue(adjusted), metaData("window-id").toLongLong());
                }
             } while (result == KMessageBox::Yes);

             if (result == KMessageBox::No) {
                setMetaData("ssl_action", "accept");
                rc = 1;
                cp = KSSLCertificateCache::Accept;
                doAddHost = true;
                   result = messageBox( WarningYesNo,
                                  i18n("Would you like to accept this "
                                       "certificate forever without "
                                       "being prompted?"),
                                  i18n("Server Authentication"),
                                         i18n("&Forever"),
                                         i18n("&Current Sessions Only"));
                    if (result == KMessageBox::Yes)
                        permacache = true;
                    else
                        permacache = false;
             } else {
                setMetaData("ssl_action", "reject");
                rc = -1;
                cp = KSSLCertificateCache::Prompt;
             }
          break;
            }
         default:
          kDebug(7029) << "TCPSlaveBase/SSL error in cert code."
                              << "Please report this to kfm-devel@kde.org."
                              << endl;
          break;
         }
      }


      //  - cache the results
      d->cc->addCertificate(pc, cp, permacache);
      if (doAddHost) d->cc->addHost(pc, ourHost);
    } else {    // Child frame
      //  - Read from cache and see if there is a policy for this
      KSSLCertificateCache::KSSLCertificatePolicy cp =
                                             d->cc->getPolicyByCertificate(pc);
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
          if (d->militantSSL) {
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
        if (d->militantSSL) {
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
               d->cc->addHost(pc, ourHost);
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
          do {
             result = messageBox(WarningYesNoCancel,
                                 i18n("The server certificate failed the "
                                      "authenticity test (%1).",
                                      ourHost),
                                 i18n("Server Authentication"),
                                 i18n("&Details"),
                                 i18n("Co&nnect"));
                if (result == KMessageBox::Yes) {
                  if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kio.uiserver"))
                      KToolInvocation::startServiceByDesktopPath("kuiserver.desktop",
                                                                 QStringList() );

                  QDBusInterface uis("org.kde.kio.uiserver", "/UIServer", "org.kde.KIO.UIServer");
                  QMap<QString, QString> adjusted = mOutgoingMetaData;
                  qDBusRegisterMetaType<QMap<QString, QString> >(); // make sure it's registered
                  uis.call("showSSLInfoDialog",
                           theurl, qVariantFromValue(adjusted), metaData("window-id").toLongLong());
                }
          } while (result == KMessageBox::Yes);

          if (result == KMessageBox::No) {
             setMetaData("ssl_action", "accept");
             rc = 1;
             cp = KSSLCertificateCache::Accept;
             result = messageBox(WarningYesNo,
                                 i18n("Would you like to accept this "
                                      "certificate forever without "
                                      "being prompted?"),
                                 i18n("Server Authentication"),
                                 i18n("&Forever"),
                                 i18n("&Current Sessions Only"));
             permacache = (result == KMessageBox::Yes);
             d->cc->addCertificate(pc, cp, permacache);
             d->cc->addHost(pc, ourHost);
          } else {
             setMetaData("ssl_action", "reject");
             rc = -1;
             cp = KSSLCertificateCache::Prompt;
             d->cc->addCertificate(pc, cp, permacache);
          }
        }
      }
    }


   if (rc == -1) {
      return rc;
   }

   if (metaData("ssl_activate_warnings") == "TRUE") {
   //  - entering SSL
   if (!isChild && metaData("ssl_was_in_use") == "FALSE" &&
                                        d->kssl->settings()->warnOnEnter()) {
     int result;
     do {
                result = messageBox(               i18n("You are about to "
                                                        "enter secure mode. "
                                                        "All transmissions "
                                                        "will be encrypted "
                                                        "unless otherwise "
                                                        "noted.\nThis means "
                                                        "that no third party "
                                                        "will be able to "
                                                        "easily observe your "
                                                        "data in transit."),
                                                   WarningYesNo,
                                                   i18n("Security Information"),
                                                   i18n("Display SSL "
                                                        "&Information"),
                                                   i18n("C&onnect"),
                                                   "WarnOnEnterSSLMode" );
      // Move this setting into KSSL instead
      KConfigGroup config = KSharedConfig::openConfig("kioslaverc")->group("Notification Messages");

      bool dialogBoxStatus = false;
      if( config.hasKey("WarnOnEnterSSLMode"))
        dialogBoxStatus = true;
      bool keyStatus = config.readEntry("WarnOnEnterSSLMode", true);
      dialogBoxStatus = dialogBoxStatus && keyStatus;
      if (!keyStatus) {
          config.deleteEntry("WarnOnEnterSSLMode");
          config.sync();
          d->kssl->settings()->setWarnOnEnter(false);
          d->kssl->settings()->save();
      }

      if ( result == KMessageBox::Yes )
      {
          if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kio.uiserver"))
              KToolInvocation::startServiceByDesktopPath("kuiserver.desktop",
                                                         QStringList() );

          QDBusInterface uis("org.kde.kio.uiserver", "/UIServer", "org.kde.KIO.UIServer");
          QMap<QString, QString> adjusted = mOutgoingMetaData;
          qDBusRegisterMetaType<QMap<QString, QString> >();
          uis.call("showSSLInfoDialog",
                   theurl, qVariantFromValue(adjusted), metaData("window-id").toLongLong());
      }
      //Laurent: When we display messagebox "Security Information" and we say "don't ask next time"
      //we can't have a result == KMessageBox::No => we create a loop.
      if(dialogBoxStatus)
          break;
      } while (result != KMessageBox::No);
   }

   }   // if ssl_activate_warnings


   kDebug(7029) << "SSL connection information follows:" << endl
          << "+-----------------------------------------------" << endl
          << "| Cipher: " << d->kssl->connectionInfo().getCipher() << endl
          << "| Description: " << d->kssl->connectionInfo().getCipherDescription() << endl
          << "| Version: " << d->kssl->connectionInfo().getCipherVersion() << endl
          << "| Strength: " << d->kssl->connectionInfo().getCipherUsedBits()
          << " of " << d->kssl->connectionInfo().getCipherBits()
          << " bits used." << endl
          << "| PEER:" << endl
          << "| Subject: " << d->kssl->peerInfo().getPeerCertificate().getSubject() << endl
          << "| Issuer: " << d->kssl->peerInfo().getPeerCertificate().getIssuer() << endl
          << "| Validation: " << (int)ksv << endl
          << "| Certificate matches IP: " << _IPmatchesCN << endl
          << "+-----------------------------------------------"
          << endl;

   // sendMetaData();  Do not call this function!!
   return rc;
}


bool TCPSlaveBase::isConnectionValid()
{
#ifdef USE_SOCKETFACTORY
    if ( !d->socket || d->socket->state() != QAbstractSocket::ConnectedState)
      return false;
#else
    if ( !d->socket || d->socket->state() != KNetwork::KStreamSocket::Connected)
      return false;
#endif

    qint64 retval = -1;
    retval = d->socket->bytesAvailable();

    // retval == -1 ==> Error
    // retval ==  0 ==> Connection Idle
    // retval ==  1 ==> either Active or Closed
    // retval >   1 ==> Connection Active
    //kDebug(7029) << "TCPSlaveBase::isConnectionValid: select returned: "
    //              << retval << endl;

    if (retval == -1)
       return false;

    if (retval == 1)
    {
      // corner case!
      char c;
      retval = d->socket->read(&c, 1);
      if (retval == 0)
	// it's in fact closed
	return false;
      d->socket->ungetChar(c);
    }

    // if there's more than 1 byte in bytesAvailable, we can't know if the
    // connection is closed or not without actually reading from it
    // (i.e., we can't do it with peek)

    return true; // Connection still valid.
}


bool TCPSlaveBase::waitForResponse( int t )
{
  if ( (d->isSSL || d->usingTLS) && !d->useSSLTunneling && d->kssl )
  {
    if (d->kssl->pending() > 0)
        return true;
  }
  else if (d->socket->bytesAvailable() > 0)
    return true;

#ifdef USE_SOCKETFACTORY
  return d->socket->waitForReadyRead(t * 1000);
#else
  // We want to return true if we saw EOF as well, as it is after all 
  // activity (and that's how KDE3 behaved)
  bool timeout = false;
  return d->socket->waitForMore(t * 1000, &timeout) >= 0 && !timeout;
#endif
}

int TCPSlaveBase::connectResult()
{
    return d->status;
}

void TCPSlaveBase::setBlockConnection( bool b )
{
    d->block = b;
#ifndef USE_SOCKETFACTORY
    if (d->socket)
        d->socket->setBlocking(b);
#endif
}

void TCPSlaveBase::setConnectTimeout( int t )
{
    d->timeout = t;
}

bool TCPSlaveBase::isSSLTunnelEnabled() const
{
    return d->useSSLTunneling;
}

void TCPSlaveBase::setEnableSSLTunnel( bool enable )
{
    d->useSSLTunneling = enable;
}

void TCPSlaveBase::setRealHost( const QString& realHost )
{
    d->realHost = realHost;
}

bool TCPSlaveBase::doSSLHandShake( bool sendError )
{
    kDebug(7029) << "TCPSlaveBase::doSSLHandShake: ";
    QString msgHost = d->host;

    d->kssl->reInitialize();

    if (hasMetaData("ssl_session_id")) {
        KSSLSession *s = KSSLSession::fromString(metaData("ssl_session_id"));
        if (s) {
            d->kssl->setSession(s);
            delete s;
	}
    }
    certificatePrompt();

    if ( !d->realHost.isEmpty() )
    {
      msgHost = d->realHost;
    }

    kDebug(7029) << "Setting real hostname: " << msgHost;
    d->kssl->setPeerHost(msgHost);

    d->status = d->kssl->connect(d->socket);
    if (d->status < 0)
    {
        closeDescriptor();
        if ( sendError )
            error( ERR_COULD_NOT_CONNECT, msgHost);
        return false;
    }

    setMetaData("ssl_session_id", d->kssl->session()->toString());
    setMetaData("ssl_in_use", "TRUE");

    if (!d->kssl->reusingSession()) {
        int rc = verifyCertificate();
        if ( rc != 1 ) {
            d->status = -1;
            closeDescriptor();
            if ( sendError )
                error( ERR_COULD_NOT_CONNECT, msgHost);
            return false;
        }
    }

    d->needSSLHandShake = false;

    d->savedMetaData = mOutgoingMetaData;
    return true;
}


bool TCPSlaveBase::userAborted() const
{
   return d->userAborted;
}

void TCPSlaveBase::virtual_hook( int id, void* data )
{ SlaveBase::virtual_hook( id, data ); }

