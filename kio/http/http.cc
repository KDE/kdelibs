// -*- c-basic-offset: 2 -*-
//
//  Portions Copyright 2000 George Staikos <staikos@kde.org>
//  (mostly SSL related)
//

#include <config.h>

#ifdef HAVE_LIBZ
#define DO_GZIP
#endif

#ifdef HTTPS
#ifndef HAVE_SSL
#define NOT_IMPLEMENTED
#endif
#else
#undef HAVE_SSL
#undef DO_SSL
#endif

#ifdef HAVE_SSL
#define DO_SSL
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>         // Needed on some systems.
#endif

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>
#include <fcntl.h>
#ifdef DO_GZIP
#include <zlib.h>
#endif

#include <qregexp.h>

#include <kapp.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kprotocolmanager.h>
#include <kdatastream.h>
#include <kextsock.h>
#include <ksocks.h>
#include <kurl.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <kservice.h>
#include <krfcdate.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kmimemagic.h>
#ifdef DO_SSL
#include <kssl.h>
#endif

#include <kio/http_slave_defaults.h>
#include <kio/ioslave_defaults.h>

#include "http.h"

using namespace KIO;

#define MAX_IPC_SIZE (1024*8)

// Default expire time in seconds: 1 min.
#define DEFAULT_EXPIRE (1*60)

// time frame in which the cache cleaner is run
// (30 Minutes)
#ifndef MAX_CACHE_AGE
#define MAX_CACHE_AGE 30*60
#endif

#define DEFAULT_MIME_TYPE       "text/html"
#define DEFAULT_ACCEPT_HEADER   "text/*, image/png, image/jpeg, image/gif, image/*, */*"

extern "C" {
  void sigalrm_handler(int);
  int kdemain(int argc, char **argv);
};

int kdemain( int argc, char **argv )
{
  KLocale::setMainCatalogue("kdelibs");
  KInstance instance( "kio_http" );

  kdDebug(7113) << "Starting " << getpid() << endl;

  if (argc != 4)
  {
     fprintf(stderr, "Usage: kio_http protocol domain-socket1 domain-socket2\n");
     exit(-1);
  }

  HTTPProtocol slave(argv[1], argv[2], argv[3]);
  slave.dispatchLoop();
  return 0;
}

/*
 * We'll use an alarm that will set this flag when transfer has timed out
 */
char sigbreak = 0;

void sigalrm_handler(int )
{
  sigbreak = 1;
}

void setup_alarm(unsigned int timeout)
{
  sigbreak = 0;
  alarm(timeout);
  signal(SIGALRM, sigalrm_handler);
}


/***********************************  Generic utility functions ********************/

static char * trimLead (char *orig_string)
{
  while (*orig_string == ' ')
    orig_string++;
  return orig_string;
}

/************************************** HTTPProtocol **********************************************/

HTTPProtocol::HTTPProtocol( const QCString &protocol, const QCString &pool, const QCString &app )
             :SlaveBase( protocol , pool, app )
{
  m_protocol = protocol;
  kdDebug(7113) << "HTTPProtocol: mProtocol=" << mProtocol << " m_protocol=" << m_protocol << endl;
  m_maxCacheAge = 0;
  m_sock = 0;
  m_fcache = 0;
  m_bKeepAlive = false;
  m_iSize = -1;

  m_dcopClient = new DCOPClient();
  if (!m_dcopClient->attach())
  {
     kdDebug(7103) << "Can't connect with DCOP server." << endl;
  }

  // Make sure to set some values here... just to be on the safe side.
  m_proxyConnTimeout = DEFAULT_PROXY_CONNECT_TIMEOUT;
  m_remoteConnTimeout = DEFAULT_CONNECT_TIMEOUT;
  m_remoteRespTimeout = DEFAULT_RESPONSE_TIMEOUT;
  m_maxCacheAge = DEFAULT_MAX_CACHE_AGE;
  m_bUseCache = true;

  reparseConfiguration();
  setMultipleAuthCaching( true );
  
#ifdef DO_SSL
  m_bUseSSL=true;
#endif

  m_sContentMD5 = "";
  ProxyAuthentication = AUTH_None;
  Authentication = AUTH_None;
  m_HTTPrev = HTTP_Unknown;

  cleanCache();
}

HTTPProtocol::~HTTPProtocol()
{
  m_bKeepAlive = false;
  http_close();
}

#ifdef DO_SSL
void HTTPProtocol::initSSL() {
  m_ssl.initialize();
  kdDebug(7103) << "SSL was initialised." << endl;
}

void HTTPProtocol::closeSSL() {
  m_ssl.close();
  kdDebug(7103) << "SSL was deinitialised." << endl;
}

void HTTPProtocol::resetSSL() {
  m_ssl.reInitialize();
}
#endif

int HTTPProtocol::openStream() {
#ifdef DO_SSL
  if (m_bUseSSL) {
    initSSL();     // incase it's not initialised somehow - it's wrapped though
    kdDebug(7103) << "SSL about to connect." << endl;
    if (m_ssl.connect(m_sock) == -1) {
      kdDebug(7103) << "SSL connection failed." << endl;
      return false;
    }
    KSSLCertificate::KSSLValidation ksv =
                               m_ssl.peerInfo().getPeerCertificate().validate();
    bool matchingCN = m_ssl.peerInfo().certMatchesAddress();
    kdDebug(7103) << "SSL connection established." << endl;
    kdDebug(7103) << "SSL connection information follows:" << endl
                  << "+-----------------------------------------------" << endl
                  << "| Cipher: " << m_ssl.connectionInfo().getCipher() << endl
                  << "| Description: " << m_ssl.connectionInfo().getCipherDescription()
                  << "| Version: " << m_ssl.connectionInfo().getCipherVersion() << endl
                  << "| Strength: " << m_ssl.connectionInfo().getCipherUsedBits()
                  << " of " << m_ssl.connectionInfo().getCipherBits()
                  << " bits used." << endl
                  << "| PEER:" << endl
                  << "| Subject: " << m_ssl.peerInfo().getPeerCertificate().getSubject() << endl
                  << "| Issuer: " << m_ssl.peerInfo().getPeerCertificate().getIssuer() << endl
                  << "| Validation: " << (int)ksv << endl
                  << "| Certificate matches CN: " << matchingCN << endl
                  << "+-----------------------------------------------"
                  << endl;

      if (!matchingCN && false) {    // FIXME: remove the "&& false" once the
                                     // certificate cache has been enabled
                                     // and make a config option to enable this
                                     // prompt in kcmcrypto.
        int result = messageBox( WarningYesNo,
                               i18n("This site presented an SSL certificate"
                                    " for an address which is not the same as"
                                    " the sites address.\nIt could mean that the"
                                    " certificate has been compromised.  Are"
                                    " you sure you wish to continue?"),
                               i18n("Bad Address"));
        if ( result == KMessageBox::No )
        {
          m_ssl.close();
          return false;
        }
      }

    setMetaData("ssl_in_use", "TRUE");
    setMetaData("ssl_peer_cert_subject", m_ssl.peerInfo().getPeerCertificate().getSubject());
    setMetaData("ssl_peer_cert_issuer", m_ssl.peerInfo().getPeerCertificate().getIssuer());
    setMetaData("ssl_cipher", m_ssl.connectionInfo().getCipher());
    setMetaData("ssl_cipher_desc", m_ssl.connectionInfo().getCipherDescription());
    setMetaData("ssl_cipher_version", m_ssl.connectionInfo().getCipherVersion());
    setMetaData("ssl_cipher_used_bits", QString::number(m_ssl.connectionInfo().getCipherUsedBits()) );
    setMetaData("ssl_cipher_bits", QString::number(m_ssl.connectionInfo().getCipherBits()) );
    setMetaData("ssl_peer_ip", m_ssl_ip);
    setMetaData("ssl_cert_state", QString::number(ksv) );
    setMetaData("ssl_good_from", m_ssl.peerInfo().getPeerCertificate().getNotBefore());
    setMetaData("ssl_good_until", m_ssl.peerInfo().getPeerCertificate().getNotAfter());

    return true;
  }
#endif
  setMetaData("ssl_in_use", "FALSE");
  return true;
}


ssize_t HTTPProtocol::write (const void *buf, size_t nbytes)
{
#ifdef DO_SSL
  if (m_bUseSSL)
    return m_ssl.write((char *)buf, nbytes);
#endif
  int n;
 keeptrying:
  if ((n = ::write(m_sock, buf, nbytes)) != (int)nbytes)
    {
      if (n == -1 && errno == EINTR)
        goto keeptrying;
    }
  return n;
}

char *HTTPProtocol::gets (char *s, int size)
{
  int len=0;
  char *buf=s;
  char mybuf[2]={0,0};
  while (len < size) {
    read(mybuf, 1);
    if (*mybuf == '\r') // Ignore!
      continue;
    if (*mybuf == '\n')
      break;
    *buf++ = *mybuf;
    len++;
  }
  *buf=0;
  return s;
}

ssize_t HTTPProtocol::read (void *b, size_t nbytes)
{
  ssize_t ret = 0;
  if (m_lineCount > 0)
  {
     ret = ( nbytes < m_lineCount ? nbytes : m_lineCount );
     m_lineCount -= ret;
     memcpy(b, m_linePtr, ret);
     m_linePtr += ret;
     return ret;
  }
  if (nbytes == 1)
  {
     m_lineCount = read(m_lineBuf, 1024); // Read into buffer
     m_linePtr = m_lineBuf;
     if (m_lineCount <= 0)
        return ret;
     return read(b, 1); // Read from buffer
  }
#ifdef DO_SSL
  if (m_bUseSSL) {
    m_bEOF=false;
    ret=m_ssl.read((char *)b, nbytes);
    if (ret==0) m_bEOF=true;
    return ret;
  }
#endif
  do {
     ret = KSocks::self()->read(m_sock, b, nbytes);
     if (ret == 0) m_bEOF = true;
  }
  while (( ret == -1) && ((errno == EAGAIN) || (errno == EINTR)));

  return ret;
}

bool HTTPProtocol::eof()
{
  return m_bEOF;
}

bool HTTPProtocol::http_isConnected()
{
    if (!m_sock)
      return false;

    fd_set rdfs;
    FD_ZERO(&rdfs);
    FD_SET(m_sock , &rdfs);

    struct timeval tv;
    tv.tv_usec = 0;
    tv.tv_sec = 0;
    int retval = select(m_sock+1, &rdfs, NULL, NULL, &tv);
    // retval ==  0 ==> Connection Idle
    // retval >=  1 ==> Connection Active
    if ( retval == -1 )
        return false;       // should really never happen, but just in-case...
    else if ( retval > 0 )
    {
      char buffer[100];
      retval = recv(m_sock, buffer, 80, MSG_PEEK);
      // retval ==  0 ==> Connection clased
      if ( retval == 0 )
        return false;
    }
    return true;
}

void HTTPProtocol::http_checkConnection()
{
  if (m_sock)
  {
     bool closeDown = false;
     if ( m_request.do_proxy && m_state.do_proxy )
     {
        // Keep the connection to the proxy.
     }
     else if ( !m_state.do_proxy && !m_request.do_proxy )
     {
        if (m_state.hostname != m_request.hostname)
        {
           closeDown = true;
        }
        else if (m_state.port != m_request.port)
        {
           closeDown = true;
        }
        else if (m_state.user != m_request.user)
        {
           closeDown = true;
        }
        else if (m_state.passwd != m_request.passwd)
        {
           closeDown = true;
        }
     }
     else
     {
        closeDown = true;
     }

     if (!closeDown && !http_isConnected())
        closeDown = true;

     if (closeDown)
        http_closeConnection();
  }

  // Let's update our current state
  m_state.hostname = m_request.hostname;
  m_state.port = m_request.port;
  m_state.user = m_request.user;
  m_state.passwd = m_request.passwd;
  m_state.do_proxy = m_request.do_proxy;
}

static bool waitForHeader( int sock, int maxTimeout )
{
  fd_set rd, wr;
  struct timeval timeout;

  int n = maxTimeout; // Timeout in seconds
  while(n--){
      FD_ZERO(&rd);
      FD_ZERO(&wr);
      FD_SET(sock, &rd);

      timeout.tv_usec = 0;
      timeout.tv_sec = 1; // 1 second

      select(sock + 1, &rd, &wr, (fd_set *)0, &timeout);

      if (FD_ISSET(sock, &rd))
      {
         return true;
      }
  }
  return false; // Timeout
}

