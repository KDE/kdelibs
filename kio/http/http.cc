// -*- c-basic-offset: 2 -*-

#include <config.h>

//
//  Portions Copyright 2000 George Staikos <staikos@kde.org>
//  (mostly SSL related)
//

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

#define DO_MD5

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
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

#include <kconfig.h>
#include <kdesu/client.h>

#ifdef DO_SSL
#include <kssl.h>
#include <ksslinfodlg.h>
#endif

#ifdef DO_MD5
#include "extern_md5.h"
#endif

#include "base64.h"

#include <fcntl.h>

#ifdef DO_GZIP
#include <zlib.h>
#endif

#include "http.h"

#include <qregexp.h>

#include <kapp.h>
#include <klocale.h>
#include <kprotocolmanager.h>
#include <kdatastream.h>
#include <ksock.h>
#include <kurl.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <kservice.h>
#include <krfcdate.h>
#include <kmessagebox.h>

using namespace KIO;

// Maximum chunk size is 256K
#define MAX_CHUNK_SIZE (1024*256)

#define MAX_IPC_SIZE (1024*8)

// Default expire time in seconds: 1 min.
#define DEFAULT_EXPIRE (1*60)

// Timeout for connections to remote sites in seconds
#define REMOTE_CONNECT_TIMEOUT 20

// Timeout for connections to proxy in seconds
#define PROXY_CONNECT_TIMEOUT 10

// Timeout for receiving an answer from a remote side in seconds.
#define RESPONSE_TIMEOUT 60

extern "C" {
  const char* create_basic_auth (const char *header, const char *user, const char *passwd);
  const char* create_digest_auth (const char *header, const char *user,
                                  const char *passwd, const char *auth_str);
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

  // Launch the cookiejar if not already running
  KConfig *cookieConfig = new KConfig("kcookiejarrc", false, false);
  if( cookieConfig->hasGroup("Browser Settings/HTTP") &&
          !cookieConfig->hasGroup("Cookie Policy") )
        cookieConfig->setGroup("Browser Settings/HTTP");
  else
        cookieConfig->setGroup("Cookie Policy");
  if( cookieConfig->readBoolEntry( "Cookies", true ) )
  {
     QString error;
     if (KApplication::startServiceByDesktopName("kcookiejar", QStringList(), &error ))
     {
        // Error starting kcookiejar.
        kdDebug(1202) << "Error starting KCookiejar: " << error << "\n" << endl;
     }
  }
  delete cookieConfig;

  HTTPProtocol slave(argv[1], argv[2], argv[3]);
  slave.dispatchLoop();

  return 0;
}

