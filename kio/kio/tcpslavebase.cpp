/*
 * $Id$
 *
 * Copyright (C) 2000 Alex Zepeda <zipzippy@sonic.net
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <time.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#include <ksocks.h>
#include <kdebug.h>
#include <kssl.h>
#include <ksslcertificate.h>
#include <ksslcertificatecache.h>
#include <ksslcertificatehome.h>
#include <ksslcertdlg.h>
#include <ksslpkcs12.h>
#include <ksslx509v3.h>
#include <kmessagebox.h>

#include <klocale.h>
#include <dcopclient.h>
#include <qcstring.h>
#include <qdatastream.h>

#include <kapplication.h>

#include <kprotocolmanager.h>

#include "kio/tcpslavebase.h"

using namespace KIO;

class TCPSlaveBase::TcpSlaveBasePrivate
{
public:

  TcpSlaveBasePrivate() : rblockSz(256), militantSSL(false), userAborted(false) {}
  ~TcpSlaveBasePrivate() {}

  KSSL *kssl;
  bool usingTLS;
  KSSLCertificateCache *cc;
  QString host;
  QString realHost;
  QString ip;
  DCOPClient *dcc;
  KSSLPKCS12 *pkcs;

  int status;
  int timeout;
  int rblockSz;      // Size for reading blocks in readLine()
  bool block;
  bool useSSLTunneling;
  bool needSSLHandShake;
  bool militantSSL;              // If true, we just drop a connection silently
                                 // if SSL certificate check fails in any way.
  bool userAborted;
  MetaData savedMetaData;
};


TCPSlaveBase::TCPSlaveBase(unsigned short int defaultPort,
                           const QCString &protocol,
                           const QCString &poolSocket,
                           const QCString &appSocket)
             :SlaveBase (protocol, poolSocket, appSocket),
              m_iSock(-1),
              m_iDefaultPort(defaultPort),
              m_sServiceName(protocol),
              fp(0)
{
    // We have to have two constructors, so don't add anything
    // else in here. Put it in doConstructorStuff() instead.
    doConstructorStuff();
    m_bIsSSL = false;
}

TCPSlaveBase::TCPSlaveBase(unsigned short int defaultPort,
                           const QCString &protocol,
                           const QCString &poolSocket,
                           const QCString &appSocket,
                           bool useSSL)
             :SlaveBase (protocol, poolSocket, appSocket),
              m_iSock(-1),
              m_bIsSSL(useSSL),
              m_iDefaultPort(defaultPort),
              m_sServiceName(protocol),
              fp(0)
{
    doConstructorStuff();
    if (useSSL)
        m_bIsSSL = InitializeSSL();
}

// The constructor procedures go here now.
void TCPSlaveBase::doConstructorStuff()
{
    d = new TcpSlaveBasePrivate;
    d->kssl = 0L;
    d->ip = "";
    d->cc = 0L;
    d->usingTLS = false;
    d->dcc = 0L;
    d->pkcs = 0L;
    d->status = -1;
    d->timeout = KProtocolManager::connectTimeout();
    d->block = false;
    d->useSSLTunneling = false;
}

TCPSlaveBase::~TCPSlaveBase()
{
    CleanSSL();
    if (d->usingTLS) delete d->kssl;
    if (d->dcc) delete d->dcc;
    if (d->pkcs) delete d->pkcs;
    delete d;
}

ssize_t TCPSlaveBase::write(const void *data, ssize_t len)
{
    if ( (m_bIsSSL || d->usingTLS) && !d->useSSLTunneling )
    {
        if ( d->needSSLHandShake )
            (void) doSSLHandShake( true );
        return d->kssl->write(data, len);
    }
    return KSocks::self()->write(m_iSock, data, len);
}

ssize_t TCPSlaveBase::read(void *data, ssize_t len)
{
    if ( (m_bIsSSL || d->usingTLS) && !d->useSSLTunneling )
    {
        if ( d->needSSLHandShake )
            (void) doSSLHandShake( true );
        return d->kssl->read(data, len);
    }
    return KSocks::self()->read(m_iSock, data, len);
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
  if (!data) 
    return -1;

  char tmpbuf[1024];   // 1kb temporary buffer for peeking
  *data = 0;
  int clen = 0;
  char *buf = data;
  int rc = 0;

if ((m_bIsSSL || d->usingTLS) && !d->useSSLTunneling) {       // SSL CASE
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
  while (clen < len-1) {
    rc = KSocks::self()->read(m_iSock, buf, 1);
    if (rc <= 0) {
      // FIXME: this doesn't cover rc == 0 case
      return -1;
    } else {
      clen++;
      if (*buf++ == '\n')
        break;
    }
  }
}

  // Both cases fall through to here
  *buf = 0;
return clen;
}

unsigned short int TCPSlaveBase::port(unsigned short int _p)
{
    unsigned short int p = _p;

    if (_p <= 0)
    {
        p = m_iDefaultPort;
    }

    return p;
}

// This function is simply a wrapper to establish the connection
// to the server.  It's a bit more complicated than ::connect
// because we first have to check to see if the user specified
// a port, and if so use it, otherwise we check to see if there
// is a port specified in /etc/services, and if so use that
// otherwise as a last resort use the supplied default port.
bool TCPSlaveBase::connectToHost( const QString &host,
                                  unsigned int _port,
                                  bool sendError )
{
    unsigned short int p;
    KExtendedSocket ks;

    d->userAborted = false;

    //  - leaving SSL - warn before we even connect
    if (metaData("ssl_activate_warnings") == "TRUE" &&
               metaData("ssl_was_in_use") == "TRUE" &&
        !m_bIsSSL) {
       KSSLSettings kss;
       if (kss.warnOnLeave()) {
          int result = messageBox( WarningContinueCancel,
                                   i18n("You are about to leave secure "
                                        "mode. Transmissions will no "
                                        "longer be encrypted.\nThis "
                                        "means that a third party could "
                                        "observe your data in transit."),
                                   i18n("Security Information"),
                                   i18n("Continue Loading") );
          if ( result == KMessageBox::Cancel ) {
             d->userAborted = true;
             return false;
          }
       }
    }

    d->status = -1;
    d->host = host;
    d->needSSLHandShake = m_bIsSSL;
    p = port(_port);
    ks.setAddress(host, p);
    if ( d->timeout > -1 )
        ks.setTimeout( d->timeout );

    if (ks.connect() < 0)
    {
        d->status = ks.status();
        if ( sendError )
        {
            if (d->status == IO_LookupError)
                error( ERR_UNKNOWN_HOST, host);
            else if ( d->status != -1 )
                error( ERR_COULD_NOT_CONNECT, host);
        }
        return false;
    }

    m_iSock = ks.fd();

    // store the IP for later
    const KSocketAddress *sa = ks.peerAddress();
    d->ip = sa->nodeName();

    ks.release(); // KExtendedSocket no longer applicable

    if ( d->block != ks.blockingMode() )
        ks.setBlockingMode( d->block );

    m_iPort=p;

    if (m_bIsSSL && !d->useSSLTunneling) {
        if ( !doSSLHandShake( sendError ) )
            return false;
    }
    else
        setMetaData("ssl_in_use", "FALSE");

    // Since we want to use stdio on the socket,
    // we must fdopen it to get a file pointer,
    // if it fails, close everything up
    if ((fp = fdopen(m_iSock, "w+")) == 0) {
        closeDescriptor();
        return false;
    }

    return true;
}

void TCPSlaveBase::closeDescriptor()
{
    stopTLS();
    if (fp) {
        fclose(fp);
        fp=0;
        m_iSock=-1;
        if (m_bIsSSL)
            d->kssl->close();
    }
    if (m_iSock != -1) {
        close(m_iSock);
        m_iSock=-1;
    }
    d->ip = "";
    d->host = "";
}

bool TCPSlaveBase::initializeSSL()
{
    if (m_bIsSSL) {
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

    if (m_bIsSSL) {
        delete d->kssl;
        d->kssl = 0;
    }
    d->militantSSL = false;
}

bool TCPSlaveBase::atEnd()
{
    return feof(fp);
}

int TCPSlaveBase::startTLS()
{
    if (d->usingTLS || d->useSSLTunneling || m_bIsSSL || !KSSL::doesSSLWork())
        return false;

    d->kssl = new KSSL(false);
    if (!d->kssl->TLSInit()) {
        delete d->kssl;
        return -1;
    }

    if ( !d->realHost.isEmpty() )
    {
      kdDebug(7029) << "Setting real hostname: " << d->realHost << endl;
      d->kssl->setPeerHost(d->realHost);
    } else {
      kdDebug(7029) << "Setting real hostname: " << d->host << endl;
      d->kssl->setPeerHost(d->host);
    }

    certificatePrompt();

    int rc = d->kssl->connect(m_iSock);
    if (rc < 0) {
        delete d->kssl;
        return -2;
    }

    d->usingTLS = true;
    setMetaData("ssl_in_use", "TRUE");
    rc = verifyCertificate();
    if (rc != 1) {
        setMetaData("ssl_in_use", "FALSE");
        d->usingTLS = false;
        delete d->kssl;
        return -3;
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
  if (!(d->usingTLS || d->useSSLTunneling || m_bIsSSL))
    return;

  mOutgoingMetaData = d->savedMetaData;
}


bool TCPSlaveBase::canUseTLS()
{
    if (m_bIsSSL || d->needSSLHandShake || !KSSL::doesSSLWork())
        return false;

    KSSLSettings kss;
    return kss.tlsv1();
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
          certname = "";
         break;
        case KSSLCertificateHome::AuthPrompt:
          send = false; prompt = true;
         break;
        default:
         break;
        }
  }

  QString ourHost;
  if (!d->realHost.isEmpty())
     ourHost = d->realHost;
  else ourHost = d->host;

  // Look for a certificate on a per-host basis as an override
  QString tmpcn = KSSLCertificateHome::getDefaultCertificateName(ourHost, &aa);
  if (aa != KSSLCertificateHome::AuthNone) {   // we must override
    switch (aa) {
        case KSSLCertificateHome::AuthSend:
          send = true; prompt = false;
          certname = tmpcn;
         break;
        case KSSLCertificateHome::AuthDont:
          send = false; prompt = false;
          certname = "";
         break;
        case KSSLCertificateHome::AuthPrompt:
          send = false; prompt = true;
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

  for (QStringList::Iterator it = certs.begin();
           it != certs.end();
           ++it) {
    KSSLPKCS12 *pkcs =
      KSSLCertificateHome::getCertificateByName(*it);
    if (pkcs)
    if (!pkcs->getCertificate() ||
        !pkcs->getCertificate()->x509V3Extensions().certTypeSSLClient()) {
      certs.remove(*it);
    }
  }

        if (certs.isEmpty()) return;  // we had nothing else, and prompt failed

     if (!d->dcc) {
        d->dcc = new DCOPClient;
        d->dcc->attach();
        if (!d->dcc->isApplicationRegistered("kio_uiserver")) {
           KApplication::startServiceByDesktopPath("kio_uiserver.desktop",
                                                   QStringList() );
        }
     }

     QByteArray data, retval;
     QCString rettype;
     QDataStream arg(data, IO_WriteOnly);
     arg << ourHost;
     arg << certs;
     bool rc = d->dcc->call("kio_uiserver", "UIServer",
                               "showSSLCertDialog(QString, QStringList)",
                               data, rettype, retval);

     if (rc && rettype == "KSSLCertDlgRet") {
        QDataStream retStream(retval, IO_ReadOnly);
        KSSLCertDlgRet drc;
        retStream >> drc;
        if (drc.ok) {
           send = drc.send;
           save = drc.save;
           certname = drc.choice;
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
     do {
        QString pass;
        QByteArray authdata, authval;
        QCString rettype;
        QDataStream qds(authdata, IO_WriteOnly);
        ai.prompt = i18n("Enter the certificate password:");
        ai.caption = i18n("SSL Certificate Password");
        ai.setModified(true);
        ai.username = certname;
        ai.keepPassword = true;
        if (!checkCachedAuthentication(ai)) {
           qds << ai;

           if (!d->dcc) {
              d->dcc = new DCOPClient;
              d->dcc->attach();
              if (!d->dcc->isApplicationRegistered("kio_uiserver")) {
                 KApplication::startServiceByDesktopPath("kio_uiserver.desktop",
                                                         QStringList() );
             }
           }

           bool rc = d->dcc->call("kio_uiserver", "UIServer",
                                   "openPassDlg(KIO::AuthInfo)",
                                   authdata, rettype, authval);
           if (!rc) break;
           if (rettype != "QByteArray") continue;

           QDataStream qdret(authval, IO_ReadOnly);
           QByteArray authdecode;
           qdret >> authdecode;
           QDataStream qdtoo(authdecode, IO_ReadOnly);
           qdtoo >> ai;
           if (!ai.isModified()) break;
        }
        pass = ai.password;
        pkcs = KSSLCertificateHome::getCertificateByName(certname, pass);

        if (!pkcs) {
              int rc = messageBox(WarningYesNo, i18n("Unable to open the "
                                                     "certificate. Try a "
                                                     "new password?"),
                                                i18n("SSL"));
              if (rc == KMessageBox::No) break;
        }
     } while (!pkcs);
     cacheAuthentication(ai);
  }

   // If we could open the certificate, let's send it
   if (pkcs) {
      if (!d->kssl->setClientCertificate(pkcs)) {
            messageBox(Information, i18n("The procedure to set the "
                                         "client certificate for the session "
                                         "failed."), i18n("SSL"));
         delete pkcs;  // we don't need this anymore
      } else {
         kdDebug(7029) << "Client SSL certificate is being used." << endl;
         setMetaData("ssl_using_client_cert", "TRUE");
         if (save) {
                KSSLCertificateHome::setDefaultCertificate(certname, ourHost,
                                                           true, false);
         }
      }
      d->pkcs = pkcs;
   }

}




bool TCPSlaveBase::usingTLS()
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

    QString theurl = QString(m_sServiceName)+"://"+ourHost+":"+QString::number(m_iPort);

   if (!hasMetaData("ssl_militant") || metaData("ssl_militant") == "FALSE")
     d->militantSSL = false;
   else if (metaData("ssl_militant") == "TRUE")
     d->militantSSL = true;

    if (!d->cc) d->cc = new KSSLCertificateCache;

    KSSLCertificate& pc = d->kssl->peerInfo().getPeerCertificate();

    KSSLCertificate::KSSLValidation ksv = pc.validate();

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
    setMetaData("ssl_cert_state", QString::number(ksv));
    setMetaData("ssl_peer_certificate", pc.toString());

    if (pc.chain().isValid() && pc.chain().depth() > 1) {
       QString theChain;
       QPtrList<KSSLCertificate> chain = pc.chain().getChain();
       for (KSSLCertificate *c = chain.first(); c; c = chain.next()) {
          theChain += c->toString();
          theChain += "\n";
       }
       setMetaData("ssl_peer_chain", theChain);
    } else setMetaData("ssl_peer_chain", "");


   if (ksv == KSSLCertificate::Ok) {
      rc = 1;
      setMetaData("ssl_action", "accept");
   }

   _IPmatchesCN = d->kssl->peerInfo().certMatchesAddress();
   if (!_IPmatchesCN && !d->militantSSL) {  // force this if the user wants it
      if (d->cc->getHostList(pc).contains(ourHost))
         _IPmatchesCN = true;
   }

   kdDebug(7029) << "SSL HTTP frame the parent? " << metaData("main_frame_request") << endl;
   if (!hasMetaData("main_frame_request") || metaData("main_frame_request") == "TRUE") {
      // Since we're the parent, we need to teach the child.
      setMetaData("ssl_parent_ip", d->ip);
      setMetaData("ssl_parent_cert", pc.toString());
      //  - Read from cache and see if there is a policy for this
      KSSLCertificateCache::KSSLCertificatePolicy cp =
                                         d->cc->getPolicyByCertificate(pc);

      //  - validation code
      if (ksv != KSSLCertificate::Ok || !_IPmatchesCN) {
   if (d->militantSSL) {
         return -1;
   }

         if (cp == KSSLCertificateCache::Unknown ||
             cp == KSSLCertificateCache::Ambiguous) {
            cp = KSSLCertificateCache::Prompt;
         } else {
            // A policy was already set so let's honour that.
            permacache = d->cc->isPermanent(pc);
         }

         if (!_IPmatchesCN && cp == KSSLCertificateCache::Accept) {
            cp = KSSLCertificateCache::Prompt;
            ksv = KSSLCertificate::Ok;
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
                if (ksv == KSSLCertificate::Ok && !_IPmatchesCN) {
                        QString msg = i18n("The IP address of the host %1 "
                                           "does not match the one the "
                                           "certificate was issued to.");
                   result = messageBox( WarningYesNoCancel,
                              msg.arg(ourHost),
                              i18n("Server Authentication"),
                              i18n("&Details..."),
                              i18n("Co&ntinue") );
                } else {
                   QString msg = i18n("The server certificate failed the "
                                      "authenticity test (%1).");
                   result = messageBox( WarningYesNoCancel,
                              msg.arg(ourHost),
                              i18n("Server Authentication"),
                              i18n("&Details..."),
                              i18n("Co&ntinue") );
                }

                if (result == KMessageBox::Yes) {
                   if (!d->dcc) {
                      d->dcc = new DCOPClient;
                      d->dcc->attach();
                      if (!d->dcc->isApplicationRegistered("kio_uiserver")) {
                         KApplication::startServiceByDesktopPath("kio_uiserver.desktop",
                         QStringList() );
                      }

                   }
                   QByteArray data, ignore;
                   QCString ignoretype;
                   QDataStream arg(data, IO_WriteOnly);
                   arg << theurl << mOutgoingMetaData;
                        d->dcc->call("kio_uiserver", "UIServer",
                                "showSSLInfoDialog(QString,KIO::MetaData)",
                                data, ignoretype, ignore);
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
          kdDebug(7029) << "TCPSlaveBase/SSL error in cert code."
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

      if (ksv == KSSLCertificate::Ok && _IPmatchesCN) {
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
             QString msg = i18n("The server certificate failed the "
                                "authenticity test (%1).");
             result = messageBox(WarningYesNoCancel,
                                 msg.arg(ourHost),
                                 i18n("Server Authentication"),
                                 i18n("&Details..."),
                                 i18n("Co&ntinue"));
                if (result == KMessageBox::Yes) {
                   if (!d->dcc) {
                      d->dcc = new DCOPClient;
                      d->dcc->attach();
                      if (!d->dcc->isApplicationRegistered("kio_uiserver")) {
                         KApplication::startServiceByDesktopPath("kio_uiserver.desktop",
                         QStringList() );
                      }
                   }
                   QByteArray data, ignore;
                   QCString ignoretype;
                   QDataStream arg(data, IO_WriteOnly);
                   arg << theurl << mOutgoingMetaData;
                        d->dcc->call("kio_uiserver", "UIServer",
                                "showSSLInfoDialog(QString,KIO::MetaData)",
                                data, ignoretype, ignore);
                }
          } while (result == KMessageBox::Yes);

          if (result == KMessageBox::No) {
             setMetaData("ssl_action", "accept");
             rc = 1;
             cp = KSSLCertificateCache::Accept;
             d->cc->addHost(pc, ourHost);
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
          d->cc->addCertificate(pc, cp, permacache);
        }
      }
    }


   if (rc == -1) return rc;

   if (metaData("ssl_activate_warnings") == "TRUE") {
   //  - entering SSL
   if (!isChild && metaData("ssl_was_in_use") == "FALSE" &&
                                        d->kssl->settings()->warnOnEnter()) {
     int result;
     do {
                result = messageBox( WarningYesNo, i18n("You are about to "
                                                        "enter secure mode. "
                                                        "All transmissions "
                                                        "will be encrypted "
                                                        "unless otherwise "
                                                        "noted.\nThis means "
                                                        "that no third party "
                                                        "will be able to "
                                                        "easily observe your "
                                                        "data in transit."),
                                                   i18n("Security Information"),
                                                   i18n("Display SSL "
                                                        "Information"),
                                                   i18n("Continue") );
      if ( result == KMessageBox::Yes )
      {
          if (!d->dcc) {
             d->dcc = new DCOPClient;
             d->dcc->attach();
             if (!d->dcc->isApplicationRegistered("kio_uiserver")) {
                KApplication::startServiceByDesktopPath("kio_uiserver.desktop",
                QStringList() );
             }
          }
          QByteArray data, ignore;
          QCString ignoretype;
          QDataStream arg(data, IO_WriteOnly);
          arg << theurl << mOutgoingMetaData;
          d->dcc->call("kio_uiserver", "UIServer",
                       "showSSLInfoDialog(QString,KIO::MetaData)",
                       data, ignoretype, ignore);
      }
      } while (result != KMessageBox::No);
   }

   }   // if ssl_activate_warnings


   kdDebug(7029) << "SSL connection information follows:" << endl
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
    if ( m_iSock == -1 )
      return false;

    fd_set rdfs;
    FD_ZERO(&rdfs);
    FD_SET(m_iSock , &rdfs);

    struct timeval tv;
    tv.tv_usec = 0;
    tv.tv_sec = 0;
    int retval;
    do {
       retval = KSocks::self()->select(m_iSock+1, &rdfs, NULL, NULL, &tv);
    } while ((retval == -1) && (errno == EAGAIN));
    // retval == -1 ==> Error  
    // retval ==  0 ==> Connection Idle
    // retval >=  1 ==> Connection Active
    //kdDebug(7029) << "TCPSlaveBase::isConnectionValid: select returned: "
    //              << retval << endl;

    if (retval == -1)
       return false;
    
    if (retval == 0)
       return true;
       
    // Connection is active, check if it has closed.
    char buffer[100];
    do {
       retval = KSocks::self()->recv(m_iSock, buffer, 80, MSG_PEEK);
                     
    } while ((retval == -1) && (errno == EAGAIN));
    //kdDebug(7029) << "TCPSlaveBase::isConnectionValid: recv returned: "
    //                 << retval << endl;
    if (retval <= 0)
       return false; // Error or connection closed.
       
    return true; // Connection still valid.
}


bool TCPSlaveBase::waitForResponse( int t )
{
  fd_set rd;
  struct timeval timeout;

  if ( (m_bIsSSL || d->usingTLS) && !d->useSSLTunneling && d->kssl )
    if (d->kssl->pending() > 0)
        return true;

  FD_ZERO(&rd);
  FD_SET(m_iSock, &rd);

  timeout.tv_usec = 0;
  timeout.tv_sec = t;
  time_t startTime;

  int rc;
  int n = t;

reSelect:
  startTime = time(NULL);
  rc = KSocks::self()->select(m_iSock+1, &rd, NULL, NULL, &timeout);

  if (rc == -1)
    return false;

  if (FD_ISSET(m_iSock, &rd))
    return true;

  // Well it returned but it wasn't set.  Let's see if it
  // returned too early (perhaps from an errant signal) and
  // start over with the remaining time
  int timeDone = time(NULL) - startTime;
  if (timeDone < n) 
  {
    n -= timeDone;
    timeout.tv_sec = n;
    goto reSelect;
  }

  return false; // Timed out!
}

int TCPSlaveBase::connectResult()
{
    return d->status;
}

void TCPSlaveBase::setBlockConnection( bool b )
{
    d->block = b;
}

void TCPSlaveBase::setConnectTimeout( int t )
{
    d->timeout = t;
}

bool TCPSlaveBase::isSSLTunnelEnabled()
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
    kdDebug(7029) << "TCPSlaveBase::doSSLHandShake: " << endl;
    QString msgHost = d->host;

    d->kssl->reInitialize();

    certificatePrompt();

    if ( !d->realHost.isEmpty() )
    {
      msgHost = d->realHost;
    }

    kdDebug(7029) << "Setting real hostname: " << msgHost << endl;
    d->kssl->setPeerHost(msgHost);

    d->status = d->kssl->connect(m_iSock);
    if (d->status < 0) 
    {
        closeDescriptor();
        if ( sendError )
            error( ERR_COULD_NOT_CONNECT, msgHost);
        return false;
    }

    setMetaData("ssl_in_use", "TRUE");

    int rc = verifyCertificate();
    if ( rc != 1 ) 
    {
        d->status = -1;
        closeDescriptor();
        if ( sendError )
            error( ERR_COULD_NOT_CONNECT, msgHost);
        return false;
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