bool
HTTPProtocol::http_openConnection()
{
    m_bKeepAlive = false;
    KExtendedSocket ks;
    if ( m_state.do_proxy )
    {
        QString proxy_host = m_proxyURL.host();
        int proxy_port = m_proxyURL.port();
        kdDebug(7113) << "http_openConnection " << proxy_host << " " << proxy_port << endl;
        // yep... open up a connection to the proxy instead of our host
        ks.setAddress(proxy_host, proxy_port);
        ks.setTimeout(m_proxyConnTimeout);
        infoMessage( i18n("Connecting to <b>%1</b>...").arg(m_state.hostname) );
        if (ks.connect() < 0)
        {
            if (ks.status() == IO_LookupError)
                error(ERR_UNKNOWN_PROXY_HOST, proxy_host);
            else
                error(ERR_COULD_NOT_CONNECT, i18n("proxy %1, port %2").arg(proxy_host).arg(proxy_port) );
            return false;
        }
        m_sock = ks.fd();

        // SSL proxying requires setting up a tunnel through the proxy server
        // with the CONNECT directive.
#ifdef DO_SSL
        if (m_bUseSSL)
        {
            // we might have DO_SSL but not m_bUseSSL someday
            kdDebug(7113) << "http proxy for SSL - setting up" << endl;
            // Set socket blocking.
            fcntl(m_sock, F_SETFL, ( fcntl(m_sock, F_GETFL) & ~O_NDELAY));
            QString proxyconheader = QString("CONNECT %1:%2 HTTP/1.1\r\n\r\n").arg(m_request.hostname).arg(m_request.port);
            // WARNING: ugly hack alert!  We don't want to use the SSL routines
            //          for this code so we have to disabled it temporarily.
            kdDebug(7113) << "Sending connect header: " << proxyconheader << endl;
            bool useSSLSaved = m_bUseSSL;
            m_bUseSSL = false;
            bool sendOk = (write(proxyconheader.latin1(), proxyconheader.length())
                           == (ssize_t) proxyconheader.length());
            char buffer[513];
            if (!sendOk)
            {
                // FIXME: do we have to close() the connection here?
                //        also the error code should be changed
                error(ERR_COULD_NOT_CONNECT, i18n("proxy %1, port %2").arg(proxy_host).arg(proxy_port) );
                m_bUseSSL = useSSLSaved;
                return false;
            }
            if (!waitForHeader(m_sock, m_remoteRespTimeout) )
            {
                // FIXME: a good workaround would be to fallback to non-proxy mode
                //        here if we can.
                // FIXME: do we have to close() the connection here?
                //        also the error code should be changed
                error(ERR_COULD_NOT_CONNECT, i18n("proxy %1, port %2").arg(proxy_host).arg(proxy_port) );
                m_bUseSSL = useSSLSaved;
                return false;
            }

            // In SSL mode we normally don't use this.  Does it cause a resource
            // leak that I'm not cleaning this up here if we're in SSL mode?
            int rhrc = KSocks::self()->read(m_sock, buffer, sizeof(buffer)-1);
            buffer[sizeof(buffer)-1] = 0;  // just in case so we don't run away!
            if (rhrc == -1 || strncmp(buffer, "HTTP/1.0 200", 12))
            {
                // FIXME: a good workaround would be to fallback to non-proxy mode
                // here if we can.
                // FIXME: do we have to close() the connection here? also the error
                // code should be changed
                error(ERR_COULD_NOT_CONNECT, i18n("proxy %1, port %2").arg(proxy_host).arg(proxy_port) );
                m_bUseSSL = useSSLSaved;
                return false;
            }
            m_bUseSSL = useSSLSaved;
        }  // if m_bUseSSL
        // m_ssl_ip = i18n("Proxied by %1.").arg(inet_ntoa(m_proxySockaddr.sin_addr));
        KSocketAddress *sa = ks.peerAddress();
        m_ssl_ip = i18n("Proxied by %1.").arg(sa->pretty());
#endif // DO_SSL
        ks.release();
    }
    else
    {
      // apparently we don't want a proxy.  let's just connect directly
        ks.setAddress(m_state.hostname, m_state.port);
        ks.setTimeout(m_remoteConnTimeout);
        if (ks.connect() < 0)
        {
            if (ks.status() == IO_LookupError)
                error(ERR_UNKNOWN_HOST, m_state.hostname);
            else
            {
                if (m_state.port != m_DefaultPort)
                    error(ERR_COULD_NOT_CONNECT, i18n("%1 (port %2)").arg(m_state.hostname).arg(m_state.port) );
                else
                    error(ERR_COULD_NOT_CONNECT, m_state.hostname );
            }
            return false;
         }
         m_sock = ks.fd();

#ifdef DO_SSL
        // m_ssl_ip = inet_ntoa(server_name.sin_addr);
        KSocketAddress *sa = ks.peerAddress();
        if (sa->isA("KInetSocketAddress"))
            m_ssl_ip = ((KInetSocketAddress*)sa)->prettyHost();
        else
            m_ssl_ip = sa->pretty();
#endif
        ks.release();
    }

    // Set socket blocking.
    ks.setBlockingMode(true);

    // Placeholder
    if (!openStream())
    {
      error( ERR_COULD_NOT_CONNECT, m_state.hostname );
      return false;
    }
    kdDebug(7103) << time(0L) << " Sending connected" << endl;
    // Tell the application that we are connected, and that the metadata (e.g. ssl) is ready
    connected();
    return true;
}

bool HTTPProtocol::checkSSL()
{
  // Check if we need to pop up a dialog box to the user
  if ( metaData( "ssl_activate_warnings" ) == "TRUE" )
  {
    kdDebug(7103) << "SSL warnings activated" << endl;
    bool ssl_was_in_use = metaData( "ssl_was_in_use" ) == "TRUE";
    kdDebug() << "ssl_was_in_use: " << ssl_was_in_use << endl;

#ifdef DO_SSL
    kdDebug(7103) << "m_bUseSSL: " << m_bUseSSL << endl;
    kdDebug(7103) << "warnOnEnter: " << m_ssl.settings()->warnOnEnter() << endl;
    kdDebug(7103) << "warnOnUnencrypted: " << m_ssl.settings()->warnOnUnencrypted() << endl;
    kdDebug(7103) << "warnOnLeave: " << m_ssl.settings()->warnOnLeave() << endl;
    kdDebug(7103) << "warnOnMixed: " << m_ssl.settings()->warnOnMixed() << endl;

    if ( !ssl_was_in_use && m_bUseSSL && m_ssl.settings()->warnOnEnter() )
    {
      kdDebug(7103) << "ENTERING SSL" << endl;
      int result = messageBox( WarningYesNo,
                               i18n("You are about to enter secure mode."
                                    " All transmissions will be encrypted unless"
                                    " otherwise noted.\nThis means that no third"
                                    " party will be able to easily observe your"
                                    " data in transfer."),
                               i18n("Security information"),
                               i18n("Display SSL Information"),
                               i18n("Continue") );
      if ( result == KMessageBox::Yes )
      {
         // Force sending of the metadata
         sendMetaData();
         messageBox( SSLMessageBox, m_request.url.prettyURL() );
      }
    }
#else

    if ( ssl_was_in_use )
    {
      kdDebug(7103) << "LEAVING SSL" << endl;
      // We come from the SSL world. Check if we should popup a warning.
      KConfig cfg("cryptodefaults");
      cfg.setGroup("Warnings");
      if ( cfg.readBoolEntry("OnLeave", true ) )
      {
        int result = messageBox(WarningContinueCancel,
                                i18n("You are about to leave secure mode. "
                                     "Transmissions will no longer be "
                                     "encrypted.\nThis means that a "
                                     "third party could observe your data "
                                     "in transfer."),
                                i18n("Security information"),
                                i18n("Continue Loading"));
        if ( result == KMessageBox::Cancel )
        {
          kdDebug(7103) << "Cancelling the loading" << endl;
          if (m_bErrorPage)
             errorPage();
          else
             error( ERR_USER_CANCELED, "ssl" );
          return false;
        }

      }
    }
#endif
/*
  if ( doing a post :/ && !m_bUseSSL && m_ssl.settings()->warnOnUnencrypted() )
  {
  kdDebug() << "UNENCRYPTED" << endl;
  // TODO
  }
*/

  }
  return true;
}

/**
 * This function is responsible for opening up the connection to the remote
 * HTTP server and sending the header.  If this requires special
 * authentication or other such fun stuff, then it will handle it.  This
 * function will NOT receive anything from the server, however.  This is in
 * contrast to previous incarnations of 'http_open'.
 *
 * The reason for the change is due to one small fact: some requests require
 * data to be sent in addition to the header (POST requests) and there is no
 * way for this function to get that data.  This function is called in the
 * slotPut() or slotGet() functions which, in turn, are called (indirectly) as
 * a result of a KIOJob::put() or KIOJob::get().  It is those latter functions
 * which are responsible for starting up this ioslave in the first place.
 * This means that 'http_open' is called (essentially) as soon as the ioslave
 * is created -- BEFORE any data gets to this slave.
 *
 * The basic process now is this:
 *
 * 1) Open up the socket and port
 * 2) Format our request/header
 * 3) Send the header to the remote server
 */
bool HTTPProtocol::http_open()
{
#ifdef NOT_IMPLEMENTED
  error( ERR_UNSUPPORTED_PROTOCOL, m_protocol );
  return false;
#else
  http_checkConnection();

  m_fcache = 0;
  m_lineCount = 0;
  m_bCachedRead = false;
  m_bCachedWrite = false;
  m_bMustRevalidate = false;
  m_bEOF = false;
  if (m_bUseCache)
  {
     m_fcache = checkCacheEntry( );
#ifdef DO_SSL
     if ((m_request.cache == CC_Reload) || m_bUseSSL)
#else
     if ((m_request.cache == CC_Reload) && m_fcache)
#endif
     {
        if (m_fcache)
          fclose(m_fcache);
        m_fcache = 0;
     }
     m_bCachedWrite = true;

     if (m_fcache && !m_bMustRevalidate)
     {
        // Cache entry is OK.
        m_bCachedRead = true; // Cache hit.
        return true;
     }
     else if (!m_fcache)
     {
        m_bMustRevalidate = false; // Cache miss
     }
     else
     {
        // Conditional cache hit. (Validate)
     }

     if (m_request.cache == CC_CacheOnly)
     {
        error( ERR_DOES_NOT_EXIST, m_request.url.url() );
        return false;
     }
  }

  // Let's also clear out some things, so bogus values aren't used.
  // m_HTTPrev = HTTP_Unknown;
  m_iWWWAuthCount = 0;
  m_iProxyAuthCount = 0;
  m_sContentMD5 = QString::null;
  m_strMimeType = QString::null;
  m_qContentEncodings.clear();
  m_qTransferEncodings.clear();
  m_bChunked = false;
  m_bError = false;
  m_bErrorPage = (metaData("errorPage") != "false");
  m_iSize = -1;

  // Let's try to open up our socket if we don't have one already.
  if (!m_sock)
  {
    if (!http_openConnection())
       return false;
  }

  // Clean up previous POST
  bool moreData = false;
  // Variable to hold the entire header...
  QString header;

  // Determine if this is a POST or GET method
  switch ( m_request.method) {
  case HTTP_GET:
      header = "GET ";
      break;
  case HTTP_PUT:
      header = "PUT ";
      moreData = true;
      m_bCachedWrite = false; // Do not put nay result in the cache
      break;
  case HTTP_POST:
      header = "POST ";
      moreData = true;
      m_bCachedWrite = false; // Do not put nay result in the cache
      break;
  case HTTP_HEAD:
      header = "HEAD ";
      break;
  case HTTP_DELETE:
      header = "DELETE ";
      m_bCachedWrite = false; // Do not put nay result in the cache
      break;
  }

  // format the URI
  if (m_state.do_proxy)
  {
    KURL u;
    u.setProtocol( m_protocol );
    u.setHost( m_state.hostname );
    u.setPort( m_state.port );
    u.setEncodedPathAndQuery( m_request.url.encodedPathAndQuery(0,true) );
/*
    // For all protocols other than HTTP/HTTPS we need to prompt the user
    // for password if a username is specified.  However, unlike normal direct
    // connections, we have no responsiblities of managing (caching) thess
    // passwords since we are simply a gateway to a proxy server! (DA)
    if ( m_protocol.find("http",0, false) == -1 &&
         !m_state.user.isEmpty() &&
         m_state.passwd.isEmpty() )
    {
      AuthInfo info;
      info.username = m_state.user;
      info.commentLabel = i18n( "Site:" );
      info.comment = i18n("<b>%1</b>").arg( m_state.hostname );

      if ( !openPassDlg( info ) )
      {
        error( ERR_USER_CANCELED, m_state.hostname );
        return false;
      }
      else
      {
        u.setUser( info.username );
        u.setPass( info.password );
        m_request.user = info.username;
        m_request.passwd = info.password;
      }
    }
    else
    {
      if ( !m_state.user.isEmpty() && !m_state.passwd.isEmpty() )
      {
        u.setUser( m_state.user );
        u.setPass( m_state.passwd );
      }
    }
*/
    header += u.url();
  }
  else
    header += m_request.url.encodedPathAndQuery(0, true);

  header += " HTTP/1.1\r\n"; /* start header */

#ifdef DO_SSL
  // With SSL we don't keep the connection.
  if (m_bUseSSL)
    header += "Connection: Close\r\n";
  else
    header += "Connection: Keep-Alive\r\n";
#else
  header += "Connection: Keep-Alive\r\n";
#endif

  if ( config()->readBoolEntry("SendUserAgent", true) )
  {
    QString agent = config()->readEntry("UserAgent");

    if( agent.isEmpty() )
      agent = KProtocolManager::defaultUserAgent(
      		config()->readEntry("UserAgentKeys", DEFAULT_USER_AGENT_KEYS));

    if( !agent.isEmpty() )
      header += "User-Agent: " + agent + "\r\n";
  }

  QString referrer = metaData("referrer");
  if (!referrer.isEmpty())
  {
     // HTTP uses "Referer" although the correct
     // spelling is "referrer"
     header += "Referer: "+referrer+"\r\n";
  }

  // Adjust the offset value based on the "resume"
  // meta-data.
  QString resumeOffset = metaData("resume");
  if ( !resumeOffset.isEmpty() )
      m_request.offset = resumeOffset.toInt();

  if ( m_request.offset > 0 )
  {
    header += QString("Range: bytes=%1-\r\n").arg(m_request.offset);
    kdDebug(7103) << "kio_http : Range = " << m_request.offset << endl;
  }


  if ( m_request.cache == CC_Reload ) { /* No caching for reload */
    header += "Pragma: no-cache\r\n"; /* for HTTP/1.0 caches */
    header += "Cache-control: no-cache\r\n"; /* for HTTP >=1.1 caches */
  }

  if (m_bMustRevalidate) { /* conditional get */
    if (!m_etag.isEmpty())
       header += "If-None-Match: "+m_etag+"\r\n";
    if (!m_lastModified.isEmpty())
       header += "If-Modified-Since: "+m_lastModified+"\r\n";
  }

  header += "Accept: ";
  QString acceptHeader = metaData("accept");
  if (!acceptHeader.isEmpty())
     header += acceptHeader;
  else
     header += DEFAULT_ACCEPT_HEADER;
  header += "\r\n";

#ifdef DO_GZIP
  // Content negotiation
  // header += "Accept-Encoding: x-gzip, x-deflate, gzip, deflate, identity\r\n";
    header += "Accept-Encoding: x-gzip, gzip, identity\r\n";
#endif

  // Charset negotiation:
  if ( !m_strCharsets.isEmpty() )
    header += "Accept-Charset: " + m_strCharsets + "\r\n";

  // Language negotiation:
  if ( !m_strLanguages.isEmpty() )
    header += "Accept-Language: " + m_strLanguages + "\r\n";

  header += "Host: "; /* support for virtual hosts and required by HTTP 1.1 */
  if (m_state.hostname.find(':') != -1)
    {
      // This is an IPv6 (not hostname)
      header += '[';
      header += m_state.hostname;
      header += ']';
    }
  else
    header += m_state.hostname;
  if (m_state.port != m_DefaultPort)
  {
    header += QString(":%1").arg(m_state.port);
  }
  header += "\r\n";

  QString cookieStr;
  QString cookieMode = metaData("cookies").lower();
  if (cookieMode == "manual")
  {
     m_cookieMode = CookiesManual;
     cookieStr = metaData("setcookies");
  }
  else if (cookieMode == "none")
  {
     m_cookieMode = CookiesNone;
  }
  else
  {
     m_cookieMode = CookiesAuto;
     if (m_bUseCookiejar)
        cookieStr = findCookies( m_request.url.url());
  }

  if (!cookieStr.isEmpty())
    header += cookieStr + "\r\n";

  if (m_request.method == HTTP_POST)
  {
      header += metaData("content-type");
      header += "\r\n";
  }

  // Only check for a cached copy if the previous
  // response was NOT a 401 or 407.
  if ( m_responseCode != 401 && m_responseCode != 407 )
  {
    AuthInfo info;
    info.url = m_request.url;
    info.verifyPath = true;    
    if ( !m_request.user.isEmpty() )
      info.username = m_request.user;
    if ( checkCachedAuthentication( info ) )
    {
      Authentication = info.digestInfo.isEmpty() ? AUTH_Basic : AUTH_Digest ;
      m_state.user   = info.username;
      m_state.passwd = info.password;
      m_strRealm = info.realmValue;
      if ( Authentication == AUTH_Digest )
        m_strAuthorization = info.digestInfo;
    }
  }

  switch ( Authentication )
  {
    case AUTH_Basic:
        header += createBasicAuth();
        header+="\r\n";
        break;
    case AUTH_Digest:
        header += createDigestAuth();
        header+="\r\n";
        break;
    case AUTH_None:
    default:
        break;

  }

  /********* Only for debugging purpose... *********/
  if ( Authentication != AUTH_None )
  {
    kdDebug(7113) << "(" << getpid() << ") Using Authentication: " << endl
                  << " HOST= " << m_state.hostname << endl
                  << " PORT= " << m_state.port << endl
                  << " USER= " << m_state.user << endl
                  << " PASSWORD= [protected]" << endl
                  << " REALM= " << m_strRealm << endl
                  << " EXTRA= " << m_strAuthorization  << endl;
  }

  // Do we need to authorize to the proxy server ?
  if ( m_state.do_proxy )
  {
    // We keep proxy authentication locally until they are
    // changed.  Thus, no need to check with kdesud on every
    // connection!!
    if ( m_strProxyRealm.isEmpty() )
    {
      AuthInfo info;
      info.url = m_proxyURL;
      info.username = m_proxyURL.user();
      info.password = m_proxyURL.pass();
      info.verifyPath = true;      

      // If the proxy URL already contains username
      // and password simply attempt to retrieve it
      // without prompting the user...
      if ( !info.username.isEmpty() && !info.password.isEmpty() )
      {
        ProxyAuthentication = AUTH_Basic;
      }
      else
      {
        if ( checkCachedAuthentication(info) )
        {
          m_proxyURL.setUser( info.username );
          m_proxyURL.setPass( info.password );
          m_strProxyRealm = info.realmValue;
          if ( info.digestInfo.isEmpty() )
            ProxyAuthentication = AUTH_Basic;
          else
          {
            ProxyAuthentication = AUTH_Digest;
            m_strProxyAuthorization = info.digestInfo;
          }
        }
        else
        {
          ProxyAuthentication = AUTH_None;
        }
      }
    }

    /********* Only for debugging purpose... *********/
    if ( ProxyAuthentication != AUTH_None )
    {
        kdDebug(7113) << "(" << getpid() << ") Using Proxy Authentication: " << endl
                      << " HOST= " << m_proxyURL.host() << endl
                      << " PORT= " << m_proxyURL.port() << endl
                      << " USER= " << m_proxyURL.user() << endl
                      << " PASSWORD= [protected]" << endl
                      << " REALM= " << m_strProxyRealm << endl
                      << " EXTRA= " << m_strProxyAuthorization  << endl;
    }

    switch ( ProxyAuthentication )
    {
        case AUTH_Basic:
            header+= createBasicAuth( true );
            header+="\r\n";
            break;
        case AUTH_Digest:
            header+= createDigestAuth( true );
            header+="\r\n";
            break;
        case AUTH_None:
        default:
            break;
    }
  }

  if ( !moreData )
    header += "\r\n";  /* end header */

  kdDebug(7113) << "(" << getpid() << ") Sending header: \n=======" << endl
                << header << "\n=======" << endl;

  // now that we have our formatted header, let's send it!
  bool sendOk;
  sendOk = (write(header.latin1(), header.length()) == (ssize_t) header.length());
  if (!sendOk) {
    kdDebug(7103) << "Connection broken! (" << m_state.hostname << ")" << endl;
    if (m_bKeepAlive)
    {
       // With a Keep-Alive connection this can happen.
       // Just reestablish the connection.
       http_closeConnection();
       if (!http_openConnection())
          return false;
       sendOk = (write(header.latin1(), header.length()) == (ssize_t) header.length());
    }
    if (!sendOk)
    {
       kdDebug(7103) << "http_open: sendOk==false. Connnection broken ! " << endl;
       error( ERR_CONNECTION_BROKEN, m_state.hostname );
       return false;
    }
  }

  bool res = true;

  if (moreData)
     res = sendBody();

  infoMessage( i18n( "<b>%1</b> contacted. Waiting for reply..." ).arg( m_request.hostname ) );

  return res;
#endif
}