static char * trimLead (char *orig_string) {
  while (*orig_string == ' ')
    orig_string++;
  return orig_string;
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

#ifdef DO_MD5
const char * create_digest_auth (const char *header, const char *user,
				                          const char *passwd, const char *auth_str)
{
  int i;
  HASHHEX HA1;
  HASHHEX HA2 = "";
  HASHHEX Response;

  const char *p=auth_str;
  QCString domain ="", realm = "", algorithm="MD5", nonce="", opaque="", qop="";

  if (!user || !passwd || p == 0)
    return "";

  QCString t1;
  kdDebug(7113) << "User is :" << user << ":    Password is : ..." << /* passwd << */ ":" << endl;
  while (*p)
  {
    while( (*p == ' ') || (*p == ',') || (*p == '\t')) { p++; }
    i = 0;
    if (strncasecmp(p, "realm=", 6 )==0)
    {
      p+=6;
      while( *p == '"' ) p++;  // Go past any number of " mark(s) first
      while( p[i] != '"' ) i++;  // Read everything until the last " mark
      realm = QCString( p, i+1 );
      kdDebug(7113) << "realm:==> " << realm.data() << endl;
    }
    else if (strncasecmp(p, "algorith=", 9)==0)
    {
      p+=9;
      while( *p == '"' ) p++;  // Go past any number of " mark(s) first
      while( p[i] != '"' ) i++;  // Read everything until the last " mark
      algorithm = QCString(p, i+1);
      kdDebug(7113) << "algorith:==> " << algorithm << endl;
    }
    else if (strncasecmp(p, "algorithm=", 10)==0)
    {
      p+=10;
      while( *p == '"' ) p++;  // Go past any " mark(s) first
      while( p[i] != '"' ) i++;  // Read everything until the last " mark
      algorithm = QCString(p,i+1);
      kdDebug(7113) << "algorithm:==> " << algorithm << endl;
    }
    else if (strncasecmp(p, "domain=", 7)==0)
    {
      p+=7;
      while( *p == '"' ) p++;  // Go past any " mark(s) first
      while( p[i] != '"' ) i++;  // Read everything until the last " mark
      domain = QCString(p,i+1);
      kdDebug(7113) << "domain:==> " << domain << endl;
    }
    else if (strncasecmp(p, "nonce=", 6)==0)
    {
      p+=6;
      while( *p == '"' ) p++;  // Go past any " mark(s) first
      while( p[i] != '"' ) i++;  // Read everything until the last " mark
      nonce = QCString(p,i+1);
      kdDebug(7113) << "nonce:==> " << nonce << endl;
    }
    else if (strncasecmp(p, "opaque=", 7)==0)
    {
      p+=7;
      while( *p == '"' ) p++;  // Go past any " mark(s) first
      while( p[i] != '"' ) i++;  // Read everything until the last " mark
      opaque = QCString(p,i+1);
      kdDebug(7113) << "opaque:==>" << opaque << endl;
    }
    else if (strncasecmp(p, "qop=", 4)==0)
    {
      p+=4;
      while( *p == '"' ) p++;  // Go past any " mark(s) first
      while( p[i] != '"' ) i++;  // Read everything until the last " mark
      qop = QCString(p,i+1);
      kdDebug(7113) << "qop:==> " << qop << endl;
    }
    p+=(i+1);
  }

  t1 += header;
  t1 += ": Digest username=\"";
  t1 += user;
  t1 += "\"";

  t1 += ", realm=\"";
  t1 += realm.data();
  t1 += "\"";

  t1 += ", nonce=\"";
  t1 += nonce.data();
  t1 += "\"";

  if( !domain.isEmpty() )
  {
    t1 += ", uri=";
	  t1 += domain.data();
  }

  const char* szCNonce = "4477b65d"; // RIDDLE: Can anyone guess what this value means ??
  const char* szNonceCount = "00000001";

  DigestCalcHA1(algorithm.data(), user, realm.data(), passwd, nonce.data(), szCNonce, HA1);

  // FIXME- I ONLY WORK FOR "GET"!!  What about other methods ??
  DigestCalcResponse(HA1, nonce.data(), szNonceCount, szCNonce, qop.data(), "GET", domain.data(), HA2, Response);

  if( !qop.isEmpty() )
  {
    t1 += ", qop=";
    t1 += qop.data();
    t1 += ", nc=";
    t1 += szNonceCount; // HACK: Should be fixed according to RFC 2617 section 3.2.2
    t1 += ", cnonce=\"";
    t1 += szCNonce;
    t1 += "\"";
  }

  t1 += ", response=\"";
  t1 += Response;
  t1 += "\"";

  if ( !opaque.isEmpty() )
  {
    t1 += ", opaque=\"";
    t1 += opaque.data();
    t1 += "\"";
  }
  // Please do not add things that should be done
  // by the header generator method itself!! (DA)
  // t1 += "\r\n";
  kdDebug(7113) << "Digest Response: " << t1.data() << endl;
  return qstrdup(t1.data());
}
#else
const char *create_digest_auth (const char *, const char *, const char *, const char *)
{
  kdError(7113) << "Cannot perform digest authentication!!!" << endl;
  // error(ERR_COULD_NOT_AUTHENTICATE, "digest");

  // Please do not add things that should be done
  // by the header generator method itself!! (DA)
  return strdup(""); // ("\r\n");
}
#endif

const char *create_basic_auth (const char *header, const char *user, const char *passwd)
{
  char *wwwauth;
  if (user && passwd) {
    char *t1, *t2;

    t1 = (char *)malloc(strlen(user)+strlen(passwd)+2);
    memset(t1, 0, strlen(user)+strlen(passwd)+2);
    sprintf(t1, "%s:%s", user, passwd);
    t2 = base64_encode_line(t1);
    free(t1);
    wwwauth = (char *)malloc(strlen(t2) + strlen(header) + 11); // UPDATE WHEN FORMAT BELOW CHANGES !!!

    // Please do not add things that should be done
    // by the header generator method itself!! (DA)
    // sprintf(wwwauth, "%s: Basic %s\r\n", header, t2);
    sprintf(wwwauth, "%s: Basic %s", header, t2);
    free(t2);
  }
  else
    wwwauth = NULL;

  return(wwwauth);
}

/* Domain suffix match. E.g. return true if host is "cuzco.inka.de" and
   nplist is "inka.de,hadiko.de" or if host is "localhost" and nplist is
   "localhost".
*/
bool revmatch(const char *host, const char *nplist)
{
  const char *hptr = host + strlen( host ) - 1;
  const char *nptr = nplist + strlen( nplist ) - 1;
  const char *shptr = hptr;

  while( nptr >= nplist )
  {
    if ( *hptr != *nptr )
    {
      hptr = shptr;
      // Try to find another domain or host in the list
      while(--nptr>=nplist && *nptr!=',' && *nptr!=' ') ;
      // Strip out multiple spaces and commas
      while(--nptr>=nplist && (*nptr==',' || *nptr==' ')) ;
    }
    else
    {
      if ( nptr==nplist || nptr[-1]==',' || nptr[-1]==' ')
        return true;
      hptr--; nptr--;
    }
  }

  return false;
}

/*
bool revmatch( const QString& host, const QString& nplist )
{
    bool found = false;
    // Any amount of "space" or "comma" separated list.
    QStringList np = QStringList::split(QRegExp("[, ]") , nplist);
    QStringList::ConstIterator it = np.begin();
    for( ; it != np.end(); ++it )
    {
        int pos = host.findRev((*it));
        int exp_pos = host.length() - (*it).length();
        if( pos == exp_pos )
        {
            found = true;
            break;
        }
    }
    return found;
}
*/
/*****************************************************************************/

HTTPProtocol::HTTPProtocol( const QCString &protocol, const QCString &pool, const QCString &app )
  : SlaveBase( (protocol=="ftp") ? QCString("ftp-proxy") : protocol , pool, app)
{
  m_protocol = protocol;
  kdDebug() << "******* mProtocol=" << mProtocol << "     m_protocol=" << m_protocol << " ********" << endl;
  m_maxCacheAge = 0;
  m_fsocket = 0L;
  m_sock = 0;
  m_fcache = 0;
  m_bKeepAlive = false;
  m_iSize = -1;
  m_iAuthFailed = 0;
  m_dcopClient = new DCOPClient();
  if (!m_dcopClient->attach())
  {
     kdDebug(7103) << "Can't connect with DCOP server." << endl;
  }

  m_bCanResume = true; // most of http servers support resuming ?

  reparseConfiguration();

  flushAuthenticationSettings();

  m_bEOF=false;
#ifdef DO_SSL
  m_bUseSSL=true;
#endif

  m_sContentMD5 = "";
  Authentication = AUTH_None;
  ProxyAuthentication = AUTH_None;

  m_HTTPrev = HTTP_Unknown;

#ifdef DO_SSL
  if (m_protocol == "https")
  {
     struct servent *sent = getservbyname("https", "tcp");
     if (sent) {
        mDefaultPort = ntohs(sent->s_port);
     } else {
        mDefaultPort = DEFAULT_HTTPS_PORT;
     }
  }
  else
#endif
  if (m_protocol == "ftp")
  {
     struct servent *sent = getservbyname("ftp", "tcp");
     if (sent) {
        mDefaultPort = ntohs(sent->s_port);
     } else {
        mDefaultPort = DEFAULT_FTP_PORT;
     }
  }
  else
  {
     struct servent *sent = getservbyname("http", "tcp");
     if (sent) {
        mDefaultPort = ntohs(sent->s_port);
     } else {
        mDefaultPort = DEFAULT_HTTP_PORT;
     }
  }

  cleanCache();
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
                  << "+-----------------------------------------------"
                  << endl;
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
  m_fsocket = fdopen( m_sock, "r+" );
  if( !m_fsocket ) {
    return false;
  }
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
  ssize_t ret;
#ifdef DO_SSL
  if (m_bUseSSL) {
    m_bEOF=false;
    ret=m_ssl.read((char *)b, nbytes);
    if (ret==0) m_bEOF=true;
    return ret;
  }
#endif
  ret=fread(b, 1, nbytes, m_fsocket);
  m_bEOF = feof(m_fsocket);
  return ret;
}


bool HTTPProtocol::eof()
{
  return m_bEOF;
}

bool
HTTPProtocol::http_isConnected()
{
   if (!m_sock) return false;
   fd_set rdfs;
   struct timeval tv;
   int retval;
   FD_ZERO(&rdfs);
   FD_SET(m_sock , &rdfs);
   tv.tv_usec = 0;
   tv.tv_sec = 0;
   retval = select(m_sock+1, &rdfs, NULL, NULL, &tv);

   if (retval != 0)
   {
       char buffer[100];
       retval = recv(m_sock, buffer, 80, MSG_PEEK);
       return false;
   }
   return true;
}

void HTTPProtocol::http_checkConnection()
{
  // do we want to use a proxy?
  // if so, we had first better make sure that our host isn't on the
  // No Proxy list
  if (m_request.do_proxy && !m_strNoProxyFor.isEmpty())
  {
    m_request.do_proxy = !revmatch( m_request.hostname.latin1(), m_strNoProxyFor.latin1() );
  }

  if (m_sock)
  {
     bool closeDown = false;
     if (!m_state.do_proxy && !m_request.do_proxy)
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
     else if (m_request.do_proxy && m_state.do_proxy)
     {
        // Keep the connection to the proxy.
     }
     else
     {
        closeDown = true;
     }
     if (!closeDown && !http_isConnected())
     {
        closeDown = true;
     }
     if (closeDown)
        http_closeConnection();
  }

  // let's update our current state
  m_state.hostname = m_request.hostname;
  m_state.port = m_request.port;
  m_state.user = m_request.user;
  m_state.passwd = m_request.passwd;
  m_state.do_proxy = m_request.do_proxy;
}

static bool waitForConnect( int sock, int maxTimeout )
{
  fd_set wr;
  struct timeval timeout;

  int n = maxTimeout; // Timeout in seconds
  while(n--){
      FD_ZERO(&wr);
      FD_SET(sock, &wr);

      timeout.tv_usec = 0;
      timeout.tv_sec = 1; // 1 sec

      select(sock + 1, (fd_set *)0, &wr, (fd_set *)0, &timeout);

      if (FD_ISSET(sock, &wr))
      {
         int errcode;
         ksize_t len = sizeof(errcode);
         int ret = getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&errcode, &len);
         if ((ret == -1) || (errcode != 0))
         {
            return false;
         }
         return true;
      }
  }
  return false; // Timeout
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
    m_sock = ::socket(PF_INET,SOCK_STREAM,0);
    if (m_sock < 0) {
      m_sock = 0;
      error( ERR_COULD_NOT_CREATE_SOCKET, m_state.hostname );
      return false;
    }

    // Set socket non-blocking.
    fcntl(m_sock, F_SETFL, ( fcntl(m_sock, F_GETFL)|O_NDELAY));

    // do we still want a proxy after all that?
    if( m_state.do_proxy ) {
      QString proxy_host = m_proxyURL.host();
      int proxy_port = m_proxyURL.port();
      kdDebug(7113) << "http_openConnection " << proxy_host << " " << proxy_port << endl;
      // yep... open up a connection to the proxy instead of our host
      if(!KSocket::initSockaddr(&m_proxySockaddr, proxy_host.latin1(), proxy_port)) {
        error(ERR_UNKNOWN_PROXY_HOST, proxy_host);
        return false;
      }

      infoMessage( i18n("Connecting to %1...").arg(m_state.hostname) );

      if(::connect(m_sock, (struct sockaddr*)(&m_proxySockaddr), sizeof(m_proxySockaddr))) {
        if((errno != EINPROGRESS) && (errno != EWOULDBLOCK)) {
          // Error
          error(ERR_COULD_NOT_CONNECT, proxy_host );
          kdDebug(7103) << "Could not connect to PROXY server!!" << endl;
          return false;
        }
        // Wait for connection
        if (!waitForConnect(m_sock, PROXY_CONNECT_TIMEOUT))
        {
          error(ERR_COULD_NOT_CONNECT, proxy_host );
          kdDebug(7103) << "Timed out waiting to connect to PROXY server!!" << endl;
          return false;
        }
      }

      // SSL proxying requires setting up a tunnel through the proxy server
      // with the CONNECT directive.
#ifdef DO_SSL
      if (m_bUseSSL) {   // we might have DO_SSL but not m_bUseSSL someday
      kdDebug(7113) << "http proxy for SSL - setting up" << endl;
      // Set socket blocking.
      fcntl(m_sock, F_SETFL, ( fcntl(m_sock, F_GETFL) & ~O_NDELAY));

      QString proxyconheader = QString("CONNECT %1:443 HTTP/1.0\r\n\r\n").arg(m_request.hostname);
      // WARNING: ugly hack alert!  We don't want to use the SSL routines
      //          for this code so we have to disabled it temporarily.
      bool useSSLSaved = m_bUseSSL;   m_bUseSSL = false;
      bool sendOk = (write(proxyconheader.latin1(), proxyconheader.length())
                           == (ssize_t) proxyconheader.length());
      char buffer[513];
      if (!sendOk) {
        // FIXME: do we have to close() the connection here?
        //        also the error code should be changed
        error(ERR_COULD_NOT_CONNECT, proxy_host );
        m_bUseSSL = useSSLSaved;
        return false;
      }

      if (!waitForHeader(m_sock, RESPONSE_TIMEOUT) ) {
        // FIXME: a good workaround would be to fallback to non-proxy mode
        //        here if we can.
        // FIXME: do we have to close() the connection here?
        //        also the error code should be changed
        error(ERR_COULD_NOT_CONNECT, proxy_host );
        m_bUseSSL = useSSLSaved;
        return false;
      }

      // In SSL mode we normally don't use this.  Does it cause a resource
      // leak that I'm not cleaning this up here if we're in SSL mode?
      int rhrc = ::read(m_sock, buffer, sizeof(buffer)-1);
      buffer[sizeof(buffer)-1] = 0;  // just in case so we don't run away!
      if (rhrc == -1 || strncmp(buffer, "HTTP/1.0 200", 12)) {
        // FIXME: a good workaround would be to fallback to non-proxy mode
        //        here if we can.
        // FIXME: do we have to close() the connection here?
        //        also the error code should be changed
        error(ERR_COULD_NOT_CONNECT, proxy_host );
        m_bUseSSL = useSSLSaved;
        return false;
      }

      m_bUseSSL = useSSLSaved;
      }  // if m_bUseSSL
      m_ssl_ip = i18n("Proxied by %1.").arg(inet_ntoa(m_proxySockaddr.sin_addr));
#endif
    } else {
      // apparently we don't want a proxy.  let's just connect directly
      ksockaddr_in server_name;

      if(!KSocket::initSockaddr(&server_name, m_state.hostname.latin1(), m_state.port)) {
        error( ERR_UNKNOWN_HOST, m_state.hostname );
        return false;
      }

      infoMessage( i18n("Connecting to %1...").arg(m_state.hostname) );

      if(::connect(m_sock, (struct sockaddr*)( &server_name ), sizeof(server_name))) {
        if((errno != EINPROGRESS) && (errno != EWOULDBLOCK)) {
          // Error
          error(ERR_COULD_NOT_CONNECT, m_state.hostname );
          return false;
        }
        // Wait for connection
        if (!waitForConnect(m_sock, REMOTE_CONNECT_TIMEOUT))
        {
          error(ERR_COULD_NOT_CONNECT, m_state.hostname );
          return false;
        }
      }
#ifdef DO_SSL
        m_ssl_ip = inet_ntoa(server_name.sin_addr);
#endif

    }

    // Set socket blocking.
    fcntl(m_sock, F_SETFL, ( fcntl(m_sock, F_GETFL) & ~O_NDELAY));

    // Placeholder
    if (!openStream())
    {
      error( ERR_COULD_NOT_CONNECT, m_state.hostname );
      return false;
    }

    kdDebug() << time(0L) << " Sending connected" << endl;
    // Tell the application that we are connected, and that the metadata (e.g. ssl) is ready
    connected();

    return true;
}