/**
 * This function will read in the return header from the server.  It will
 * not read in the body of the return message.  It will also not transmit
 * the header to our client as the client doesn't need to know the gory
 * details of HTTP headers.
 */
bool HTTPProtocol::readHeader()
{
  // Check
  if (m_bCachedRead)
  {
     // Read header from cache...
     char buffer[4097];
     if (!fgets(buffer, 4096, m_fcache) )
     {
        // Error, delete cache entry
        kdDebug(7103) << "readHeader: Connnection broken ! " << endl;
        error( ERR_CONNECTION_BROKEN, m_state.hostname );
        return false;
     }
     kdDebug(7103) << "readHeader: returning mimetype " << buffer << endl;
     m_strMimeType = QString::fromUtf8( buffer).stripWhiteSpace();
     mimeType(m_strMimeType);
     if (!fgets(buffer, 4096, m_fcache) )
     {
        // Error, delete cache entry
        kdDebug(7103) << "readHeader(2): Connnection broken ! " << endl;
        error( ERR_CONNECTION_BROKEN, m_state.hostname );
        return false;
     }
     m_strCharset = QString::fromUtf8( buffer).stripWhiteSpace().lower();
     setMetaData("charset", m_strCharset);
     return true;
  }

  m_etag = QString::null;
  m_lastModified = QString::null;
  m_strCharset = QString::null;

  time_t dateHeader = 0;
  time_t expireDate = 0; // 0 = no info, 1 = already expired, > 1 = actual date
  int currentAge = 0;
  int maxAge = -1; // -1 = no max age, 0 already expired, > 0 = actual time

  QCString locationStr; // In case we get a redirect.
  QCString cookieStr; // In case we get a cookie.
  QString disposition; // Incase we get a Content-Disposition

  // read in 4096 bytes at a time (HTTP cookies can be quite large.)
  int len = 0;
  char buffer[4097];
  bool cont = false;
  bool cacheValidated = false; // Revalidation was successfull
  bool mayCache = true;
  bool hasCacheDirective = false;

  if (!waitForHeader(m_sock, m_remoteRespTimeout))
  {
     // No response error
     error( ERR_SERVER_TIMEOUT , m_state.hostname );
     return false;
  }

  gets(buffer, sizeof(buffer)-1);
  if (eof())
  {
     kdDebug(7103) << "readHeader: EOF while waiting for header start." << endl;
     if (m_bKeepAlive) // Try to reestablish connection.
     {
        http_closeConnection();
        if ( !http_open() )
           return false;
        if (!waitForHeader(m_sock, m_remoteRespTimeout))
        {
           // No response error
           error( ERR_SERVER_TIMEOUT, m_state.hostname );
           return false;
        }
        gets(buffer, sizeof(buffer)-1);
     }
     if (eof())
     {
        if (m_request.method == HTTP_HEAD)
        {
           // HACK
           // Some web-servers fail to respond properly to a HEAD request.
           // We compensate for their failure to properly implement the HTTP standard
           // by assuming that they will be sending html.
           mimeType(QString::fromLatin1(DEFAULT_MIME_TYPE));
           return true;
        }
        kdDebug(7103) << "readHeader(3): Connnection broken ! " << endl;
        error( ERR_CONNECTION_BROKEN, m_state.hostname );
        return false;
     }
  }

  do {
    // strip off \r and \n if we have them
    len = strlen(buffer);

    while(len && (buffer[len-1] == '\n' || buffer[len-1] == '\r'))
      buffer[--len] = 0;

    // if there was only a newline then continue
    if (!len)
    {
      kdDebug(7113) << "Got header (" << getpid() << "): --empty--" << endl;
      continue;
    }

    kdDebug(7113) << "Got header (" << getpid() << "): \"" << buffer << "\"" << endl;

    // are we allowd to resume?  this will tell us
    if (strncasecmp(buffer, "Accept-Ranges:", 14) == 0) {
      if (strncasecmp(trimLead(buffer + 14), "none", 4) == 0)
            m_bCanResume = false;
    }
    else if (strncasecmp(buffer, "Cache-Control:", 14) == 0) {
      QStringList cacheControls = QStringList::split(',',
                                     QString::fromLatin1(trimLead(buffer+14)));
      for(QStringList::ConstIterator it = cacheControls.begin();
          it != cacheControls.end();
          it++)
      {
         QString cacheControl = (*it).stripWhiteSpace();
         if (strncasecmp(cacheControl.latin1(), "no-cache", 8) == 0)
         {
            m_bCachedWrite = false; // Don't put in cache
            mayCache = false;
         }
         else if (strncasecmp(cacheControl.latin1(), "no-store", 8) == 0)
         {
            m_bCachedWrite = false; // Don't put in cache
            mayCache = false;
         }
         else if (strncasecmp(cacheControl.latin1(), "max-age=", 8) == 0)
         {
            maxAge = atol(cacheControl.mid(8).stripWhiteSpace().latin1());
         }
      }
      hasCacheDirective = true;
    }

    // get the size of our data
    else if (strncasecmp(buffer, "Content-length:", 15) == 0) {
      m_iSize = atol(trimLead(buffer + 15));
    }

    // what type of data do we have?
    else if (strncasecmp(buffer, "Content-type:", 13) == 0) {
       m_strMimeType = QString::fromLatin1(trimLead(buffer + 13)).stripWhiteSpace().lower();
       int semicolonPos = m_strMimeType.find( ';' );
       if ( semicolonPos != -1 )
       {
         int pos = semicolonPos;
         while ( m_strMimeType[++pos] == ' ' );
         if ( m_strMimeType.find("charset", pos, false) == pos )
         {
           pos+=7;
           while( m_strMimeType[pos] == ' ' || m_strMimeType[pos] == '=' ) pos++;
           m_strCharset = m_strMimeType.mid( pos );
           //kdDebug(7103) << "Found charset: " << m_strCharset << endl;
         }
         m_strMimeType.truncate( semicolonPos );
       }
       //kdDebug(7103) << "Content-type: " << m_strMimeType << endl;
    }
    //
    else if (strncasecmp(buffer, "Date:", 5) == 0) {
      dateHeader = KRFCDate::parseDate(trimLead(buffer+5));
    }

    // Cache management
    else if (strncasecmp(buffer, "ETag:", 5) == 0) {
      m_etag = trimLead(buffer+5);
    }

    // Cache management
    else if (strncasecmp(buffer, "Expires:", 8) == 0) {
      expireDate = KRFCDate::parseDate(trimLead(buffer+8));
      if (!expireDate)
        expireDate = 1; // Already expired
    }

    // Cache management
    else if (strncasecmp(buffer, "Last-Modified:", 14) == 0) {
      m_lastModified = (QString::fromLatin1(trimLead(buffer+14))).stripWhiteSpace();
    }

    // whoops.. we received a warning
    else if (strncasecmp(buffer, "Warning:", 8) == 0) {
      //Don't use warning() here, no need to bother the user.
      //Those warnings are mostly about caches.
      infoMessage(trimLead(buffer + 8));
    }

    // Cache management (HTTP 1.0)
    else if (strncasecmp(buffer, "Pragma:", 7) == 0) {
      QCString pragma = QCString(trimLead(buffer+7)).stripWhiteSpace().lower();
      if (pragma == "no-cache")
      {
         m_bCachedWrite = false; // Don't put in cache
         mayCache = false;
      }
      hasCacheDirective = true;
    }
    // The deprecated Refresh Response
    else if (strncasecmp(buffer,"Refresh:", 8) == 0) {
      kdDebug(7113) << buffer << endl;
      mayCache = false;  // Do not cache page as it defeats purpose of Refresh tag!
      setMetaData( "http-refresh", QString::fromLatin1(trimLead(buffer+8)).stripWhiteSpace() );
    }
    // We got the header
    else if (strncasecmp(buffer, "HTTP/", 5) == 0) {
      if (strncmp(buffer+5, "1.0 ",4) == 0)
      {
         m_HTTPrev = HTTP_10;
         m_bKeepAlive = false;
      }
      else // Assume everything else to be 1.1 or higher....
      {
         m_HTTPrev = HTTP_11;
         //Authentication = AUTH_None;  Do not do this here!! it is reset before each request!!!
         // Connections with proxies are closed by default because
         // some proxies like junkbuster can't handle persistent
         // connections but don't tell us.
         // We will still use persistent connections if the proxy
         // sends us a "Connection: Keep-Alive" header.
         if (m_state.do_proxy)
         {
            m_bKeepAlive = false;
         }
         else
         {
#ifdef DO_SSL
            // Don't do persistant connections with SSL.
            if (!m_bUseSSL)
               m_bKeepAlive = true; // HTTP 1.1 has persistant connections.
#else
               m_bKeepAlive = true; // HTTP 1.1 has persistant connections by default.
#endif
         }
      }

      if ( m_responseCode )
        m_prevResponseCode = m_responseCode;

      m_responseCode = atoi(buffer+9);

      // server side errors
      if (m_responseCode >= 500 && m_responseCode <= 599) {
        if (m_request.method == HTTP_HEAD) {
           // Ignore error
        } else {
           if (m_bErrorPage)
              errorPage();
           else
           {
              error(ERR_INTERNAL_SERVER, m_request.url.url());
              return false;
           }
        }
        m_bCachedWrite = false; // Don't put in cache
        mayCache = false;
      }
      // Unauthorized access
      else if (m_responseCode == 401 || m_responseCode == 407) {
        // Double authorization requests, i.e. a proxy auth
        // request followed immediately by a regular auth request.
        if ( m_prevResponseCode != m_responseCode &&
            (m_prevResponseCode == 401 || m_prevResponseCode == 407) )
          saveAuthorization();

        m_bUnauthorized = true;
        m_bCachedWrite = false; // Don't put in cache
        mayCache = false;
      }
      // Any other client errors
      else if (m_responseCode >= 400 && m_responseCode <= 499) {
        // Tell that we will only get an error page here.
        if (m_bErrorPage)
           errorPage();
        else
        {
           error(ERR_DOES_NOT_EXIST, m_request.url.url());
           return false;
        }
        m_bCachedWrite = false; // Don't put in cache
        mayCache = false;
      }
      else if (m_responseCode == 307)
      {
        // 307 Temporary Redirect
        m_bCachedWrite = false; // Don't put in cache
        mayCache = false;
      }
      else if (m_responseCode == 304)
      {
        // 304 Not Modified
        // The value in our cache is still valid.
        cacheValidated = true;
      }
      else if (m_responseCode >= 301 && m_responseCode<= 303)
      {
        // 301 Moved permanently
        // 302 Found (temporary location)
        // 303 See Other
        if (m_request.method != HTTP_HEAD && m_request.method != HTTP_GET)
        {
           // NOTE: This is wrong according to RFC 2616.  However,
           // because most other existing user agent implementations
           // treat a 301/302 response as a 303 response and preform
           // a GET action regardless of what the previous method was,
           // many servers have simply adapted to this way of doing
           // things!!  Thus, we are forced to do the same thing or we
           // won't be able to retrieve these pages correctly!!  This
           // implementation is therefore only correct for a 303 response
           // according to RFC 2616 section 10.3.2/3/4/8
           m_request.method = HTTP_GET; // Force a GET
        }
        m_bCachedWrite = false; // Don't put in cache
        mayCache = false;
      }
      else if ( m_responseCode == 204 ) // No content
      {
        error(ERR_NO_CONTENT, i18n("Data has been successfully sent."));
        return false;
      }
      else if ( m_responseCode == 206 )
      {
        if ( m_request.offset )
          m_bCanResume = true;
      }
      else if (m_responseCode == 100)
      {
        // We got 'Continue' - ignore it
        cont = true;
      }
    }
    // In fact we should do redirection only if we got redirection code
    else if (strncasecmp(buffer, "Location:", 9) == 0 ) {
      // Redirect only for 3xx status code, will ya! Thanks, pal!
      if ( m_responseCode > 299 && m_responseCode < 400 )
        locationStr = QCString(trimLead(buffer+9)).stripWhiteSpace();
    }

    // Check for cookies
    else if (strncasecmp(buffer, "Set-Cookie", 10) == 0) {
      cookieStr += buffer;
      cookieStr += '\n';
    }

    // check for direct authentication
    else if (strncasecmp(buffer, "WWW-Authenticate:", 17) == 0) {
      configAuth(trimLead(buffer + 17), false);
    }

    // check for proxy-based authentication
    else if (strncasecmp(buffer, "Proxy-Authenticate:", 19) == 0) {
        configAuth(trimLead(buffer + 19), true);
    }

    // content?
    else if (strncasecmp(buffer, "Content-Encoding:", 17) == 0) {
      // This is so wrong !!  No wonder kio_http is stripping the
      // gzip encoding from downloaded files.  This solves multiple
      // bug reports and caitoo's problem with downloads when such a
      // header is encountered...

      // A quote from RFC 2616:
      // " When present, its (Content-Encoding) value indicates what additional
      // content have been applied to the entity body, and thus what decoding
      // mechanism must be applied to obtain the media-type referenced by the
      // Content-Type header field.  Content-Encoding is primarily used to allow
      // a document to be compressed without loosing the identity of its underlying
      // media type.  Simply put if it is specified, this is the actual mime-type
      // we should use when we pull the resource !!!
      addEncoding(trimLead(buffer + 17), m_qContentEncodings);
    }

    // continue only if we know that we're HTTP/1.1
    else if (m_HTTPrev == HTTP_11) {
      // let them tell us if we should stay alive or not
      if (strncasecmp(buffer, "Connection:", 11) == 0) {
        if (strncasecmp(trimLead(buffer + 11), "Close", 5) == 0) {
          m_bKeepAlive = false;
        } else if (strncasecmp(trimLead(buffer + 11), "Keep-Alive", 10)==0) {
#ifdef DO_SSL
          // Don't do persistant connections with SSL.
          if (!m_bUseSSL)
             m_bKeepAlive = true;
#else
          m_bKeepAlive = true;
#endif
        }

      }

      // what kind of encoding do we have?  transfer?
      else if (strncasecmp(buffer, "Transfer-Encoding:", 18) == 0) {
        // If multiple encodings have been applied to an entity, the
        // transfer-codings MUST be listed in the order in which they
        // were applied.
        addEncoding(trimLead(buffer + 18), m_qTransferEncodings);
      }

      // md5 signature
      else if (strncasecmp(buffer, "Content-MD5:", 12) == 0) {
        m_sContentMD5 = strdup(trimLead(buffer + 12));
      }
      // Refer to RFC 2616 sec 15.5/19.5.1 and RFC 2183
      else if(strncasecmp(buffer, "Content-Disposition:", 20) == 0) {
        disposition = trimLead(buffer + 20);
        int pos = disposition.find( ';' );
        if ( pos != -1 )
        {
          if( disposition.find( QString::fromLatin1("attachment"), 0, false ) == 0 )
            disposition = disposition.mid(pos+1).stripWhiteSpace();
        }
        if ( disposition.find( QString::fromLatin1("filename"), 0, false) == 0 )
        {
          pos = 8;
          int len = disposition.length();
          while( disposition[pos] == ' ' || disposition[pos] == '=' ||
                 disposition[pos] == '"' )
              pos++;
          if( pos < len )
          {
            int start = pos;
            while ( pos < len &&
                   (disposition[pos] != '"' || disposition[pos] != ';') )
                pos++;
            disposition = disposition.mid(start, pos);
          }
          else
            disposition = QString::null;
        }
        else
          disposition = QString::null;

        // Content-Dispostion is not allowed to dictate directory
        // path, thus we extract the filename only.
        pos = disposition.findRev( '/' );
        if( pos > -1 )
          disposition = disposition.mid(pos+1);
        kdDebug(7113) << "Content-Disposition: " << disposition << endl;
      }
    }

    // Clear out our buffer for further use.
    memset(buffer, 0, sizeof(buffer));
  } while (len && (gets(buffer, sizeof(buffer)-1)));

  // If we do not support the requested authentication method...
  if ( (m_responseCode == 401 && Authentication == AUTH_None) ||
       (m_responseCode == 407 && ProxyAuthentication == AUTH_None) )
  {
    error( ERR_UNSUPPORTED_ACTION, "Unknown Authorization method!" );
    return false;
  }

  // Fixup expire date for clock drift.
  if (expireDate && (expireDate <= dateHeader))
     expireDate = 1; // Already expired.

  // Convert max-age into expireDate (overriding previous set expireDate)
  if (maxAge == 0)
     expireDate = 1; // Already expired.
  else if (maxAge > 0)
  {
     if (currentAge)
        maxAge -= currentAge;
     if (maxAge <=0)
        maxAge = 0;
     expireDate = time(0) + maxAge;
  }

  if (!expireDate)
     expireDate = time(0) + DEFAULT_EXPIRE;

  // DONE receiving the header!
  if (!cookieStr.isEmpty())
  {
     if ((m_cookieMode == CookiesAuto) && m_bUseCookiejar)
     {
        // Give cookies to the cookiejar.
        addCookies( m_request.url.url(), cookieStr );
     }
     else if (m_cookieMode == CookiesManual)
     {
        // Pass cookie to application
        setMetaData("setcookies", cookieStr);
     }
  }

  if (m_bMustRevalidate)
  {
     m_bMustRevalidate = false; // Reset just in case.
     if (cacheValidated)
     {
       // Yippie, we can use the cached version.
       // Update the cache with new "Expire" headers.
       fclose(m_fcache);
       m_fcache = 0;
       updateExpireDate( expireDate, true );
       m_fcache = checkCacheEntry( ); // Re-read cache entry

       if (m_fcache)
       {
          m_bCachedRead = true;
          return readHeader(); // Read header again, but now from cache.
       }
       else
       {
          // Where did our cache entry go???
       }
     }
     else
     {
       // Validation failed. Close cache.
       fclose(m_fcache);
       m_fcache = 0;
     }
  }

  // We need to reread the header if we got a '100 Continue'
  if ( cont )
  {
    return readHeader();
  }

  // We need to try to login again if we failed earlier
  if ( m_bUnauthorized )
  {
    if ( m_responseCode == 401 || m_responseCode == 407 )
    {
        if ( getAuthorization() )
        {
           http_closeConnection();
           return false; // Try again.
        }

        if (m_bError)
           return false; // Error out

        // Show error page...
    }
    m_bUnauthorized = false;
  }

  // We need to do a redirect
  if (!locationStr.isEmpty())
  {
    KURL u(m_request.url, locationStr);
    if(u.isMalformed() || u.isLocalFile() )
    {
      error(ERR_MALFORMED_URL, u.url());
      return false;
    }

    kdDebug(7113) << "request.url: " << m_request.url.url() << endl
                  << "LocationStr: " << locationStr.data() << endl;
    kdDebug(7113) << "Requesting redirection to: " << u.url() << endl;

    redirection(u.url());
    m_bCachedWrite = false; // Turn off caching on re-direction (DA)
    mayCache = false;
  }

  // Inform the job that we can indeed resume...
  if ( m_bCanResume && m_request.offset )
    canResume();

  // Reset the POST buffer if we do not get an authorization
  // request and the previous action was POST.
  if ( m_request.method==HTTP_POST && !m_bUnauthorized )
    m_bufPOST.resize(0);

  // Do not cache pages originating from password
  // protected sites.
  if ( !hasCacheDirective && Authentication != AUTH_None )
  {
    m_bCachedWrite = false;
    mayCache = false;
  }

  // WABA: Correct for tgz files with a gzip-encoding.
  // They really shouldn't put gzip in the Content-Encoding field!
  // Web-servers really shouldn't do this: They let Content-Size refer
  // to the size of the tgz file, not to the size of the tar file,
  // while the Content-Type refers to "tar" instead of "tgz".
  if (m_qContentEncodings.last() == "gzip")
  {
     if (m_strMimeType == "application/x-tar")
     {
        m_qContentEncodings.remove(m_qContentEncodings.fromLast());
        m_strMimeType = QString::fromLatin1("application/x-tgz");
     }
     else if ((m_strMimeType.startsWith("text/")) &&
              (m_request.url.path().right(3) != ".gz"))
     {
        // Unzip!
     }
     else
     {
        m_qContentEncodings.remove(m_qContentEncodings.fromLast());
        m_strMimeType = QString::fromLatin1("application/x-gzip");
     }
  }

  // We can't handle "bzip2" encoding (yet). So if we get something with
  // bzip2 encoding, we change the mimetype to "application/x-bzip2".
  // Note for future changes: some web-servers send both "bzip2" as
  //   encoding and "application/x-bzip2" as mimetype. That is wrong.
  //   currently that doesn't bother us, because we remove the encoding
  //   and set the mimetype to x-bzip2 anyway.
  if (m_qContentEncodings.last() == "bzip2")
  {
     m_qContentEncodings.remove(m_qContentEncodings.fromLast());
     m_strMimeType = QString::fromLatin1("application/x-bzip2");
  }

  // Convert some common mimetypes to standard KDE mimetypes
  if (m_strMimeType == "application/x-targz")
     m_strMimeType = QString::fromLatin1("application/x-tgz");
  else if (m_strMimeType == "image/x-png")
     m_strMimeType = QString::fromLatin1("image/png");
  else if (m_strMimeType == "audio/mpeg" || m_strMimeType == "audio/x-mpeg" || m_strMimeType == "audio/mp3")
     m_strMimeType = QString::fromLatin1("audio/x-mp3");
  else if (m_strMimeType == "audio/midi")
     m_strMimeType = QString::fromLatin1("audio/x-midi");

  // Prefer application/x-tgz over application/x-gzip
  else if (m_strMimeType == "application/x-gzip")
  {
     if ((m_request.url.path().right(7) == ".tar.gz") ||
         (m_request.url.path().right(4) == ".tar"))
        m_strMimeType = QString::fromLatin1("application/x-tgz");
  }

  // Some webservers say "text/plain" when they mean "application/x-bzip2"
  else if (m_strMimeType == "text/plain")
  {
     if (m_request.url.path().right(4) == ".bz2")
        m_strMimeType = QString::fromLatin1("application/x-bzip2");
  }
#if 0
  // Even if we can't rely on content-length, it seems that we should
  // never get more data than content-length. Maybe less, if the
  // content-length refers to the unzipped data.
  if (!m_qContentEncodings.isEmpty())
  {
     // If we still have content encoding we can't rely on the Content-Length.
     m_iSize = -1;
  }
#endif

  // Set charset. Maybe charSet should be a class member, since
  // this method is somewhat recursive....
  if ( !m_strCharset.isEmpty() )
  {
     kdDebug(7103) << "Setting charset metadata to: " << m_strCharset << endl;
     setMetaData("charset", m_strCharset);
  }

  if( !disposition.isEmpty() )
  {
     kdDebug(7103) << "Setting Content-Disposition metadata to: " << disposition << endl;
     setMetaData("content-disposition", disposition);
  }

  // Let the app know about the mime-type iff this is not
  // a redirection and the mime-type string is not empty.
  if( locationStr.isEmpty() && (!m_strMimeType.isEmpty() ||
      m_request.method == HTTP_HEAD) &&
      m_request.url.filename() != "favicon.ico" )
  {
     kdDebug(7103) << "Emitting mimetype " << m_strMimeType << endl;
     mimeType( m_strMimeType );
  }

  if (m_request.method == HTTP_HEAD)
     return true;

  if (!m_lastModified.isEmpty())
     setMetaData("modified", m_lastModified);

  if (!mayCache)
     setMetaData("no-cache", "true");

  // Do we want to cache this request?
  if ( m_bCachedWrite && !m_strMimeType.isEmpty() )
  {
     // Check...
     createCacheEntry(m_strMimeType, expireDate); // Create a cache entry
     if (!m_fcache)
        m_bCachedWrite = false; // Error creating cache entry.
     m_expireDate = expireDate;
  }

  if (m_bCachedWrite && !m_strMimeType.isEmpty())
    kdDebug(7113) << "Cache, adding \"" << m_request.url.url() << "\"" << endl;
  else if (m_bCachedWrite && m_strMimeType.isEmpty())
    kdDebug(7113) << "Cache, pending \"" << m_request.url.url() << "\"" << endl;
  else
    kdDebug(7113) << "Cache, not adding \"" << m_request.url.url() << "\"" << endl;
  return true;
}


void HTTPProtocol::addEncoding(QString encoding, QStringList &encs)
{
  encoding = encoding.stripWhiteSpace().lower();
  // Identity is the same as no encoding
  if (encoding == "identity") {
    return;
  } else if (encoding == "8bit") {
    // Strange encoding returned by http://linac.ikp.physik.tu-darmstadt.de
    return;
  } else if (encoding == "chunked") {
    m_bChunked = true;
    // Anyone know of a better way to handle unknown sizes possibly/ideally with unsigned ints?
    //if ( m_cmd != CMD_COPY )
      m_iSize = -1;
  } else if ((encoding == "x-gzip") || (encoding == "gzip")) {
    encs.append(QString::fromLatin1("gzip"));
  } else if ((encoding == "x-bzip2") || (encoding == "bzip2")) {
    encs.append(QString::fromLatin1("bzip2")); // Not yet supported!
  } else if ((encoding == "x-deflate") || (encoding == "deflate")) {
    encs.append(QString::fromLatin1("deflate"));
  } else {
    kdWarning(7103) << "Unknown encoding encountered.  Please write code. Pid = " << getpid() << " Encoding = \"" << encoding << "\"" << endl;
  }
}

void HTTPProtocol::configAuth( const char *p, bool b )
{
  HTTP_AUTH f = AUTH_None;
  const char *strAuth = p;

  while( *p == ' ' ) p++;
  if ( strncasecmp( p, "Basic", 5 ) == 0 )
  {
    f = AUTH_Basic;
    p += 5;
  }
  else if (strncasecmp (p, "Digest", 6) ==0 )
  {
    f = AUTH_Digest;
    p += 6;
    strAuth = p;
  }
  else if (strncasecmp( p, "MBS_PWD_COOKIE", 14 ) == 0)
  {
    // Found on http://www.webscription.net/baen/default.asp
    f = AUTH_Basic;
    p += 14;
  }
  else
  {
    kdWarning(7103) << "Unsupported or invalid authorization type requested" << endl;
    kdWarning(7103) << "Request Authorization: " << p << endl;
  }

  /*
     This check ensures the following:
     1.) Rejection of any unknown/unsupported authentication schemes
     2.) Useage of the strongest possible authentication schemes if
         and when multiple Proxy-Authenticate or WWW-Authenticate
         header field is sent.
  */
  if ( f == AUTH_None ||
       (b && m_iProxyAuthCount > 0 && f < ProxyAuthentication) ||
       (!b && m_iWWWAuthCount > 0 && f < Authentication) )
  {
    // Since I purposefully made the Proxy-Authentication settings
    // persistent to reduce the number of round-trips to kdesud we
    // have to take special care when an unknown/unsupported auth-
    // scheme is received. This check accomplishes just that...
    if ( b )
    {
      if ( !m_iProxyAuthCount )
        ProxyAuthentication = f;
      m_iProxyAuthCount++;
    }
    else
      m_iWWWAuthCount++;
    return;
  }

  while (*p)
  {
    int i = 0;
    while( (*p == ' ') || (*p == ',') || (*p == '\t') ) { p++; }
    if ( strncasecmp( p, "realm=\"", 7 ) == 0 )
    {
      p += 7;
      while( p[i] != '"' ) i++;
      if( b )
        m_strProxyRealm = QString::fromLatin1( p, i );
      else
        m_strRealm = QString::fromLatin1( p, i );
    }
    p+=(i+1);
  }

  if( b )
  {
    ProxyAuthentication = f;
    m_strProxyAuthorization = QString::fromLatin1( strAuth, strlen(strAuth) );
  }
  else
  {
    Authentication = f;
    m_strAuthorization = QString::fromLatin1( strAuth, strlen(strAuth) );
  }
}