bool HTTPProtocol::checkSSL()
{
  // Check if we need to pop up a dialog box to the user
  if ( metaData( "ssl_activate_warnings" ) == "TRUE" )
  {
    kdDebug() << "SSL warnings activated" << endl;
    bool ssl_was_in_use = metaData( "ssl_was_in_use" ) == "TRUE";
    kdDebug() << "ssl_was_in_use: " << ssl_was_in_use << endl;

#ifdef DO_SSL
    kdDebug() << "m_bUseSSL: " << m_bUseSSL << endl;
    kdDebug() << "warnOnEnter: " << m_ssl.settings()->warnOnEnter() << endl;
    kdDebug() << "warnOnUnencrypted: " << m_ssl.settings()->warnOnUnencrypted() << endl;
    kdDebug() << "warnOnLeave: " << m_ssl.settings()->warnOnLeave() << endl;
    kdDebug() << "warnOnMixed: " << m_ssl.settings()->warnOnMixed() << endl;

    if ( !ssl_was_in_use && m_bUseSSL && m_ssl.settings()->warnOnEnter() )
    {
      kdDebug() << "ENTERING SSL" << endl;
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
      kdDebug() << "LEAVING SSL" << endl;
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
  m_bCachedRead = false;
  m_bCachedWrite = false;
  m_bMustRevalidate = false;
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
  m_sContentMD5 = "";
  m_HTTPrev = HTTP_Unknown;
  m_qContentEncodings.clear();
  m_qTransferEncodings.clear();
  m_bChunked = false;
  m_iSize = -1;

  // let's try to open up our socket if we don't have one already.
  if (!m_sock)
  {
    if (!http_openConnection())
       return false;
  }

  // this will be the entire header
  QString header;

  bool moreData = false;

  // determine if this is a POST or GET method
  switch( m_request.method) {
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
  char c_buffer[64];
  memset(c_buffer, 0, 64);
  if(m_state.do_proxy) {
    sprintf(c_buffer, ":%u", m_state.port);
    // The URL for the request uses ftp:// if we are in "ftp-proxy" mode
    kdDebug() << "Using basis for URL : " << m_protocol << endl;
    header += (m_protocol == "ftp") ? "ftp://" : "http://";
    header += m_state.hostname;
    header += c_buffer;
  }
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
  QString agent = metaData("userAgent");
  // If application did not specify a user agent,
  // use configured by the user or the default!!
  if( agent.isEmpty() )
    agent = KProtocolManager::userAgentForHost( m_state.hostname );

  if( !agent.isEmpty() )
  {
    header += "User-Agent: " + agent;
#ifdef DO_MD5
    header+="; Supports MD5-Digest";
#endif
#ifdef DO_GZIP
    header+="; Supports gzip encoding";
#endif
#ifdef HAVE_SSL
    header+="; Supports SSL/HTTPS";
#endif
    header += "\r\n";
  }

  QString referrer = metaData("referrer");
  if (!referrer.isEmpty())
  {
     // HTTP uses "Referer" although the correct spelling is "referrer"
     header += "Referer: "+referrer+"\r\n";
  }

  QString acceptHeader = metaData("accept");
  if (!acceptHeader.isEmpty())
  {
     header += "Accept: "+acceptHeader+"\r\n";
  }

  if ( m_request.offset > 0 ) {
    sprintf(c_buffer, "Range: bytes=%li-\r\n", m_request.offset);
    header += c_buffer;
    kdDebug(7113) << "kio_http : Range = " << c_buffer << endl;
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

#ifdef DO_GZIP
  // Content negotiation
  header += "Accept-Encoding: x-gzip; q=1.0, x-deflate, gzip; q=1.0, deflate, identity\r\n";
#endif

  // Charset negotiation:
  if ( !m_strCharsets.isEmpty() )
    header += "Accept-Charset: " + m_strCharsets + "\r\n";

  // Language negotiation:
  if ( !m_strLanguages.isEmpty() )
    header += "Accept-Language: " + m_strLanguages + "\r\n";

  header += "Host: "; /* support for virtual hosts and required by HTTP 1.1 */
  header += m_state.hostname;
  if (m_state.port != mDefaultPort) {
    memset(c_buffer, 0, 64);
    sprintf(c_buffer, ":%u", m_state.port);
    header += c_buffer;
  }

  header += "\r\n";

  QString cookieStr = findCookies( m_request.url.url());
  if (!cookieStr.isEmpty())
  {
    header += cookieStr + "\r\n";
  }

  if (m_request.method == HTTP_POST) {
      header += metaData("content-type");
      header += "\r\n";
  }

  // Check the cache if it is not a re-authentication request.
  if( m_iAuthFailed == 0 )
  {
    kdDebug(7113) << "(" << getpid() << ")  Checking for Authentication info in cache..." << endl;
    QString user, passwd, realm, extra;
    if( checkCachedAuthentication( m_request.url, user, passwd, realm, extra, true ) )
    {
      kdDebug(7103) << "(" << getpid() << ") Found a matching Authentication entry..." << endl;
      Authentication = extra.isEmpty() ? AUTH_Basic : AUTH_Digest ;
      m_state.user   = user;
      m_state.passwd = passwd;
      m_strRealm = realm;
      if( Authentication == AUTH_Digest )
        m_strAuthString = extra;
    }
  }

  // Check if we need to send "Authentication" is required
  if( Authentication == AUTH_Basic )
  {
    kdDebug(7113) << "Using authentication: " << endl
                  << " User= " << m_state.user << endl
                  << " Password= [protected]" << endl
                  << " Realm= " << m_strRealm << endl
                  << " Extra= " << m_strAuthString << endl;

    header += create_basic_auth("Authorization", m_state.user.latin1(), m_state.passwd.latin1());
    header+="\r\n";
  }
  else if( Authentication == AUTH_Digest )
  {
    kdDebug(7113) << "Using authentication: " << endl
                  << " User= " << m_state.user << endl
                  << " Password= [protected]" << endl
                  << " Realm= " << m_strRealm << endl
                  << " Extra= " << m_strAuthString  << endl;

    header += create_digest_auth("Authorization", m_state.user.latin1(), m_state.passwd.latin1(),
                                 m_strAuthString.latin1());
    header+="\r\n";
  }

  // Do we need to authorize to the proxy server ?
  if( m_state.do_proxy )
  {
    // We keep proxy authentication locally until they are
    // changed.  Thus, no need to check with kdesud for it
    // when they are available...
    kdDebug(7113) << "Proxy Setting HOST: " << m_proxyURL.host() << endl
                  << "Proxy Setting PORT: " << m_proxyURL.port() << endl
                  << "Proxy Setting USER: " << m_proxyURL.user() << endl
                  << "Proxy Setting PASS: [protected]" << endl
                  << "Proxy Setting REALM: " << m_strProxyRealm << endl
                  << "Proxy Setting AUTH_TYPE: " << ProxyAuthentication << endl
                  << "Proxy Setting AUTH_STRING: " << m_strProxyAuthString << endl;

    if( m_strProxyRealm.isEmpty() )
    {
      kdDebug(7113) << "Checking for Proxy Authentication..." << endl;
      QString user, passwd, realm, extra;
      bool isCached = checkCachedAuthentication( m_proxyURL, user, passwd, realm, extra, false );
      if( isCached )
      {
        m_proxyURL.setUser( user );
        m_proxyURL.setPass( passwd );
        m_strProxyRealm = realm;
        if( !extra.isEmpty() )
        {
          ProxyAuthentication = AUTH_Digest;
          m_strProxyAuthString = extra;
        }
        else
        {
          ProxyAuthentication = AUTH_Basic;
        }
        kdDebug(7113) << "Request URL matches protection space. Retreived cached authentication: " << endl
                      << " User= " << user << endl
                      << " Pasword= [protected]" << endl
                      << " Realm= " << realm << endl
                      << " Extra= " << extra << endl;
      }
      else
      {
          ProxyAuthentication = AUTH_None;
      }
    }

    if( ProxyAuthentication == AUTH_Basic )
    {
      header+= create_basic_auth("Proxy-Authorization", m_proxyURL.user().latin1(), m_proxyURL.pass().latin1() );
      header+="\r\n";
    }
    else if( ProxyAuthentication == AUTH_Digest )
    {
      header+= create_digest_auth( "Proxy-Authorization", m_proxyURL.user().latin1(),
                                   m_proxyURL.pass().latin1(), m_strProxyAuthString.latin1());
      header+="\r\n";
    }
  }

  if( !moreData )
    header += "\r\n";  /* end header */

  kdDebug(7113) << "(" << getpid() << ") Sending header: \n=======" << endl << header << "\n=======" << endl;

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
       error( ERR_CONNECTION_BROKEN, m_state.hostname );
       return false;
    }
  }

  bool res = true;

  if (moreData)
     res = sendBody();

  infoMessage( i18n( "%1 contacted. Waiting for reply..." ).arg( m_request.hostname ) );

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
        error( ERR_CONNECTION_BROKEN, m_state.hostname );
        return false;
     }
     kdDebug(7113) << "readHeader: returning mimetype " << buffer << endl;
     m_strMimeType = QString::fromUtf8( buffer).stripWhiteSpace();
     mimeType(m_strMimeType);
     // TODO save charset in cache and set it here ?
     return true;
  }

  // to get rid of those "Open with" dialogs...
  m_strMimeType = "text/html";
  m_etag = QString::null;
  m_lastModified = QString::null;

  time_t dateHeader = 0;
  time_t expireDate = 0; // 0 = no info, 1 = already expired, > 1 = actual date
  int currentAge = 0;
  int maxAge = -1; // -1 = no max age, 0 already expired, > 0 = actual time

  QCString locationStr; // In case we get a redirect.
  QCString cookieStr; // In case we get a cookie.
  QString charsetStr;

  // read in 4096 bytes at a time (HTTP cookies can be quite large.)
  int len = 0;
  int code;
  char buffer[4097];
  bool unauthorized = false;
  bool cont = false;
  bool noRedirect = false; // No automatic redirection
  bool cacheValidated = false; // Revalidation was successfull
  bool mayCache = true;

  if (!waitForHeader(m_sock, RESPONSE_TIMEOUT))
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
        if (!waitForHeader(m_sock, RESPONSE_TIMEOUT))
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
           mimeType(QString::fromLatin1("text/html"));
           return true;
        }
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
    }

    // get the size of our data
    else if (strncasecmp(buffer, "Content-length:", 15) == 0) {
      m_iSize = atol(trimLead(buffer + 15));
    }

    // what type of data do we have?
    else if (strncasecmp(buffer, "Content-type:", 13) == 0) {
      // Jacek: We can't send mimeType signal now,
      // because there may be another Content-Type. Or even
      // worse the entity-body is encoded and there is a
      // Content-Encoding specified which would then contain
      // the true mime-type for the requested URI i.e. the content
      // type is only applicable to the actual message-body!!
      m_strMimeType = trimLead(buffer + 13);

      // This header can be something like "text/html; charset foo-blah"
      int semicolonPos = m_strMimeType.find( ';' );
      if ( semicolonPos != -1 )
      {
        int equalPos = m_strMimeType.find( '=' );
        if ( equalPos != -1 )
        {
          charsetStr = m_strMimeType.mid( equalPos+1 );
          kdDebug(7103) << "Found charset : " << charsetStr << endl;
        }
        m_strMimeType = m_strMimeType.left( semicolonPos );
      }
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
      m_lastModified = trimLead(buffer+14);
    }

    // whoops.. we received a warning
    else if (strncasecmp(buffer, "Warning:", 8) == 0) {
      //Don't use warning() here, no need to bother the user.
      //Those warnings are mostly about caches.
      infoMessage(trimLead(buffer + 8));
    }

    // Cache management (HTTP 1.0)
    else if (strncasecmp(buffer, "Pragma:", 7) == 0) {
      QCString pragma = QCString(trimLead(buffer+7)).lower();
      if (pragma == "no-cache")
      {
         m_bCachedWrite = false; // Don't put in cache
         mayCache = false;
      }
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
         Authentication = AUTH_None;
#ifdef DO_SSL
        // Don't do persistant connections with SSL.
        if (!m_bUseSSL)
           m_bKeepAlive = true; // HTTP 1.1 has persistant connections.
#else
           m_bKeepAlive = true; // HTTP 1.1 has persistant connections by default.
#endif
      }
      code = atoi(buffer+9);

      // unauthorized access
      if ((code == 401) || (code == 407)) {
    	unauthorized = true;
    	m_iAuthFailed++;
        m_bCachedWrite = false; // Don't put in cache
        mayCache = false;
      }
      // server side errors
      else if ((code >= 500) && (code <= 599)) {
        if (m_request.method == HTTP_HEAD) {
           // Ignore error
        } else {
           errorPage();
        }
        m_bCachedWrite = false; // Don't put in cache
        mayCache = false;
      }
      // client errors
      else if ((code >= 400) && (code <= 499)) {
#if 0
#warning To be fixed! error terminates the job!
        // Let's first send an error message
        // this will be moved to slotErrorPage(), when it will be written
        error(ERR_ACCESS_DENIED, m_state.hostname);
#endif
        // Tell that we will only get an error page here.
        errorPage();
        m_bCachedWrite = false; // Don't put in cache
        mayCache = false;
      }
      else if (code == 100)
      {
        // We got 'Continue' - ignore it
        cont = true;
      }
      else if (code == 304)
      {
        // 304 Not Modified
        // The value in our cache is still valid.
        cacheValidated = true;
      }
      else if ((code == 301) || (code == 307))
      {
        // 301 Moved permanently
        // 307 Temporary Redirect
        if (m_request.method == HTTP_POST)
        {
           errorPage();
           m_bCachedWrite = false; // Don't put in cache
           mayCache = false;
           noRedirect = true;
        }
      }
      else if ((code == 302) || (code == 303))
      {
        // 302 Found
        // 303 See Other
        if (m_request.method != HTTP_HEAD)
        {
           m_request.method = HTTP_GET; // Force a GET!
           m_bCachedWrite = false; // Don't put in cache
           mayCache = false;
        }
      }
    }
    // In fact we should do redirection only if we got redirection code
    else if (strncasecmp(buffer, "Location:", 9) == 0 ) {
      locationStr = trimLead(buffer+9);
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
    }

    // clear out our buffer for further use
    memset(buffer, 0, sizeof(buffer));
  }
  while(len && (gets(buffer, sizeof(buffer)-1)));

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
     // Give cookies to the cookiejar.
     addCookies( m_request.url.url(), cookieStr );
  }

  if (m_bMustRevalidate)
  {
     m_bMustRevalidate = false; // Reset just in case.
     if (cacheValidated)
     {
       // Yippie, we can use the cached version.
       // TODO: Update the cache with new "Expire" headers.
       // If we don't, we need to revalidate each and every time
       // after this request.
       if (expireDate > 10 + m_expireDate)
       {
          fclose(m_fcache);
          m_fcache = 0;
          updateExpireDate( expireDate );
          m_fcache = checkCacheEntry( ); // Re-read cache entry
       }
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
  else if( unauthorized )
  {
    http_closeConnection();  // Close the connection first
    QString msg, user, passwd;

    if( m_iAuthFailed > 1 || (!user.isEmpty() && !passwd.isEmpty()) )
    {
      if( code == 401 )
        msg = i18n( "Authentication Failed!" );
      else if( code == 407 )
        msg = i18n( "Proxy Authentication Failed!");

      msg += i18n("  Do you want to retry ?");

      if( messageBox(QuestionYesNo, msg, i18n("Authentication")) != 3  )
      {
        kdDebug(7103) << "User rejected Authentication retry!!" << endl;
        /*
        if( code == 401 )
            msg = QString("%1").arg(m_state.hostname);
        else if( code == 407 )
            msg = QString("%1").arg(m_proxyURL.host());
        error(ERR_ACCESS_DENIED, msg);
        */
        error(ERR_USER_CANCELED, QString::null);
        return false;
      }
    }
    if( code == 401 )
    {
      // For cases where the user does http://foo@www.foo.org
      if( !m_request.user.isEmpty() && m_iAuthFailed == 1 )
        user = m_request.user;

      if( m_strRealm.isEmpty() )
        m_strRealm = m_state.hostname;

      msg = i18n( "<center>Authentication required to access<br/>"
                    "<b>%1</b> at <b>%2</b></center>" ).arg( m_strRealm ).arg( m_request.hostname );
    }
    else if( code == 407 )
    {
      if( m_strProxyRealm.isEmpty() )
        m_strProxyRealm = m_state.hostname;

      msg = i18n( "<b>Proxy Authentication</b> required to access this site.<br/>"
                  "Enter your Authentication information below:" );
    }

    kdDebug(7103) << "(" << getpid() << ") Request URI's user: "<< m_request.url.user() << endl;
    bool result = openPassDlg( msg, user, passwd, (!m_request.user.isEmpty() && m_iAuthFailed == 1) );
    if( result )
    {
      // Note that a single io-slave cannot be doing
      // both authentications at the same time sine
      // these requests come sequentially...
      if( code == 401 )  // Request-Authentication
      {
        m_request.user = user;
        m_request.passwd = passwd;
      }
      else if( code == 407 )  // Proxy-Authentication
      {
        m_proxyURL.setUser( user );
        m_proxyURL.setPass( passwd );
      }
    }
    else
    {
      error( ERR_USER_CANCELED, "" ); // Ignore the user then!!
      return false;
    }

    if( !http_open() )
        return false;

    result = readHeader();
    m_iAuthFailed--;
    if( result && m_iAuthFailed == 0 )
    {
        if( code == 401 )
        {
          kdDebug(7103) << "(" << getpid() << ") Caching request Authentication..." << endl;
          cacheAuthentication( m_request.url, m_request.user, m_request.passwd, m_strRealm, m_strAuthString );
        }
        else if( code == 407 )
        {
          kdDebug(7103) << "(" << getpid() << ") Caching Proxy Authentication..." << endl;
          cacheAuthentication( m_proxyURL, m_proxyURL.user(), m_proxyURL.pass(), m_strProxyRealm, m_strProxyAuthString );
        }
    }
    return result;
  }
  // We need to do a redirect
  else if (!locationStr.isEmpty() && !noRedirect)
  {
    kdDebug(7113) << "request.url: " << m_request.url.url() << endl
                  << "LocationStr: " << locationStr.data() << endl;

    KURL u(m_request.url, locationStr);
    if(u.isMalformed())
    {
      error(ERR_MALFORMED_URL, u.url());
      return false;
    }

    redirection(u.url());
    m_bCachedWrite = false; // Turn off caching on re-direction (DA)
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
     else if (m_strMimeType == "text/html")
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

  // Prefer application/x-tgz over application/x-gzip
  if (m_strMimeType == "application/x-gzip")
  {
     if ((m_request.url.path().right(7) == ".tar.gz") ||
         (m_request.url.path().right(4) == ".tar"))
        m_strMimeType = QString::fromLatin1("application/x-tgz");
  }

  // Some webservers say "text/plain" when they mean "application/x-bzip2"
  if (m_strMimeType == "text/plain")
  {
     if (m_request.url.path().right(4) == ".bz2")
        m_strMimeType = QString::fromLatin1("application/x-bzip2");
  }

  if (!m_qContentEncodings.isEmpty())
  {
     // If we still have content encoding we can't rely on the Content-Length.
     m_iSize = -1;
  }

  // FINALLY, let the world know what kind of data we are getting
  // and that we do indeed have a header
  // Do this only if there is no redirection. Buggy server implementations
  // incorrectly send Content-Type with a redirection response. (DA)
  if( locationStr.isEmpty() )
     mimeType(m_strMimeType);

  // Set charset. Maybe charSet should be a class member, since
  // this method is somewhat recursive....
  if ( !charsetStr.isEmpty() )
  {
     kdDebug(7103) << "Setting charset metadata to " << charsetStr << endl;
     setMetaData("charset", charsetStr);
  }

  if (m_request.method == HTTP_HEAD)
     return true;

  if (!m_lastModified.isEmpty())
     setMetaData("modified", m_lastModified);

  if (!mayCache)
     setMetaData("no-cache", "true");

  // Do we want to cache this request?
  if (m_bCachedWrite)
  {
     // Check...
     createCacheEntry(m_strMimeType, expireDate); // Create a cache entry
     if (!m_fcache)
        m_bCachedWrite = false; // Error creating cache entry.
  }

  if (m_bCachedWrite)
    kdDebug(7113) << "Cache, adding \"" << m_request.url.url() << "\"" << endl;
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
    kdDebug(7103) << "Unknown encoding encountered.  Please write code. Pid = " << getpid() << " Encoding = \"" << encoding << "\"" << endl;
    abort();
  }
}

void HTTPProtocol::configAuth(const char *p, bool b)
{
  HTTP_AUTH f;
  char * strAuth=0, *assign=0;
  int i;

  while( *p == ' ' ) p++;
  if ( strncasecmp( p, "Basic", 5 ) == 0 ) {
    kdDebug(7103) << "Basic Authentication" << endl;
    f = AUTH_Basic;
    p += 5;
  } else if (strncasecmp (p, "Digest", 6) ==0 ) {
    kdDebug(7103) << "Digest Authentication" << endl;
    p += 6;
    f = AUTH_Digest;
    strAuth = strdup(p);
  } else if (strncasecmp (p, "NTLM", 4) == 0) {
    // NT Authentification sheme. not yet implemented
    // we try to ignore it. maybe we return later here
    // with a Basic or Digest authentification request
    // and then it should be okay.
    // i.e. NT IIS sends
    // WWW-Authentification: NTLM\r\n
    // WWW-Authentification: Basic\r\n
    return;
  } else {
    kdDebug(7103) << "Invalid Authorization type requested" << endl;
    kdDebug(7103) << "buffer: " << p << endl;
    error(ERR_UNSUPPORTED_ACTION,
          QCString().sprintf("Unknown Authorization method: %s", p).data());
    return;
  }

  while (*p) {
    while( (*p == ' ') || (*p == ',') || (*p == '\t'))
      p++;
    i=0;
    if ( strncasecmp( p, "realm=\"", 7 ) == 0 ) {
      p += 7;
      while( p[i] != '"' ) i++;
      assign=(char *)malloc(i+1);
      memcpy((void *)assign, (const void *)p, i);
      assign[i]=0;
      if( b )
        m_strProxyRealm = assign;
      else
        m_strRealm = assign;
      free(assign);
    }
    p+=i;
    p++;
  }
  if (b) {
    ProxyAuthentication=f;
    m_strProxyAuthString = strAuth;
  } else {
    Authentication=f;
    m_strAuthString = strAuth;
  }
}

bool HTTPProtocol::sendBody()
{
  QList<QByteArray> bufferList;
  int length = 0;

  int result;
  // Loop until we got 'dataEnd'
  do
  {
     QByteArray *buffer = new QByteArray();
     dataReq(); // Request for data
     result = readData( *buffer );
     if (result > 0)
     {
        bufferList.append(buffer);
        length += result;
     }
   }
   while ( result > 0 );

   if ( result != 0)
   {
     error( ERR_ABORTED, m_request.hostname );
     return false;
   }

   char c_buffer[64];
   sprintf(c_buffer, "Content-Length: %d\r\n\r\n", length);
   kdDebug( 7113 ) << "POST: " << c_buffer << endl;

   bool sendOk;
   sendOk = (write(c_buffer, strlen(c_buffer)) == (ssize_t) strlen(c_buffer));

   if (!sendOk)
   {
     kdDebug(7103) << "Connection broken (sendBody(1))! (" << m_state.hostname << ")" << endl;
     error( ERR_CONNECTION_BROKEN, m_state.hostname );
     return false;
   }

   QByteArray *buffer;
   while ( !bufferList.isEmpty() )
   {
     buffer = bufferList.take(0);

     sendOk = (write(buffer->data(), buffer->size()) == (ssize_t) buffer->size());
     delete buffer;
     if (!sendOk)
     {
       kdDebug(7103) << "Connection broken (sendBody(2))! (" << m_state.hostname << ")" << endl;
       error( ERR_CONNECTION_BROKEN, m_state.hostname );
       return false;
     }
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
        unlink( filename.latin1());
     }
  }
  if (!m_bKeepAlive)
     http_closeConnection();
  else
     kdDebug(7113) << "http_close: keep alive" << endl;
}