bool HTTPProtocol::sendBody()
{
  int result=-1;
  int length=0;

  // Loop until we got 'dataEnd'
  kdDebug(7113) << "Response code: " << m_responseCode << endl;
  if ( m_responseCode == 401 || m_responseCode == 407 )
  {
    // For RE-POST on authentication failure the
    // buffer should not be empty...
    if ( m_bufPOST.isNull() )
    {
      error( ERR_ABORTED, m_request.hostname );
      return false;
    }
    kdDebug(7113) << "POST'ing saved data..." << endl;
    length = m_bufPOST.size();
    result = 0;
  }
  else
  {
    kdDebug(7113) << "POST'ing live data..." << endl;
    m_bufPOST.resize(0);
    QByteArray buffer;
    int old_size;
    do
    {
      dataReq(); // Request for data
      result = readData( buffer );
      if ( result > 0 )
      {
        kdDebug(7113) << "POST data read: " << QString(buffer) << endl;
        length += result;
        old_size = m_bufPOST.size();
        m_bufPOST.resize( old_size+result );
        memcpy( m_bufPOST.data()+ old_size, buffer.data(), buffer.size() );
        buffer.resize(0);
      }
    } while ( result > 0 );
  }

  if ( result != 0 )
  {
    error( ERR_ABORTED, m_request.hostname );
    return false;
  }

  char c_buffer[64];
  sprintf(c_buffer, "Content-Length: %d\r\n\r\n", length);
  kdDebug( 7113 ) << c_buffer << endl;

  // Debugging code...
  kdDebug( 7113 ) << "POST'ing Data: " << QString(m_bufPOST ) << endl;

  // Send the content length...
  bool sendOk = (write(c_buffer, strlen(c_buffer)) == (ssize_t) strlen(c_buffer));
  if (!sendOk)
  {
    kdDebug(7103) << "Connection broken (sendBody(1))! (" << m_state.hostname << ")" << endl;
    error( ERR_CONNECTION_BROKEN, m_state.hostname );
    return false;
  }

  // Send the data...
  sendOk = (write(m_bufPOST.data(), m_bufPOST.size()) == (ssize_t) m_bufPOST.size());
  if (!sendOk)
  {
    kdDebug(7103) << "Connection broken (sendBody(2))! (" << m_state.hostname << ")" << endl;
    error( ERR_CONNECTION_BROKEN, m_state.hostname );
    return false;
  }
  return true;
}

void HTTPProtocol::http_close()
{
  if (m_fcache)
  {
     fclose(m_fcache);
     m_fcache = 0;
     if (m_bCachedWrite)
     {
        QString filename = m_state.cef + ".new";
        unlink( filename.latin1() );
     }
  }
  if (!m_bKeepAlive)
     http_closeConnection();
  else
     kdDebug(7113) << "(" << getpid() << ") http_close: keep alive" << endl;
}

void HTTPProtocol::closeConnection()
{
  http_closeConnection();
}

void HTTPProtocol::http_closeConnection()
{
  kdDebug(7113) << "http_closeConnection: closing (" << getpid() << ")" << endl;
  m_bKeepAlive = false; // Just in case.
  if ( m_sock )
    ::close( m_sock );
  m_sock = 0;
#ifdef DO_SSL
  closeSSL();
#endif
}

void HTTPProtocol::setHost(const QString& host, int port, const QString& user, const QString& pass)
{
  kdDebug(7113) << "Hostname is now: " << host << endl;
  m_request.hostname = host;

  // try to ensure that the port is something reasonable
  if ( port == 0 )
     port = m_DefaultPort;

  m_proxyURL = metaData("UseProxy");
  kdDebug(7113) << "Proxy URL is now: " << m_proxyURL.url() << endl;
  m_bUseProxy = m_proxyURL.isValid();
  m_request.port = port;
  m_request.user = user;
  m_request.passwd = pass;
  m_request.do_proxy = m_bUseProxy;

  m_bUseCache = config()->readBoolEntry("UseCache", true);
  m_strCacheDir = config()->readEntry("CacheDir");
  if (m_strCacheDir.isEmpty())
     m_strCacheDir = KGlobal::dirs()->saveLocation("data", "kio_http/cache");
  m_maxCacheAge = config()->readNumEntry("MaxCacheAge", DEFAULT_MAX_CACHE_AGE);

  kdDebug(7103) << "UseCache: " << metaData("UseCache") << endl;
  kdDebug(7103) << "CacheDir: " << metaData("CacheDir") << endl;
  kdDebug(7103) << "MaxCacheAge: " << metaData("MaxCacheAge") << endl;

  // Obtain the proxy and remote server timeout values
  m_proxyConnTimeout = proxyConnectTimeout();
  m_remoteConnTimeout = connectTimeout();
  m_remoteRespTimeout = responseTimeout();

  kdDebug(7103) << "Timeout proxy = " << m_proxyConnTimeout <<
                   " connection = " << m_remoteConnTimeout <<
                   " response = " << m_remoteRespTimeout << endl;
}

void HTTPProtocol::slave_status()
{
  bool connected = http_isConnected();
  if ( m_sock && !connected )
     http_closeConnection();
  slaveStatus( m_state.hostname, connected );
}

void HTTPProtocol::buildURL()
{
  m_request.url.setProtocol( m_protocol );
  m_request.url.setUser( m_request.user );
  m_request.url.setPass( m_request.passwd );
  m_request.url.setHost( m_request.hostname );
  m_request.url.setPort( m_request.port );
  m_request.url.setPath( m_request.path );
  if (m_request.query.length())
    m_request.url.setQuery( m_request.query );
}

// We just return 'FILE' here.
void HTTPProtocol::stat(const KURL& url)
{
  if (m_request.hostname.isEmpty())
  {
     error( KIO::ERR_UNKNOWN_HOST, i18n("No host specified!"));
     return;
  }

  kdDebug(7113) << "HTTPProtocol::stat " << url.prettyURL() << endl;

  UDSEntry entry;
  UDSAtom atom;
  atom.m_uds = KIO::UDS_NAME;
  atom.m_str = url.fileName();
  entry.append( atom );

  atom.m_uds = KIO::UDS_FILE_TYPE;
  atom.m_long = S_IFREG; // a file
  entry.append( atom );

  atom.m_uds = KIO::UDS_ACCESS;
  atom.m_long = S_IRUSR | S_IRGRP | S_IROTH; // readable by everybody
  entry.append( atom );

  statEntry( entry );

  finished();
}

void HTTPProtocol::get( const KURL& url )
{
  if ( !checkRequestURL( url ) )
    return;

  kdDebug(7113) << "HTTPProtocol::get " << url.url() << endl;

  m_request.method = HTTP_GET;
  m_request.path = url.path();
  m_request.query = url.query();
  QString tmp = metaData("cache");
  if (!tmp.isEmpty())   
    m_request.cache = parseCacheControl(tmp);
  else
    m_request.cache = DEFAULT_CACHE_CONTROL;

  m_request.offset = 0;
  m_request.do_proxy = m_bUseProxy;
  m_request.url = url;
  m_request.passwd = url.pass();
  m_request.user = url.user();

  retrieveContent( true /* SSL checks */ );
}

void HTTPProtocol::put( const KURL &url, int, bool, bool)
{
  if ( !checkRequestURL( url ) )
    return;

  kdDebug(7113) << "HTTPProtocol::put " << url.prettyURL() << endl;

  m_request.method = HTTP_PUT;
  m_request.path = url.path();
  m_request.query = QString::null;
  m_request.cache = CC_Reload;
  m_request.offset = 0;
  m_request.do_proxy = m_bUseProxy;
  m_request.url = url;

  retrieveContent();

}

void HTTPProtocol::post( const KURL& url)
{
  if ( !checkRequestURL( url ) )
    return;

  kdDebug(7113) << "HTTPProtocol::post " << url.prettyURL() << endl;

  m_request.method = HTTP_POST;
  m_request.path = url.path();
  m_request.query = url.query();
  m_request.cache = CC_Reload;
  m_request.offset = 0;
  m_request.do_proxy = m_bUseProxy;
  m_request.url = url;

  retrieveContent();

}

void HTTPProtocol::cache_update( const KURL& url, bool no_cache, time_t expireDate)
{
  m_request.path = url.path();
  m_request.query = url.query();
  m_request.cache = CC_Reload;
  m_request.offset = 0;
  m_request.do_proxy = m_bUseProxy;
  m_request.url = url;

  if (no_cache)
  {
     m_fcache = checkCacheEntry( );
     if (m_fcache)
     {
       fclose(m_fcache);
       m_fcache = 0;
       ::unlink(m_state.cef.latin1());
     }
  }
  else
  {
     updateExpireDate( expireDate );
  }

  finished();
}

void HTTPProtocol::mimetype( const KURL& url )
{
  if ( !checkRequestURL( url ) )
    return;

  kdDebug(7113) << "HTTPProtocol::mimetype " << url.prettyURL() << endl;

  m_request.method = HTTP_HEAD;
  m_request.path = url.path();
  m_request.query = url.query();
  m_request.cache = CC_Cache;
  m_request.offset = 0;
  m_request.do_proxy = m_bUseProxy;
  m_request.url = url;

  retrieveHeader();

  kdDebug(7103) << "http: mimetype = " << m_strMimeType << endl;

}

void HTTPProtocol::special( const QByteArray &data)
{
  int tmp;
  QDataStream stream(data, IO_ReadOnly);

  stream >> tmp;
  switch (tmp) {
    case 1: // HTTP POST
    {
      KURL url;
      stream >> url;
      post( url );
      break;
    }
    case 2: // cache_update
    {
      KURL url;
      bool no_cache;
      time_t expireDate;
      stream >> url >> no_cache >> expireDate;
      cache_update( url, no_cache, expireDate );
      break;
    }
    default:
      assert(0);
  }

}

bool HTTPProtocol::checkRequestURL( const KURL& u )
{
  if (m_request.hostname.isEmpty())
  {
     error( KIO::ERR_UNKNOWN_HOST, i18n("No host specified!"));
     return false;
  }

  if ( m_protocol != u.protocol().latin1() )
  {
    short unsigned int oldDefaultPort = m_DefaultPort;
    m_protocol = u.protocol().latin1();
    reparseConfiguration();
    if ( m_DefaultPort != oldDefaultPort &&
         m_request.port == oldDefaultPort )
      m_request.port = m_DefaultPort;
  }

  return true;
}


void HTTPProtocol::decodeDeflate()
{
#ifdef DO_GZIP
  // Okay the code below can probably be replaced with
  // a single call to decompress(...) instead of a read/write
  // to and from a temporary file, but I was not sure of how to
  // estimate the size of the decompressed data which needs to be
  // passed as a parameter to decompress function call.  Anyone
  // want to try this approach or have a better experience with
  // the cool zlib library ??? (DA)
  //
  // TODO: Neither deflate nor gzip completely check for errors!!!
  z_stream z;
  QByteArray tmp_buf;
  const unsigned int max_len = 1024;
  unsigned char in_buf[max_len];  // next_in
  unsigned char out_buf[max_len]; // next_out
  int status = Z_OK; //status of the deflation
  char* filename=strdup("/tmp/kio_http.XXXXXX");

  z.avail_in = 0;
  z.avail_out = max_len;
  z.next_out = out_buf;

  // Create the file
  int fd = mkstemp(filename);

  // TODO: Need sanity check here. Doing no error checking when writing
  // to HD cannot be good :))  What if the target is bloody full or it
  // for some reason could not be written to ?? (DA)
  ::write(fd, big_buffer.data(), big_buffer.size()); // Write data into file
  lseek(fd, 0, SEEK_SET);
  FILE* fin = fdopen( fd, "rb" );

  // Read back and decompress data.
  for( ; ; )
  {
    if( z.avail_in == 0 )
    {
        z.next_in = in_buf;
        z.avail_in = ::fread(in_buf, 1, max_len, fin );
    }
    if( z.avail_in == 0 )
        break;
    status = inflate( &z, Z_NO_FLUSH );
    if( status !=  Z_OK )
        break;
    unsigned int count = max_len - z.avail_out;
    if( count )
    {
        unsigned int old_len = tmp_buf.size();
        memcpy( tmp_buf.data() + old_len, out_buf, count );
        z.next_out = out_buf;
        z.avail_out = max_len;
    }
  }

  for( ; ; )
  {
    status = inflate( &z, Z_FINISH );
    unsigned int count = max_len - z.avail_out;
    if ( count )
    {
        unsigned int old_len = tmp_buf.size();
        // Copy the data into a temporary buffer
        memcpy( tmp_buf.data() + old_len, out_buf, count );
        z.next_out = out_buf;
        z.avail_out = max_len;
    }
    if( status !=  Z_OK )
        break;
  }
  if( fin )
    ::fclose( fin );
  ::unlink(filename); // Bye bye to beloved temp file.  We will miss you!!

  // Replace big_buffer with the
  // "decoded" data.
  big_buffer.resize(0);
  big_buffer = tmp_buf;
  big_buffer.detach();
#endif
}

void HTTPProtocol::decodeGzip()
{
#ifdef DO_GZIP
  // Note I haven't found an implementation
  // that correctly handles this stuff.  Apache
  // sends both Transfer-Encoding and Content-Length
  // headers, which is a no-no because the content
  // could really be bigger than the content-length
  // header implies.. like with gzip.
  // eek. This is no fun for progress indicators.
  QByteArray ar;

  char tmp_buf[1024], *filename=strdup("/tmp/kio_http.XXXXXX");
  unsigned long len;
  int fd;


  // Siince I can't figure out how to do the mem to mem
  // gunzipping, this should suffice.  It just writes out
  // the gzip'd data to a file.
  fd=mkstemp(filename);
  ::write(fd, big_buffer.data(), big_buffer.size());
  lseek(fd, 0, SEEK_SET);
  gzFile gzf = gzdopen(fd, "rb");
  unlink(filename); // If you want to inspect the raw data, comment this line out

  // And then reads it back in with gzread so it'll
  // decompress on the fly.
  while ( (len=gzread(gzf, tmp_buf, 1024))>0){
    int old_len=ar.size();
    ar.resize(ar.size()+len);
    memcpy(ar.data()+old_len, tmp_buf, len);
  }
  gzclose(gzf);

  // And then we replace big_buffer with
  // the "decoded" data.
  big_buffer.resize(0);
  big_buffer=ar;
  big_buffer.detach();
#endif
}


size_t HTTPProtocol::sendData( )
{
  // This was rendered necesary b/c
  // the IPC stuff can't handle
  // chunks much larger than 2048.

  size_t sent=0;
  size_t bufferSize = MAX_IPC_SIZE;
  size_t sz = big_buffer.size();
  processedSize(sz);
  totalSize(sz);
  QByteArray array;
  while (sent+bufferSize < sz) {
    array.setRawData( big_buffer.data()+sent, bufferSize);
    data( array );
    array.resetRawData( big_buffer.data()+sent, bufferSize);
    sent+=bufferSize;
  }
  if (sent < sz)
  {
    array.setRawData( big_buffer.data()+sent, sz-sent);
    data( array );
    array.resetRawData( big_buffer.data()+sent, sz-sent);
  }

  if (m_bCachedWrite &&  m_fcache)
  {
     writeCacheEntry(big_buffer.data(), big_buffer.size());
     closeCacheEntry();
  }

  data( QByteArray() );
  return sz;
}

/**
 * Read a chunk from the data stream.
 */
int HTTPProtocol::readChunked()
{
  if (m_iBytesLeft <= 0)
  {
     m_bufReceive.resize(4096);

     if (!gets(m_bufReceive.data(), m_bufReceive.size()-1))
     {
       kdDebug(7103) << "gets() failure on Chunk header" << endl;
       return -1;
     }
     // We could have got the CRLF of the previous chunk.
     // If so, try again.
     if (m_bufReceive[0] == '\0')
     {
        if (!gets(m_bufReceive.data(), m_bufReceive.size()-1))
        {
           kdDebug(7103) << "gets() failure on Chunk header" << endl;
           return -1;
        }
     }
     if (eof())
     {
        kdDebug(7103) << "EOF on Chunk header" << endl;
        return -1;
     }

     m_iBytesLeft = strtol(m_bufReceive.data(), 0, 16);
     if (m_iBytesLeft < 0)
     {
        kdDebug(7103) << "Negative chunk size" << endl;
        return -1;
     }

     // kdDebug(7113) << "Chunk size = " << m_iBytesLeft << " bytes" << endl;

     if (m_iBytesLeft == 0)
     {
       // Last chunk.
       // Skip trailers.
       do {
         // Skip trailer of last chunk.
         if (!gets(m_bufReceive.data(), m_bufReceive.size()-1))
         {
           kdDebug(7103) << "gets() failure on Chunk trailer" << endl;
           return -1;
         }
         kdDebug(7113) << "Chunk trailer = \"" << m_bufReceive.data() << "\"" << endl;
       }
       while (strlen(m_bufReceive.data()) != 0);

       return 0;
     }
  }

  int bytesReceived = readLimited();
  if (!m_iBytesLeft)
     m_iBytesLeft = -1; // Don't stop, continue with next chunk
  return bytesReceived;
}

int HTTPProtocol::readLimited()
{
  m_bufReceive.resize(4096);

  int bytesReceived;
  int bytesToReceive;

  if (m_iBytesLeft > (int) m_bufReceive.size())
     bytesToReceive = m_bufReceive.size();
  else
     bytesToReceive = m_iBytesLeft;

  bytesReceived = read(m_bufReceive.data(), bytesToReceive);

  if (bytesReceived > 0)
  {
     m_iBytesLeft -= bytesReceived;
  }

  return bytesReceived;
}

int HTTPProtocol::readUnlimited()
{
  if (m_bKeepAlive)
  {
     kdWarning(7103) << "Unbounded datastream on a Keep Alive connection!" << endl;
     m_bKeepAlive = false;
  }
  m_bufReceive.resize(4096);

  return read(m_bufReceive.data(), m_bufReceive.size());
}

/**
 * This function is our "receive" function.  It is responsible for
 * downloading the message (not the header) from the HTTP server.  It
 * is called either as a response to a client's KIOJob::dataEnd()
 * (meaning that the client is done sending data) or by 'http_open()'
 * (if we are in the process of a PUT/POST request).
 */
bool HTTPProtocol::readBody( )
{
  QByteArray array;

  // Check if we need to decode the data.
  // If we are in copy mode, then use only transfer decoding.
  bool decode = !m_qTransferEncodings.isEmpty() || !m_qContentEncodings.isEmpty();
  bool useMD5 = !m_sContentMD5.isEmpty();

  // Get the starting time.  This is used
  // later to compute the transfer speed.
  time_t t_start = time(0L);
  time_t t_last = t_start;

  // Deal with the size of the file.

  long sz = m_request.offset;
  if ( sz ) { m_iSize += sz; }

  totalSize( (m_iSize > -1) ? m_iSize : 0 );
  infoMessage( i18n( "Retrieving data from <b>%1</b>" ).arg( m_request.hostname ) );

  if (m_bCachedRead)
  {
     char buffer[ MAX_IPC_SIZE ];
     // Jippie! It's already in the cache :-)
     while (!feof(m_fcache) && !ferror(m_fcache))
     {
        int nbytes = fread( buffer, 1, MAX_IPC_SIZE, m_fcache);
        if (nbytes > 0)
        {
          array.setRawData( buffer, nbytes);
          data( array );
          array.resetRawData( buffer, nbytes );
          sz += nbytes;
        }
     }
     processedSize( sz );
     // FINALLY, we compute our final speed and let everybody know that we
     // are done
     t_last = time(0L);
     if (t_last - t_start) {
       speed(sz / (t_last - t_start));
     } else {
       speed(0);
     }
     data( QByteArray() );
     return true;
  }

  if (m_iSize > -1)
    m_iBytesLeft = m_iSize - sz;
  else
    m_iBytesLeft = -1;

  if (m_bChunked)
    m_iBytesLeft = -1;

  kdDebug(7113) << "HTTPProtocol::readBody m_iBytesLeft=" << m_iBytesLeft << endl;

  // Main incoming loop...  Gather everything while we can...
  KMD5 context;
  bool cpMimeBuffer = false;
  QByteArray mimeTypeBuffer;
  big_buffer.resize(0);
  while (!eof())
  {
    int bytesReceived;
    if (m_bChunked)
       bytesReceived = readChunked();
    else if (m_iSize > -1)
       bytesReceived = readLimited();
    else
       bytesReceived = readUnlimited();

    // make sure that this wasn't an error, first
    if (bytesReceived == -1)
    {
      // erg.  oh well, log an error and bug out
      kdDebug(7103) << "readBody: bytesReceived==-1. Connnection broken ! " << endl;
      error(ERR_CONNECTION_BROKEN, m_state.hostname);
      return false;
    }

    // i guess that nbytes == 0 isn't an error.. but we certainly
    // won't work with it!
    if (bytesReceived > 0)
    {
      // If a broken server does not send the mime-type,
      // we try to id it from the content before dealing
      // with the content itself.
      if ( m_strMimeType.isEmpty() && !( m_responseCode >= 300 && m_responseCode <=399) )
      {
        kdDebug(7113) << "Attempting to determine mime-type from content..." << endl;
        int old_size = mimeTypeBuffer.size();
        mimeTypeBuffer.resize( old_size + bytesReceived );
        memcpy( mimeTypeBuffer.data() + old_size, m_bufReceive.data(), bytesReceived );
        if ( m_iBytesLeft > 0 && mimeTypeBuffer.size() < 1024 )
        {
          cpMimeBuffer = true;
          continue;   // Do not send up the data since we do not yet know its mimetype!
        }
        
	kdDebug(7113) << "Mimetype buffer size: " << mimeTypeBuffer.size() << endl;
        KMimeMagicResult * result = KMimeMagic::self()->findBufferFileType( mimeTypeBuffer, m_request.url.fileName() );
        if( result )
        {
          m_strMimeType = result->mimeType();
          kdDebug(7113) << "Mimetype from content: " << m_strMimeType << endl;
        }

        if ( m_strMimeType.isEmpty() )
        {
          m_strMimeType = QString::fromLatin1( DEFAULT_MIME_TYPE );  // if all else fails...
          kdDebug(7113) << "Using default mimetype:  " <<  m_strMimeType << endl;
        }

        if ( m_bCachedWrite )
        {
          createCacheEntry( m_strMimeType, m_expireDate );
          if (!m_fcache)
            m_bCachedWrite = false;
        }

        if ( cpMimeBuffer )
        {
          m_bufReceive.resize(0);
          m_bufReceive.resize(mimeTypeBuffer.size());
          memcpy( m_bufReceive.data(), mimeTypeBuffer.data(), mimeTypeBuffer.size() );
        }
        mimeType(m_strMimeType);
        mimeTypeBuffer.resize(0);
      }

      // check on the encoding.  can we get away with it as is?
      if ( !decode )
      {
        if (useMD5)
          context.update( m_bufReceive );

        // yep, let the world know that we have some data
        array.setRawData( m_bufReceive.data(), bytesReceived );
        data( array );
        array.resetRawData( m_bufReceive.data(), bytesReceived );

        if (m_bCachedWrite && m_fcache)
           writeCacheEntry(m_bufReceive.data(), bytesReceived);
        sz += bytesReceived;
        processedSize( sz );
        time_t t = time( 0L );
        if ( t - t_last >= 1 )
        {
          speed( (sz - m_request.offset) / ( t - t_start ) );
          t_last = t;
        }
      }
      else
      {
        // nope.  slap this all onto the end of a big buffer for later use
        // kdDebug( 7113 ) << "Further decoding needed..." << endl;
        unsigned int old_len = 0;
        old_len = big_buffer.size();
        big_buffer.resize(old_len + bytesReceived);
        memcpy(big_buffer.data() + old_len, m_bufReceive.data(), bytesReceived);
      }
    }
    if (m_iBytesLeft == 0)
      break;
  }

  m_bufReceive.resize(0);
  // if we have something in big_buffer, then we know that we have
  // encoded data.  of course, we need to do something about this
  if (!big_buffer.isNull())
  {
    // decode all of the transfer encodings
    while (!m_qTransferEncodings.isEmpty())
    {
      QString enc = m_qTransferEncodings.last();
      m_qTransferEncodings.remove(m_qTransferEncodings.fromLast());
      if ( enc == "gzip" )
        decodeGzip();
      else if ( enc == "deflate" )
        decodeDeflate();
    }

    // From HTTP 1.1 Draft 6:
    // The MD5 digest is computed based on the content of the entity-body,
    // including any content-coding that has been applied, but not including
    // any transfer-encoding applied to the message-body. If the message is
    // received with a transfer-encoding, that encoding MUST be removed
    // prior to checking the Content-MD5 value against the received entity.
    if ( useMD5 )
        context.update( big_buffer );

    // now decode all of the content encodings
    // -- Why ?? We are not
    // -- a proxy server, be a client side implementation!!  The applications
    // -- are capable of determinig how to extract the encoded implementation.
    // WB: That's a misunderstanding. We are free to remove the encoding.
    // WB: Some braindead www-servers however, give .tgz files an encoding
    // WB: of "gzip" (or even "x-gzip") and a content-type of "applications/tar"
    // WB: They shouldn't do that. We can work around that though...
    while (!m_qContentEncodings.isEmpty())
    {
      QString enc = m_qContentEncodings.last();
      m_qContentEncodings.remove(m_qContentEncodings.fromLast());
      if ( enc == "gzip" )
        decodeGzip();
      else if ( enc == "deflate" )
        decodeDeflate();

    }
    sz = sendData();
  }

  if ( useMD5 )
  {
    HASH digest;
    context.finalize();
    context.rawDigest(digest);
    QByteArray out, in;
    in.setRawData( digest, sizeof(digest) );
    KCodecs::base64Encode( in, out );
    if ( m_sContentMD5 == QString(out) )
      kdDebug(7103) << "MD5 checksum present and is match!!" << endl;
    else
      kdDebug(7103) << "MD5 checksum mismatch: got " << m_sContentMD5
                    << ", calculated " << QString(out) << endl;
    in.resetRawData( digest, sizeof(digest) );
  }

  // Close cache entry
  if (m_iBytesLeft == 0)
  {
     if (m_bCachedWrite && m_fcache)
        closeCacheEntry();
  }

  // FINALLY, we compute our final speed and let
  // everybody know that we are done...
  t_last = time(0L);
  if (t_last - t_start)
    speed((sz - m_request.offset) / (t_last - t_start));
  else
    speed(0);

  data( QByteArray() );
  return true;
}