void HTTPProtocol::http_closeConnection()
{
  kdDebug(7113) << "http_closeConnection: closing (" << getpid() << ")" << endl;
  m_bKeepAlive = false; // Just in case.
  if ( m_fsocket )
    fclose( m_fsocket );
  m_fsocket = 0;
  if ( m_sock )
    ::close( m_sock );
  m_sock = 0;
#ifdef DO_SSL
  closeSSL();
#endif
}

void HTTPProtocol::setHost(const QString& host, int port, const QString& user, const QString& pass)
{
  m_request.hostname = host;

  // try to ensure that the port is something reasonable
  if ( port == 0 )
     port = mDefaultPort;

  m_request.port = port;
  m_request.user = user;
  m_request.passwd = pass;
  m_request.do_proxy = m_bUseProxy;
}

void HTTPProtocol::slave_status()
{
  bool connected = (m_sock != 0);
  if (connected && !http_isConnected())
  {
     http_closeConnection();
     connected = false;
  }
  slaveStatus( m_state.hostname, connected );
}

void HTTPProtocol::buildURL()
{
  m_request.url = QString::fromLatin1(m_protocol+":/");
  m_request.url.setUser( m_request.user );
  m_request.url.setPass( m_request.passwd );
  m_request.url.setHost( m_request.hostname );
  m_request.url.setPort( m_request.port );
  m_request.url.setPath( m_request.path );
  if (m_request.query.length())
    m_request.url.setQuery( m_request.query );
}