void HTTPProtocol::error( int _err, const QString &_text )
{
  m_bKeepAlive = false;
  http_close();
  SlaveBase::error( _err, _text );
  m_bError = true;
}

void HTTPProtocol::addCookies( const QString &url, const QCString &cookieHeader )
{
   long windowId = m_request.window.toLong();
   QByteArray params;
   QDataStream stream(params, IO_WriteOnly);
   stream << url << cookieHeader << windowId;
   if (!m_dcopClient->send("kcookiejar", "kcookiejar",
        "addCookies(QString,QCString,long int)", params))
   {
      kdWarning(7103) << "Can't communicate with cookiejar!" << endl;
   }
}

QString HTTPProtocol::findCookies( const QString &url)
{
   QCString replyType;
   QByteArray params, reply;
   QDataStream stream(params, IO_WriteOnly);
   stream << url;
   if (!m_dcopClient->call("kcookiejar", "kcookiejar",
        "findCookies(QString)", params, replyType, reply))
   {
      kdWarning(7103) << "Can't communicate with cookiejar!" << endl;
      return QString::null;
   }

   QDataStream stream2(reply, IO_ReadOnly);
   if(replyType != "QString")
   {
      kdError(7103) << "DCOP function findCookies(...) returns " << replyType << ", expected QString" << endl;
      return QString::null;
   }

   QString result;
   stream2 >> result;

   return result;
}

// !START SYNC!
// The following code should be kept in sync
// with the code in http_cache_cleaner.cpp

#define CACHE_REVISION "7\n"

FILE* HTTPProtocol::checkCacheEntry( bool readWrite)
{
   const QChar seperator = '_';

   QString CEF = m_request.path;

   int p = CEF.find('/');

   while(p != -1)
   {
      CEF[p] = seperator;
      p = CEF.find('/', p);
   }

   QString host = m_state.hostname.lower();
   CEF = host + CEF + ':';

   QString dir = m_strCacheDir;
   if (dir[dir.length()-1] != '/')
      dir += "/";

   int l = host.length();
   for(int i = 0; i < l; i++)
   {
      if (host[i].isLetter() && (host[i] != 'w'))
      {
         dir += host[i];
         break;
      }
   }
   if (dir[dir.length()-1] == '/')
      dir += "0";

   unsigned long hash = 0x00000000;
   QCString u = m_request.url.url().latin1();
   for(int i = u.length(); i--;)
   {
      hash = (hash * 12211 + u[i]) % 2147483563;
   }

   QString hashString;
   hashString.sprintf("%08lx", hash);

   CEF = CEF + hashString;

   CEF = dir + "/" + CEF;

   m_state.cef = CEF;

   const char *mode = (readWrite ? "r+" : "r");

   FILE *fs = fopen( CEF.latin1(), mode); // Open for reading and writing
   if (!fs)
      return 0;

   char buffer[401];
   bool ok = true;

  // CacheRevision
  if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok && (strcmp(buffer, CACHE_REVISION) != 0))
      ok = false;

   time_t date;
   time_t currentDate = time(0);

   // URL
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      int l = strlen(buffer);
      if (l>0)
         buffer[l-1] = 0; // Strip newline
      if (m_request.url.url() != buffer)
      {
         ok = false; // Hash collision
      }
   }

   // Creation Date
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      date = (time_t) strtoul(buffer, 0, 10);
      if (m_maxCacheAge && (difftime(currentDate, date) > m_maxCacheAge))
      {
         m_bMustRevalidate = true;
         m_expireDate = currentDate;
      }
   }

   // Expiration Date
   m_cacheExpireDateOffset = ftell(fs);
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      if (m_request.cache == CC_Verify)
      {
         date = (time_t) strtoul(buffer, 0, 10);
         // After the expire date we need to revalidate.
         if (!date || difftime(currentDate, date) >= 0)
            m_bMustRevalidate = true;
         m_expireDate = date;
      }
   }

   // ETag
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      m_etag = QString(buffer).stripWhiteSpace();
   }

   // Last-Modified
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      m_lastModified = QString(buffer).stripWhiteSpace();
   }

   if (ok)
      return fs;

   fclose(fs);
   unlink( CEF.latin1());
   return 0;
}

void HTTPProtocol::updateExpireDate(time_t expireDate, bool updateCreationDate)
{
    bool ok = true;

    FILE *fs = checkCacheEntry(true);
    if (fs)
    {
        QString date;
        char buffer[401];
        time_t creationDate;

        fseek(fs, 0, SEEK_SET);
        if (ok && !fgets(buffer, 400, fs))
            ok = false;
        if (ok && !fgets(buffer, 400, fs))
            ok = false;
        long cacheCreationDateOffset = ftell(fs);
        if (ok && !fgets(buffer, 400, fs))
            ok = false;
        creationDate = strtoul(buffer, 0, 10);
        if (!creationDate)
            ok = false;

        if (updateCreationDate)
        {
           if (!ok || fseek(fs, cacheCreationDateOffset, SEEK_SET))
              return;
           QString date;
           date.setNum( time(0) );
           date = date.leftJustify(16);
           fputs(date.latin1(), fs);      // Creation date
           fputc('\n', fs);
        }

        if (expireDate>(30*365*24*60*60))
        {
            // expire date is a really a big number, it can't be
            // a relative date.
            date.setNum( expireDate );
        }
        else
        {
            // expireDate before 2000. those values must be
            // interpreted as relative expiration dates from
            // <META http-equiv="Expires"> tags.
            // so we have to scan the creation time and add
            // it to the expiryDate
            date.setNum( creationDate + expireDate );
        }
        date = date.leftJustify(16);
        if (!ok || fseek(fs, m_cacheExpireDateOffset, SEEK_SET))
            return;
        fputs(date.latin1(), fs);      // Expire date
        fseek(fs, 0, SEEK_END);
        fclose(fs);
    }
}

void HTTPProtocol::createCacheEntry( const QString &mimetype, time_t expireDate)
{
   QString dir = m_state.cef;
   int p = dir.findRev('/');
   if (p == -1) return; // Error.
   dir.truncate(p);

   // Create file
   (void) ::mkdir( dir.latin1(), 0700 );

   QString filename = m_state.cef + ".new";  // Create a new cache entryexpireDate

   m_fcache = fopen( filename.latin1(), "w");
   if (!m_fcache)
   {
      kdWarning(7103) << "createCacheEntry: opening " << filename << " failed." << endl;
      return; // Error.
   }

   fputs(CACHE_REVISION, m_fcache);    // Revision

   fputs(m_request.url.url().latin1(), m_fcache);  // Url
   fputc('\n', m_fcache);

   QString date;
   date.setNum( time(0) );
   date = date.leftJustify(16);
   fputs(date.latin1(), m_fcache);      // Creation date
   fputc('\n', m_fcache);

   date.setNum( expireDate );
   date = date.leftJustify(16);
   fputs(date.latin1(), m_fcache);      // Expire date
   fputc('\n', m_fcache);

   if (!m_etag.isEmpty())
      fputs(m_etag.latin1(), m_fcache);    //ETag
   fputc('\n', m_fcache);

   if (!m_lastModified.isEmpty())
      fputs(m_lastModified.latin1(), m_fcache);    // Last modified
   fputc('\n', m_fcache);

   fputs(mimetype.latin1(), m_fcache);  // Mimetype
   fputc('\n', m_fcache);

   if (!m_strCharset.isEmpty())
      fputs(m_strCharset.latin1(), m_fcache);    // Charset
   fputc('\n', m_fcache);

   return;
}
// The above code should be kept in sync
// with the code in http_cache_cleaner.cpp
// !END SYNC!

void HTTPProtocol::writeCacheEntry( const char *buffer, int nbytes)
{
   if (fwrite( buffer, nbytes, 1, m_fcache) != 1)
   {
      kdWarning(7103) << "writeCacheEntry: writing " << nbytes << " bytes failed." << endl;
      fclose(m_fcache);
      m_fcache = 0;
      QString filename = m_state.cef + ".new";
      unlink( filename.latin1());
      return;
   }
}

void HTTPProtocol::closeCacheEntry()
{
   QString filename = m_state.cef + ".new";
   int result = fclose( m_fcache);
   m_fcache = 0;
   if (result == 0)
   {
      if (::rename( filename.latin1(), m_state.cef.latin1()) == 0)
      {
         return; // Success
      }
      kdWarning(7103) << "closeCacheEntry: error renaming cache entry. ("
                   << filename << " -> " << m_state.cef << ")" << endl;
   }
   kdWarning(7103) << "closeCacheEntry: error closing cache entry. ("
                   << filename<< ")" << endl;
}

void HTTPProtocol::cleanCache()
{
   const time_t maxAge = MAX_CACHE_AGE; // 30 Minutes.
   bool doClean = false;
   QString cleanFile = m_strCacheDir;
   if (cleanFile[cleanFile.length()-1] != '/')
      cleanFile += "/";
   cleanFile += "cleaned";

   struct stat stat_buf;

   int result = ::stat(cleanFile.latin1(), &stat_buf);
   if (result == -1)
   {
      int fd = creat( cleanFile.latin1(), 0666);
      if (fd != -1)
      {
         doClean = true;
         ::close(fd);
      }
   }
   else
   {
      time_t age = (time_t) difftime( time(0), stat_buf.st_mtime );
      if (age > maxAge) //
        doClean = true;
   }
   if (doClean)
   {
      // Touch file.
      utime(cleanFile.latin1(), 0);
      KApplication::startServiceByDesktopPath("http_cache_cleaner.desktop");
   }
}

void HTTPProtocol::retrieveContent( bool check_ssl )
{
  if ( !retrieveHeader(false) )
    return;

  if (check_ssl)
    if (!checkSSL())
      return;

  if (!readBody())
    return;

  http_close();
  finished();
}

bool HTTPProtocol::retrieveHeader( bool close_connection )
{
  resetSessionSettings();
  while ( 1 )
  {
    if (!http_open())
        return false;

    if (!readHeader())
    {
        if ( m_bError )
            return false;
    }
    else
    {
        // Do not save authorization if the current response code is
        // 4xx (client error) or 5xx (server error).
        kdDebug(7113) << "Previous Response: " << m_prevResponseCode << endl
                      << "Current Response: " << m_responseCode << endl;
        if ( m_responseCode < 400 &&
            (m_prevResponseCode == 401 || m_prevResponseCode == 407) )
            saveAuthorization();
        break;
    }
  }
  if ( close_connection )
  {
    http_close();
    finished();
  }
  return true;
}

bool HTTPProtocol::retryPrompt()
{
  QString prompt;
  switch ( m_responseCode )
  {
    case 401:
      prompt = i18n("Authentication Failed!");
      break;
    case 407:
      prompt = i18n("Proxy Authentication Failed!");
      break;
    default:
      break;
  }
  prompt += i18n("  Do you want to retry ?");
  return (messageBox(QuestionYesNo, prompt, i18n("Authentication")) == 3);
}

void HTTPProtocol::promptInfo( AuthInfo& info )
{
  if ( m_responseCode == 401 )
  {
    info.url = m_request.url;
    if ( !m_state.user.isEmpty() )
      info.username = m_state.user;
    info.readOnly = !m_request.url.user().isEmpty();
    info.prompt = i18n( "You need to supply a username and a "
                        "password to access this site." );
    //info.keepPassword = true; // Prompt the user for persistence as well.
    if ( !m_strRealm.isEmpty() )
    {
      info.realmValue = m_strRealm;
      info.verifyPath = false;
      if ( Authentication == AUTH_Digest )
        info.digestInfo = m_strAuthorization;
      info.commentLabel = i18n( "Site:" );
      info.comment = i18n("<b>%1</b> at <b>%2</b>").arg( m_strRealm ).arg( m_request.hostname );
    }
  }
  else if ( m_responseCode == 407 )
  {
    info.url = m_proxyURL;
    info.username = m_proxyURL.user();
    info.prompt = i18n( "You need to supply a username and a password for "
                        "the proxy server listed below before you are allowed "
                        "to access any sites." );
    info.keepPassword = true;
    if ( !m_strProxyRealm.isEmpty() )
    {
      info.realmValue = m_strProxyRealm;
      info.verifyPath = false;
      if ( ProxyAuthentication == AUTH_Digest )
        info.digestInfo = m_strProxyAuthorization;
      info.commentLabel = i18n( "Proxy:" );
      info.comment = i18n("<b>%1</b> at <b>%2</b>").arg( m_strProxyRealm ).arg( m_proxyURL.host() );
    }
  }
}

bool HTTPProtocol::getAuthorization()
{
  AuthInfo info;
  bool result = false;
  bool repeatFailure = (m_prevResponseCode == m_responseCode);

  if ( repeatFailure )
  {
    bool prompt = true;
    if ( Authentication == AUTH_Digest )
    {
      bool isStaleNonce = false;
      QString auth = ( m_responseCode == 401 ) ? m_strAuthorization : m_strProxyAuthorization;
      int pos = auth.find("stale", 0, false);
      if ( pos != -1 )
      {
        pos += 5;
        int len = auth.length();
        while( pos < len && (auth[pos] == ' ' || auth[pos] == '=') ) pos++;
        if ( pos < len && auth.find("true", pos, false) != -1 )
        {
          isStaleNonce = true;
          kdDebug(7113) << "Stale nonce value. Will retry using same info..." << endl;
        }
      }
      if ( isStaleNonce )
      {
        prompt = false;
        result = true;
        if ( m_responseCode == 401 )
        {
          info.username = m_request.user;
          info.password = m_request.passwd;
          info.realmValue = m_strRealm;
          info.digestInfo = m_strAuthorization;
        }
        else if ( m_responseCode == 407 )
        {
          info.username = m_proxyURL.user();
          info.password = m_proxyURL.pass();
          info.realmValue = m_strProxyRealm;
          info.digestInfo = m_strProxyAuthorization;
        }
      }
    }
    if ( prompt && !retryPrompt() )
    {
      if (m_bErrorPage)
         errorPage();
      else
         error(ERR_USER_CANCELED, QString::null);
      return false;
    }
  }
  else
  {
    // At this point we know more detials, so use it to find
    // out if we have a cached version and avoid a re-prompt!
    // We also do not use verify path unlike the pre-emptive
    // requests because we already know the realm value...
    if ( m_responseCode == 407 )
    {
      info.url = m_proxyURL;
      info.realmValue = m_strProxyRealm;
    }
    else
    {
      info.url = m_request.url;
      info.username = m_request.user;
      info.realmValue = m_strRealm;
    }

    info.verifyPath = false;    
    result = checkCachedAuthentication( info );
    if ( Authentication == AUTH_Digest )
    {
      QString auth = (m_responseCode == 401) ? m_strAuthorization : m_strProxyAuthorization;
      int pos = auth.find("stale", 0, false);
      if ( pos != -1 )
      {
        pos += 5;
        int len = auth.length();
        while( pos < len && (auth[pos] == ' ' || auth[pos] == '=') ) pos++;
        if ( pos < len && auth.find("true", pos, false) != -1 )
        {
          info.digestInfo = (m_responseCode == 401) ? m_strAuthorization : m_strProxyAuthorization;
          kdDebug(7113) << "Just a stale nonce value! Retrying with the new nonce sent!" << endl;
        }
      }
    }
  }

  if ( !result  )
  {
    if ( !repeatFailure && !m_request.user.isEmpty() &&
         !m_request.passwd.isEmpty() && m_responseCode == 401 )
      result = true;
    else
    {
      kdDebug( 7113 ) << "About to prompt user for authorization..." << endl;
      promptInfo( info );
      result = openPassDlg( info );
    }
  }

  if ( result )
  {
    switch (m_responseCode)
    {
      case 401: // Request-Authentication
        m_request.user = info.username;
        m_request.passwd = info.password;
        m_strRealm = info.realmValue;
        if ( Authentication == AUTH_Digest )
          m_strAuthorization = info.digestInfo;
        break;
      case 407: // Proxy-Authentication
        m_proxyURL.setUser( info.username );
        m_proxyURL.setPass( info.password );
        m_strProxyRealm = info.realmValue;
        if ( Authentication == AUTH_Digest )
          m_strProxyAuthorization = info.digestInfo;
        break;
      default:
        break;
    }
    return true;
  }

  if (m_bErrorPage)
     errorPage();
  else
     error( ERR_USER_CANCELED, QString::null );
  return false;
}

void HTTPProtocol::saveAuthorization()
{
  AuthInfo info;  
  if ( m_prevResponseCode == 407 )
  {
    info.url = m_proxyURL;
    info.username = m_proxyURL.user();
    info.password = m_proxyURL.pass();
    info.realmValue = m_strProxyRealm;
    if( Authentication == AUTH_Digest )
      info.digestInfo = m_strProxyAuthorization;
    cacheAuthentication( info );
  }
  else
  {
    info.url = m_request.url;
    info.username = m_request.user;
    info.password = m_request.passwd;
    info.realmValue = m_strRealm;
    if( Authentication == AUTH_Digest )
      info.digestInfo = m_strAuthorization;
    cacheAuthentication( info );
  }
}

QString HTTPProtocol::createBasicAuth( bool isForProxy )
{
  QString auth;
  QCString user, passwd;
  if ( isForProxy )
  {
    auth = "Proxy-Authorization: Basic ";
    user = m_proxyURL.user().latin1();
    passwd = m_proxyURL.pass().latin1();
  }
  else
  {
    auth = "Authorization: Basic ";
    user = m_state.user.latin1();
    passwd = m_state.passwd.latin1();
  }

  if ( !user.isEmpty() )
  {
    if( !passwd.isEmpty() )
      user += ':' + passwd;
    auth += KCodecs::base64Encode(user);
  }
  else
    auth = QString::null;

  return auth;
}

void HTTPProtocol::calculateResponse( DigestAuthInfo& info, HASHHEX Response )
{
  KMD5 md;
  HASHHEX HA1, HA2;
  QCString authStr;

  // Calculate H(A1)
  authStr = info.username + ':' + info.realm + ':' + info.password;
  md.update( authStr );
  md.finalize();
  if ( info.algorithm == "md5-sess" )
  {
    authStr = md.hexDigest();
    authStr += ':' + info.nonce + ':' + info.cnonce;
    md.reset();
    md.update( authStr );
    md.finalize();
  }
//  kdDebug(7113) << "A1 => " << authStr << endl;
  md.hexDigest( HA1 );

  // Calcualte H(A2)
  authStr = info.method + ':' + info.digestURI.at( 0 );
  if ( info.qop == "auth-int" )
    authStr += ':' + info.entity_body;
//  kdDebug(7113) << "A2 => " << authStr << endl;
  md.reset();
  md.update( authStr );
  md.finalize();
  md.hexDigest( HA2 );

  // Calcualte the response.
  authStr = HA1;
  authStr += ':' + info.nonce + ':';
  if ( !info.qop.isEmpty() )
    authStr += info.nc + ':' + info.cnonce + ':' + info.qop + ':';
  authStr += HA2;

//  kdDebug(7113) << "response:" << authStr << endl;
  md.reset();
  md.update( authStr );
  md.finalize();
  md.hexDigest( Response );
}

QString HTTPProtocol::createDigestAuth ( bool isForProxy )
{
  QString auth;
  const char *p;
  HASHHEX Response;
  QCString opaque = "";
  DigestAuthInfo info;

  if ( isForProxy )
  {
    auth = "Proxy-Authorization: Digest ";
    info.username = m_proxyURL.user().latin1();
    info.password = m_proxyURL.pass().latin1();
    p = m_strProxyAuthorization.latin1();
  }
  else
  {
    auth = "Authorization: Digest ";
    info.username = m_state.user.latin1();
    info.password = m_state.passwd.latin1();
    p = m_strAuthorization.latin1();

  }

  if ( info.username.isEmpty() || info.password.isEmpty() || !p )
    return QString::null;

  // info.entity_body = p;   // FIXME: need to have the data to be sent for POST action!!
  info.realm = "";
  info.algorithm = "MD5";
  info.nonce = "";
  info.qop = "";

  // Use some random # b/n 1 and 100,000 for generating the nonce value...
  info.cnonce.setNum((1 + static_cast<int>(100000.0*rand()/(RAND_MAX+1.0))));
  info.cnonce = KCodecs::base64Encode( info.cnonce );

  // HACK: Should be fixed according to RFC 2617 section 3.2.2
  info.nc = "00000001";

  // Set the method used...
  switch ( m_request.method )
  {
    case HTTP_GET:
        info.method = "GET";
        break;
    case HTTP_PUT:
        info.method = "PUT";
        break;
    case HTTP_POST:
        info.method = "POST";
        break;
    case HTTP_HEAD:
        info.method = "HEAD";
        break;
    case HTTP_DELETE:
        info.method = "DELETE";
        break;
    default:
        break;
  }

  // Parse the Digest response....
  while (*p)
  {
    int i = 0;
    while ( (*p == ' ') || (*p == ',') || (*p == '\t')) { p++; }
    if (strncasecmp(p, "realm=", 6 )==0)
    {
      p+=6;
      while ( *p == '"' ) p++;  // Go past any number of " mark(s) first
      while ( p[i] != '"' ) i++;  // Read everything until the last " mark
      info.realm = QCString( p, i+1 );
    }
    else if (strncasecmp(p, "algorith=", 9)==0)
    {
      p+=9;
      while ( *p == '"' ) p++;  // Go past any number of " mark(s) first
      while ( ( p[i] != '"' ) && ( p[i] != ',' ) && ( p[i] != '\0' ) ) i++;
      info.algorithm = QCString(p, i+1).lower();
    }
    else if (strncasecmp(p, "algorithm=", 10)==0)
    {
      p+=10;
      while ( *p == '"' ) p++;  // Go past any " mark(s) first
      while ( ( p[i] != '"' ) && ( p[i] != ',' ) && ( p[i] != '\0' ) ) i++;
      info.algorithm = QCString(p,i+1).lower();
    }
    else if (strncasecmp(p, "domain=", 7)==0)
    {
      p+=7;
      while ( *p == '"' ) p++;  // Go past any " mark(s) first
      while ( p[i] != '"' ) i++;  // Read everything until the last " mark
      int pos = 0, idx = 0;
      QCString uri = QCString(p,i+1);
      do
      {
        pos = uri.find( ',', pos );
        if ( pos != -1 )
          info.digestURI.append( uri.mid(idx, pos-idx) );
        else
          info.digestURI.append( uri.mid(idx, uri.length()-idx) );
        idx = pos+1;
      } while ( pos != -1 );
    }
    else if (strncasecmp(p, "nonce=", 6)==0)
    {
      p+=6;
      while ( *p == '"' ) p++;  // Go past any " mark(s) first
      while ( p[i] != '"' ) i++;  // Read everything until the last " mark
      info.nonce = QCString(p,i+1);
    }
    else if (strncasecmp(p, "opaque=", 7)==0)
    {
      p+=7;
      while ( *p == '"' ) p++;  // Go past any " mark(s) first
      while ( p[i] != '"' ) i++;  // Read everything until the last " mark
      opaque = QCString(p,i+1);
    }
    else if (strncasecmp(p, "qop=", 4)==0)
    {
      p+=4;
      while ( *p == '"' ) p++;  // Go past any " mark(s) first
      while ( p[i] != '"' ) i++;  // Read everything until the last " mark
      info.qop = QCString(p,i+1);
    }
    p+=(i+1);
  }

  if ( info.digestURI.isEmpty() )
    info.digestURI.append( m_request.path.latin1() );

  kdDebug(7113) << "RESULT OF PARSING:" << endl
                << "  algorithm: " << info.algorithm << endl
                << "  realm:     " << info.realm << endl
                << "  nonce:     " << info.nonce << endl
                << "  opaque:    " << opaque << endl
                << "  qop:       " << info.qop << endl;
  int count = info.digestURI.count();
  for( int i = 0; i < count; i++ )
    kdDebug(7113) << "  domain[" << i << "]:    " << info.digestURI.at(i) << endl;

  // Calculate the response...
  calculateResponse( info, Response );

  auth += "username=\"";
  auth += info.username;

  auth += "\", realm=\"";
  auth += info.realm;
  auth += "\"";

  auth += ", nonce=\"";
  auth += info.nonce;

  auth += "\", uri=\"";
  auth += info.digestURI.at(0);

  auth += "\", algorithm=\"";
  auth += info.algorithm;
  auth +="\"";

  if ( !info.qop.isEmpty() )
  {
    auth += ", qop=\"";
    auth += info.qop;
    auth += "\", cnonce=\"";
    auth += info.cnonce;
    auth += "\", nc=";
    auth += info.nc;
  }

  auth += ", response=\"";
  auth += Response;
  if ( !opaque.isEmpty() )
  {
    auth += "\", opaque=\"";
    auth += opaque;
  }
  auth += "\"";

  kdDebug(7113) << "Digest header: " << auth << endl;
  return auth;
}

void HTTPProtocol::reparseConfiguration()
{
  kdDebug(7103) << "(" << getpid() << ") Reparse Configuration!" << endl;
  m_strProxyRealm = QString::null;
  m_strProxyAuthorization = QString::null;

  ProxyAuthentication = AUTH_None;

  // Define language and charset settings from KLocale (David)
  // Get rid of duplicate language entries!!
  QString tmp;
  QStringList languageList = KGlobal::locale()->languageList();
  QStringList::Iterator it = languageList.find( QString::fromLatin1("C") );
  kdDebug(7103) << "Languages: " << KGlobal::locale()->languages() << endl;
  if ( it != languageList.end() )
  {
    if ( languageList.contains( QString::fromLatin1("en") ) > 0 )
        languageList.remove( it );
    else
        (*it) = QString::fromLatin1("en");
  }

  // Use commas not spaces.
  m_strLanguages = languageList.join( ", " );
  kdDebug(7103) << "Languages list set to " << m_strLanguages << endl;
  // Ugly conversion. kdeglobals has the xName (e.g. iso8859-1 instead of iso-8859-1)
  m_strCharsets = KGlobal::charsets()->name(KGlobal::charsets()->xNameToID(KGlobal::locale()->charset()));
  m_strCharsets += QString::fromLatin1(", utf-8, *");

  // Launch the cookiejar if not already running
  KConfig *cookieConfig = new KConfig("kcookiejarrc", false, false);
  cookieConfig->setGroup("Cookie Policy");
  m_bUseCookiejar = cookieConfig->readBoolEntry( "Cookies", true );
  if (m_bUseCookiejar && !m_dcopClient->isApplicationRegistered("kcookiejar"))
  {
     QString error;
     if (KApplication::startServiceByDesktopName("kcookiejar", QStringList(), &error ))
     {
        // Error starting kcookiejar.
        kdDebug(1202) << "Error starting KCookiejar: " << error << "\n" << endl;
     }
  }
  delete cookieConfig;

#ifdef DO_SSL
  if (m_protocol == "https")
  {
     struct servent *sent = getservbyname("https", "tcp");
     if (sent) {
        m_DefaultPort = ntohs(sent->s_port);
     } else {
        m_DefaultPort = DEFAULT_HTTPS_PORT;
     }
  }
  else
#endif
  if (m_protocol == "ftp")
  {
     struct servent *sent = getservbyname("ftp", "tcp");
     if (sent) {
        m_DefaultPort = ntohs(sent->s_port);
     } else {
        m_DefaultPort = DEFAULT_FTP_PORT;
     }
  }
  else
  {
     struct servent *sent = getservbyname("http", "tcp");
     if (sent) {
        m_DefaultPort = ntohs(sent->s_port);
     } else {
        m_DefaultPort = DEFAULT_HTTP_PORT;
     }
  }
}

void HTTPProtocol::resetSessionSettings()
{
  m_request.window = metaData("window-id");
  m_responseCode = 0;
  m_prevResponseCode = 0;

  m_strRealm = QString::null;
  m_strAuthorization = QString::null;
  Authentication = AUTH_None;

  m_bCanResume = false;
  m_bUnauthorized = false;
}