static HTTPProtocol::CacheControl parseCacheControl(const QString &cacheControl)
{
  HTTPProtocol::CacheControl _default = HTTPProtocol::CC_Verify;
  if (cacheControl.isEmpty())
     return _default;

  QString tmp = cacheControl.lower();
  if (tmp == "cacheonly")
     return HTTPProtocol::CC_CacheOnly;
  if (tmp == "cache")
     return HTTPProtocol::CC_Cache;
  if (tmp == "verify")
     return HTTPProtocol::CC_Verify;
  if (tmp == "reload")
     return HTTPProtocol::CC_Reload;

  return _default;
}

// Returns only the file size, that's all kio_http can guess.
void HTTPProtocol::stat(const KURL& url)
{
  kdDebug() << "HTTPProtocol::stat " << url.prettyURL() << endl;
  if (m_request.hostname.isEmpty())
  {
     error( KIO::ERR_UNKNOWN_HOST, i18n("No host specified!"));
     return;
  }

  m_request.method = HTTP_HEAD;
  m_request.path = url.path();
  m_request.query = url.query();
  m_request.cache = parseCacheControl(metaData("cache"));
  m_request.offset = 0;
  m_request.do_proxy = m_bUseProxy;
  m_request.url = url;

  if (!retrieveHeader( false ))
    return;

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

  if ( m_iSize != -1 )
  {
    atom.m_uds = KIO::UDS_SIZE;
    atom.m_long = m_iSize;
    entry.append( atom );
  }

  statEntry( entry );

  http_close();
  finished();
}

void HTTPProtocol::get( const KURL& url )
{
  if (m_request.hostname.isEmpty())
  {
     error( KIO::ERR_UNKNOWN_HOST, i18n("No host specified!"));
     return;
  }

  m_request.method = HTTP_GET;
  m_request.path = url.path();
  m_request.query = url.query();
  m_request.cache = parseCacheControl(metaData("cache"));

  m_request.offset = 0;
  m_request.do_proxy = m_bUseProxy;
  m_request.url = url;
  m_request.passwd = url.pass();
  m_request.user = url.user();

  retrieveContent( true /* SSL checks */ );
}

void HTTPProtocol::put( const KURL &url, int, bool, bool)
{
  if (m_request.hostname.isEmpty())
  {
     error( KIO::ERR_UNKNOWN_HOST, i18n("No host specified!"));
     return;
  }

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
  if (m_request.hostname.isEmpty())
  {
     error( KIO::ERR_UNKNOWN_HOST, i18n("No host specified!"));
     return;
  }

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
  kdDebug() << "HTTPProtocol::mimetype " << url.prettyURL() << endl;
  if (m_request.hostname.isEmpty())
  {
     error( KIO::ERR_UNKNOWN_HOST, i18n("No host specified!"));
     return;
  }

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
      stream >> url; // >> m_request.user_headers; //DA: replaced by a meta-data labeled"content-type"...
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

void HTTPProtocol::decodeDeflate()
{
#ifdef DO_GZIP
  // Okay the code below can probably be replaced with
  // the a single call to decompress(...) instead of a read/write
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
  // to HD cannot be good :))  What if the target is full buddy or could
  // not be written to for some reason ??
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
    if( count )
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
  m_iBytesLeft = 0; // Assume failure

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
  kdDebug(7113) << "Chunk header = \"" << m_bufReceive.data() << "\"" << endl;
  if (eof())
  {
     kdDebug(7103) << "EOF on Chunk header" << endl;
     return -1;
  }

  int chunkSize = strtol(m_bufReceive.data(), 0, 16);
  if ((chunkSize < 0) || (chunkSize > MAX_CHUNK_SIZE))
     return -1;

  kdDebug(7113) << "Chunk size = " << chunkSize << " bytes" << endl;

  if (chunkSize == 0)
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

  if (chunkSize > (int) m_bufReceive.size())
  {
     if (!m_bufReceive.resize(chunkSize))
        return -1; // Failure
  }

  int totalBytesReceived = 0;
  int bytesToReceive = chunkSize;

  do
  {
     if (eof()) return -1; // Unexpected EOF.

     int bytesReceived = read( m_bufReceive.data()+totalBytesReceived, bytesToReceive );

     kdDebug(7113) << "Read from chunk got " << bytesReceived << " bytes" << endl;

     if (bytesReceived == -1)
        return -1; // Failure.

     totalBytesReceived += bytesReceived;
     bytesToReceive -= bytesReceived;

     kdDebug(7113) << "Chunk has " << totalBytesReceived << " bytes, " << bytesToReceive << " bytes to go" << endl;
  }
  while(bytesToReceive > 0);

  m_iBytesLeft = 1; // More to come.
  return totalBytesReceived; // This is what we got.
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
  // If we are in copy mode the use only transfer decoding.
  bool decode = !m_qTransferEncodings.isEmpty() || !m_qContentEncodings.isEmpty();

#ifdef DO_MD5
  bool useMD5 = !m_sContentMD5.isEmpty();
#endif

  totalSize( (m_iSize > -1) ? m_iSize : 0 );

  infoMessage( i18n( "Retrieving data from %1" ).arg( m_request.hostname ) );

  // get the starting time.  this is used later to compute the transfer
  // speed.
  time_t t_start = time(0L);
  time_t t_last = t_start;
  long sz = 0;

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

#ifdef DO_MD5
  Local_MD5_CTX context;
  MD5Init(&context);
#endif
  if (m_iSize > -1)
    m_iBytesLeft = m_iSize;
  else
    m_iBytesLeft = 1;

  kdDebug() << "HTTPProtocol::readBody m_iBytesLeft=" << m_iBytesLeft << endl;

  // this is the main incoming loop.  gather everything while we can...
  big_buffer.resize(0);
  while (!eof()) {
    int bytesReceived;
    if (m_bChunked)
       bytesReceived = readChunked();
    else if (m_iSize > -1)
       bytesReceived = readLimited();
    else {
       bytesReceived = readUnlimited();
    }

    // make sure that this wasn't an error, first
    if (bytesReceived == -1) {
      // erg.  oh well, log an error and bug out
      error(ERR_CONNECTION_BROKEN, m_state.hostname);
      return false;
    }

    // i guess that nbytes == 0 isn't an error.. but we certainly
    // won't work with it!
    if (bytesReceived > 0) {
      // check on the encoding.  can we get away with it as is?
      if ( !decode ) {
#ifdef DO_MD5
        if (useMD5) {
          MD5Update(&context, (const unsigned char*)m_bufReceive.data(), bytesReceived);
        }
#endif
        // yep, let the world know that we have some data
        array.setRawData( m_bufReceive.data(), bytesReceived );
        data( array );
        array.resetRawData( m_bufReceive.data(), bytesReceived );

        if (m_bCachedWrite && m_fcache)
           writeCacheEntry(m_bufReceive.data(), bytesReceived);
        sz += bytesReceived;
        processedSize( sz );
        time_t t = time( 0L );
        if ( t - t_last >= 1 ) {
          speed( sz / ( t - t_start ) );
          t_last = t;
        }
      } else {
        // nope.  slap this all onto the end of a big buffer
        // for later use
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
  if (!big_buffer.isNull()) {
    // decode all of the transfer encodings
    while (!m_qTransferEncodings.isEmpty()) {
      QString enc = m_qTransferEncodings.last();
      m_qTransferEncodings.remove(m_qTransferEncodings.fromLast());
      if ( enc == "gzip" ) {
        decodeGzip();
      }
      else if( enc == "deflate" ) {
        decodeDeflate();
      }
    }

    // From HTTP 1.1 Draft 6:
    // The MD5 digest is computed based on the content of the entity-body,
    // including any content-coding that has been applied, but not including
    // any transfer-encoding applied to the message-body. If the message is
    // received with a transfer-encoding, that encoding MUST be removed
    // prior to checking the Content-MD5 value against the received entity.
#ifdef DO_MD5
    MD5Update(&context, (const unsigned char*)big_buffer.data(),
               big_buffer.size());
#endif

    // now decode all of the content encodings
    // -- Why ?? We are not
    // -- a proxy server, be a client side implementation!!  The applications
    // -- are capable of determinig how to extract the encoded implementation.
    // WB: That's a misunderstanding. We are free to remove the encoding.
    // WB: Some braindead www-servers however, give .tgz files an encoding
    // WB: of "gzip" (or even "x-gzip") and a content-type of "applications/tar"
    // WB: They shouldn't do that. We can work around that though...
    while (!m_qContentEncodings.isEmpty()) {
      QString enc = m_qContentEncodings.last();
      m_qContentEncodings.remove(m_qContentEncodings.fromLast());
      if ( enc == "gzip" ) {
        decodeGzip();
      }
      else if( enc == "deflate" ) {
        decodeDeflate();
      }
    }
    sz = sendData();
  }

  // this block is all final MD5 stuff
#ifdef DO_MD5
  char buf[16], *enc_digest;
  MD5Final((unsigned char*)buf, &context); // Wrap everything up
  enc_digest = base64_encode_string(buf, 16);
  if ( useMD5 ) {
    int f;
    if ((f = m_sContentMD5.find("=")) <= 0) {
      f = m_sContentMD5.length();
    }

    if (m_sContentMD5.left(f) != enc_digest) {
      kdDebug(7103) << "MD5 checksum mismatch : got " << m_sContentMD5.left(f) << " , calculated " << enc_digest << endl;
    } else {
      kdDebug(7103) << "MD5 checksum present, and hey it matched what I calculated." << endl;
    }
  } else {
    kdDebug(7103) << "No MD5 checksum found.  Too Bad." << endl;
  }

  fflush(stderr);
  free(enc_digest);
#endif

  // Close cache entry
  if (m_iBytesLeft == 0)
  {
     if (m_bCachedWrite && m_fcache)
        closeCacheEntry();
  }

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

void HTTPProtocol::error( int _err, const QString &_text )
{
  m_bKeepAlive = false;
  http_close();
  SlaveBase::error( _err, _text );
}

void
HTTPProtocol::addCookies( const QString &url, const QCString &cookieHeader )
{
   long windowId = m_request.window.toLong();
   QByteArray params;
   QDataStream stream(params, IO_WriteOnly);
   stream << url << cookieHeader << windowId;
   if (!m_dcopClient->send("kcookiejar", "kcookiejar",
        "addCookies(QString, QCString, long)", params))
   {
      kdWarning(7103) << "Can't communicate with cookiejar!" << endl;
   }
}

QString
HTTPProtocol::findCookies( const QString &url)
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

#define CACHE_REVISION "5\n"

FILE *
HTTPProtocol::checkCacheEntry( bool readWrite)
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
         ok = false; // Expired
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

void
HTTPProtocol::updateExpireDate(time_t expireDate)
{
   FILE *fs = checkCacheEntry(true);
   if (fs)
   {
      int result = fseek(fs, m_cacheExpireDateOffset, SEEK_SET);
      QString date;
      date.setNum( expireDate );
      date = date.leftJustify(16);
      fputs(date.latin1(), fs);      // Expire date
      result = fseek(fs, 0, SEEK_END);
      fclose(fs);
   }
}

void
HTTPProtocol::createCacheEntry( const QString &mimetype, time_t expireDate)
{
   QString dir = m_state.cef;
   int p = dir.findRev('/');
   if (p == -1) return; // Error.
   dir.truncate(p);

   // Create file
   (void) ::mkdir( dir.latin1(), 0700 );

   QString filename = m_state.cef + ".new";  // Create a new cache entry

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

   return;
}
// The above code should be kept in sync
// with the code in http_cache_cleaner.cpp
// !END SYNC!

void
HTTPProtocol::writeCacheEntry( const char *buffer, int nbytes)
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

void
HTTPProtocol::closeCacheEntry()
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

void
HTTPProtocol::cleanCache()
{
   const time_t maxAge = 30*60; // 30 Minutes.
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

void HTTPProtocol::reparseConfiguration()
{
  kdDebug( 7103 ) << "reparseConfiguration!" << endl;
  m_bUseProxy = false;

  if ( KProtocolManager::useProxy() )
  {
    // Use the appropriate proxy depending on the protocol
    m_proxyURL = KURL( KProtocolManager::proxyFor( m_protocol ) );
    kdDebug() << "m_proxyURL=" << KProtocolManager::proxyFor( m_protocol ) << endl;
    if (!m_proxyURL.isMalformed() )
    {
        // Set "use proxy" to true if we got a non empty proxy URL
        m_bUseProxy = true;
        m_strNoProxyFor = KProtocolManager::noProxyFor();
        // Reset the current set-up since the proxy info changed
        // If the proxy needs authentication, it would reject the
        // request with a 407 where this info will then be setup
        // properly.
        m_strProxyRealm = QString::null;
        m_strProxyAuthString = QString::null;
        ProxyAuthentication = AUTH_None;
        kdDebug(7103) << "Using Proxy HOST: " << m_proxyURL.host() << endl
                      << "Using Proxy PORT: " << m_proxyURL.port() << endl
                      << "Using Proxy REALM: " << m_strProxyRealm << endl
                      << "Using Proxy AUTH_STRING: " << m_strProxyAuthString << endl
                      << "No Proxy for: " << m_strNoProxyFor << endl;
    }
    else
        kdDebug(7103) << "The proxy URL \"" << m_proxyURL.url() << "\" is either MALFORMED or not supported. IGNORED!!" << endl;
  }

  m_bUseCache = KProtocolManager::useCache();
  if (m_bUseCache)
  {
    m_strCacheDir = KGlobal::dirs()->saveLocation("data", "kio_http/cache");
    m_maxCacheAge = KProtocolManager::maxCacheAge();
  }

  // Define language and charset settings from KLocale (David)
  QStringList languageList = KGlobal::locale()->languageList();
  QStringList::Iterator c = languageList.find( QString::fromLatin1("C") );
  // HTTP servers don't understand "C", they understand "en" :)
  if ( c != languageList.end() )
    (*c) = QString::fromLatin1("en");
  m_strLanguages = languageList.join( " " );
  kdDebug(7103) << "Languages list set to " << m_strLanguages << endl;
  m_strCharsets = KGlobal::locale()->charset() + QString::fromLatin1(";q=1.0, utf-8;q=0.8, *;q=0.9");
}

void HTTPProtocol::flushAuthenticationSettings()
{
  // Flush Authentication settings before
  // starting a request.
  m_strRealm = QString::null;
  m_strAuthString = QString::null;
  Authentication = AUTH_None;
  m_iAuthFailed = 0;
}

void HTTPProtocol::retrieveContent( bool check_ssl )
{
  m_request.window = metaData("window-id");
  flushAuthenticationSettings();

  if(!http_open())
    return;

  if(!readHeader())
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
  m_request.window = metaData("window-id");
  flushAuthenticationSettings();

  if (!http_open())
    return false;

  if (!readHeader())
    return false;

  if ( close_connection )
  {
    http_close();
    finished();
  }

  return true;
}
