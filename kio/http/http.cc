// -*- c-basic-offset: 2 -*-

#include <config.h>

#ifdef HAVE_LIBZ
#define DO_GZIP
#endif

#ifndef HTTPS
#undef HAVE_SSL
#undef DO_SSL
#endif

#ifdef HAVE_SSL
#define DO_SSL
#define DO_MD5
#endif

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>		// Needed on some systems.
#endif

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#include <string>
#include <kconfig.h>
#include <kdesu/client.h>

#ifdef DO_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#ifdef DO_MD5
#include <openssl/md5.h>
#include "extern_md5.h"
#endif

#include "base64.h"

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef DO_GZIP
#include <zlib.h>
#endif

#include "http.h"

#include <qregexp.h>

#include <kapp.h>
#include <klocale.h>
#include <kprotocolmanager.h>
#include <ksock.h>
#include <kurl.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <kservice.h>

using namespace KIO;

// Maximum chunk size is 256K
#define MAX_CHUNK_SIZE (1024*256)

#define MAX_IPC_SIZE (1024*32)

// Timeout for connections to remote sites in seconds
#define REMOTE_CONNECT_TIMEOUT 20

// Timeout for connections to proxy in seconds
#define PROXY_CONNECT_TIMEOUT 10

// Timeout for receiving an answer from a remote side in seconds.
#define RESPONSE_TIMEOUT 60

extern "C" {
  char *create_basic_auth (const char *header, const char *user, const char *passwd);
  const char *create_digest_auth (const char *header, const char *user, const char *passwd, const char *auth_str);
  void sigalrm_handler(int);
#ifdef DO_SSL
  int verify_callback(int, X509_STORE_CTX *);
#endif
};

extern "C" { int kdemain(int argc, char **argv); }

int kdemain( int argc, char **argv )
{
  KLocale::setMainCatalogue("kdelibs");
  KInstance instance( "kio_http" );

  kdDebug(7103) << "Starting " << getpid() << endl;

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

  kdDebug(7103) << "Done" << endl;
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
const char *create_digest_auth (const char *header, const char *user, const char *passwd, const char *auth_str)
{
  string domain, realm, algorithm, nonce, opaque, qop;
  const char *p=auth_str;
  int i;
  HASHHEX HA1, HA2 = "", Response;

  if (!user || !passwd)
    return "";

  QString t1;

  while (*p) {
    while( (*p == ' ') || (*p == ',') || (*p == '\t'))
      p++;
    i = 0;
    if ( strncasecmp(p, "realm=\"", 7 ) == 0 ) {
      p += 7;
      while( p[i] != '"' ) i++;
      realm.assign( p, i );
      kdDebug(7103) << "Realm is :" << realm.c_str() << ":" << endl;
    } else if (strncasecmp(p, "algorith=\"", 10)==0) {
      p+=10;
      while (p[i] != '"' ) i++;
      algorithm.assign(p, i);
      kdDebug(7103) << "Algorithm is :" << algorithm.c_str() << ":" << endl;
    } else if (strncasecmp(p, "algorithm=\"", 11)==0) {
      p+=11;
      while (p[i] != '"') i++;
      algorithm.assign(p,i);
    } else if (strncasecmp(p, "domain=\"", 8)==0) {
      p+=8;
      while (p[i] != '"') i++;
      domain.assign(p,i);
    } else if (strncasecmp(p, "nonce=\"", 7)==0) {
      p+=7;
      while (p[i] != '"') i++;
      nonce.assign(p,i);
    } else if (strncasecmp(p, "opaque=\"", 8)==0) {
      p+=8;
      while (p[i] != '"') i++;
      opaque.assign(p,i);
    } else if (strncasecmp(p, "qop=\"", 5)==0) {
      p+=5;
      while (p[i] != '"') i++;
      qop.assign(p,i);
    }

    p+=i;
    p++;
  }

  t1 += header;
  t1 += ": Digest username=\"";
  t1 += user;
  t1 += "\", ";

  t1 += "realm=\"";
  t1 += realm.c_str();
  t1 += "\", ";

  t1 += "nonce=\"";
  t1 += nonce.c_str();
  t1 += "\", ";

  t1 += "uri=\"";
  t1 += domain.c_str();
  t1 += "\", ";

  char szCNonce[10] = "abcdefghi";
  char szNonceCount[9] = "00000001";



  DigestCalcHA1("md5", user, realm.c_str(), passwd, nonce.c_str(), szCNonce, HA1);
  DigestCalcResponse(HA1, nonce.c_str(), szNonceCount, szCNonce, qop.c_str(), "GET", domain.c_str(), HA2, Response);
  t1 += "qop=\"auth\", ";

  t1 += "cnonce=\"";
  t1 += szCNonce;
  t1 += "\", ";

  t1 += "response=\"";
  t1 += Response;
  t1 += "\", ";

  if (opaque != "") {
    t1 += "opaque=\"";
    t1 += opaque.c_str();
    t1 += "\" ";
  }

  t1 += "\r\n";

  return strdup(t1.latin1());
}
#else
const char *create_digest_auth (const char *, const char *, const char *, const char *)
{
  //error(ERR_COULD_NOT_AUTHENTICATE, "digest");
  return strdup("\r\n");
}
#endif

char *create_basic_auth (const char *header, const char *user, const char *passwd)
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
    sprintf(wwwauth, "%s: Basic %s\r\n", header, t2);
    free(t2);
  }
  else
    wwwauth = NULL;

  return(wwwauth);
}

/*****************************************************************************/

/* Domain suffix match. E.g. return true if host is "cuzco.inka.de" and
   nplist is "inka.de,hadiko.de" or if host is "localhost" and
   nplist is "localhost" */

bool revmatch(const char *host, const char *nplist)
{
  const char *hptr = host + strlen( host ) - 1;
  const char *nptr = nplist + strlen( nplist ) - 1;
  const char *shptr = hptr;

  while( nptr >= nplist ) {
    if ( *hptr != *nptr ) {
      hptr = shptr;
      // Try to find another domain or host in the list
      while ( --nptr>=nplist && *nptr!=',' && *nptr!=' ')
	;
      while ( --nptr>=nplist && (*nptr==',' || *nptr==' '))
	;
    } else {
      if ( nptr==nplist || nptr[-1]==',' || nptr[-1]==' ') {
	return true;
      }
      hptr-=2;
    }
  }

  return false;
}

#ifdef DO_SSL
// This is for now a really stupid yes-man callback.
// If I (or someone else) feels motivated enough to do some
// real verification a la OpenSSL, then this might be a
// more useful function.
int verify_callback (int, X509_STORE_CTX *)
{
  return 1;
}
#endif


/*****************************************************************************/

HTTPProtocol::HTTPProtocol( const QCString &protocol, const QCString &pool, const QCString &app )
  : SlaveBase( protocol, pool, app)
{
  m_maxCacheAge = 0;
  m_fsocket = 0L;
  m_sock = 0;
  m_fcache = 0;
  m_bKeepAlive = false;
  m_iSize = -1;
  m_dcopClient = new DCOPClient();
  if (!m_dcopClient->attach())
  {
     kdDebug(7103) << "Can't connect with DCOP server." << endl;
  }

  m_bCanResume = true; // most of http servers support resuming ?

  reparseConfiguration();

  m_bEOF=false;
#ifdef DO_SSL
  m_bUseSSL2=true; m_bUseSSL3=true; m_bUseTLS1=false;
  m_bUseSSL=true;
  meth=0; ctx=0; hand=0;
#endif

  m_sContentMD5 = "";
  Authentication = AUTH_None;
  ProxyAuthentication = AUTH_None;

  m_HTTPrev = HTTP_Unknown;

#ifdef DO_SSL
  if (mProtocol == "https")
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
  if (mProtocol == "ftp")
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
  m_bUseSSL2=true; m_bUseSSL3=true; m_bUseTLS1=false;
  if (m_bUseSSL2 && m_bUseSSL3)
    meth=SSLv23_client_method();
  else if (m_bUseSSL3)
    meth=SSLv3_client_method();
  else
    meth=SSLv2_client_method();

  SSLeay_add_all_algorithms();
  SSLeay_add_ssl_algorithms();
  ctx=SSL_CTX_new(meth);
  if (ctx == NULL) {
    kdDebug(7103) << "We've got a problem!" << endl;
    fflush(stderr);
  }
  SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, verify_callback);
  hand=SSL_new(ctx);
}
#endif


int HTTPProtocol::openStream() {
#ifdef DO_SSL
  if (m_bUseSSL) {
    initSSL();
    SSL_set_fd(hand, m_sock);
    if (SSL_connect(hand)== -1)
      return false;
    return true;
  }
#endif
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
    return SSL_write(hand, (char *)buf, nbytes);
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
    ret=SSL_read(hand, (char *)b, nbytes);
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
   kdDebug(7103) << "Testing existing connection." << endl;
   fd_set rdfs;
   struct timeval tv;
   int retval;
   FD_ZERO(&rdfs);
   FD_SET(m_sock , &rdfs);
   tv.tv_usec = 0;
   tv.tv_sec = 0;
   retval = select(m_sock+1, &rdfs, NULL, NULL, &tv);

   kdDebug(7103) << "Select returns with = " << retval << endl;
   if (retval != 0)
   {
       char buffer[100];
       retval = recv(m_sock, buffer, 80, MSG_PEEK);
       kdDebug(7103) << "Recv returns with = " << retval << endl;
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
      m_request.do_proxy = !revmatch(m_request.hostname.latin1(), m_strNoProxyFor.latin1());

  if (m_sock)
  {
     bool closeDown = false;
     kdDebug(7103) << "http_checkConnection: connection still active (" << getpid() << ")" << endl;
     if (!m_state.do_proxy && !m_request.do_proxy)
     {
        if (m_state.hostname != m_request.hostname)
        {
           closeDown = true;
           kdDebug(7103) << "keep_alive: host does not match. (" << m_state.hostname << " vs " << m_request.hostname << ")" << endl;
        }
        else if (m_state.port != m_request.port)
        {
           closeDown = true;
           kdDebug(7103) << "keep_alive: port does not match. (" << m_state.port << " vs " << m_request.port << ")" << endl;
        }
        else if (m_state.user != m_request.user)
        {
           closeDown = true;
           kdDebug(7103) << "keep_alive: user does not match. (" << m_state.user << " vs " << m_request.user << ")" << endl;
        }
        else if (m_state.passwd != m_request.passwd)
        {
           closeDown = true;
           kdDebug(7103) << "keep_alive: paswd does not match." << endl;
        }
     }
     else if (m_request.do_proxy && m_state.do_proxy)
     {
        // Keep the connection to the proxy.
     }
     else
     {
        closeDown = true;
        kdDebug(7103) << "keep_alive: proxy setting changed." << endl;
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
    kdDebug(7103) << "http_openConnection: making new connection (" << getpid() << ")" << endl;
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
      kdDebug(7103) << "http_openConnection " << m_strProxyHost << " " << m_strProxyPort << endl;
      // yep... open up a connection to the proxy instead of our host
      if(!KSocket::initSockaddr(&m_proxySockaddr, m_strProxyHost.latin1(), m_strProxyPort)) {
        error(ERR_UNKNOWN_PROXY_HOST, m_strProxyHost);
        return false;
      }

      infoMessage( i18n("Connecting to %1...").arg(m_state.hostname) );

      if(::connect(m_sock, (struct sockaddr*)(&m_proxySockaddr), sizeof(m_proxySockaddr))) {
        if((errno != EINPROGRESS) && (errno != EWOULDBLOCK)) {
          // Error
          error(ERR_COULD_NOT_CONNECT, m_strProxyHost );
          kdDebug(7103) << "Could not connect to PROXY (line:697)" << endl;
          return false;
        }
        // Wait for connection
        if (!waitForConnect(m_sock, PROXY_CONNECT_TIMEOUT))
        {
          error(ERR_COULD_NOT_CONNECT, m_strProxyHost );
          kdDebug(7103) << "Timed out waiting to connect to PROXY (line:703)" << endl;
          return false;
        }
      }
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
    }

    // Set socket blocking.
    fcntl(m_sock, F_SETFL, ( fcntl(m_sock, F_GETFL) & ~O_NDELAY));

    // Placeholder
    if (!openStream())
    {
      error( ERR_COULD_NOT_CONNECT, m_state.hostname );
      return false;
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
  http_checkConnection();

  m_fcache = 0;
  m_bCachedRead = false;
  m_bCachedWrite = false;
  if (m_bUseCache)
  {
     m_fcache = checkCacheEntry( m_state.cef );
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

     if (m_fcache)
     {
        m_bCachedRead = true;
        return true;
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
    header += (mProtocol == "ftp") ? "ftp://" : "http://";
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
  header += "User-Agent: "; /* User agent */
  header += getUserAgentString();
  header += "\r\n";

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
    kdDebug(7103) << "kio_http : Range = " << c_buffer << endl;
  }


  if ( m_request.cache == CC_Reload ) { /* No caching for reload */
    header += "Pragma: no-cache\r\n"; /* for HTTP/1.0 caches */
    header += "Cache-control: no-cache\r\n"; /* for HTTP >=1.1 caches */
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
    header += cookieStr;
  }

  if (m_request.method == HTTP_POST) {
    header += m_request.user_headers;
  }

  int auth_type;
  QString user, password;
  // we now try to determine if we have any saved authentication
  // credentials.  this can be done two ways; either this particular
  // process has a saved username and password in the state OR this
  // process has just started and we check if it is cached.
  if (!m_state.passwd.isNull() && !m_state.user.isNull())
  {
    kdDebug(7103) << "(" << getpid() << ") Saved state authentication: m_state.user = " << m_state.user << ", m_state.passwd = " << m_state.passwd << endl;
    user     = m_state.user;
    password = m_state.passwd;
  }
  else
  {
    kdDebug(7103) << "(" << getpid() << ")  Checking cached authentication" << endl;
    // we have just started... check if we are in a "state" of
    // authentication and didn't even know it
    QString realm;
    if (m_strRealm.isNull())
      realm = m_request.url.host();
    else
      realm = m_strRealm + "@" + m_request.url.host();

    QString valid_path;
    checkCachedAuthentication(user, password, auth_type, valid_path, realm);
    if (valid_path.isNull())
      valid_path = "/";

    // now we make sure that the current path is a valid one
    QString path(m_request.url.directory());
    if (valid_path != path.left(valid_path.length())) {
      user     = QString::null;
      password = QString::null;
      Authentication = AUTH_None;
    }
    else
    {
      Authentication = (HTTP_AUTH)auth_type;
      m_state.user   = user;
      m_state.passwd = password;
    }
  }
  kdDebug(7103) << "(" << getpid() << "): Cached auth for realm " << m_strRealm << ": user= " << user << ", pass= " << password << ", auth_type = " << auth_type << endl;

  // check if we need to login
  if (!password.isNull() || !user.isNull()) {
    if (Authentication == AUTH_Basic || Authentication == AUTH_None) {
      header += create_basic_auth("Authorization", user.latin1(), password.latin1());
    } else if (Authentication == AUTH_Digest) {
      header += create_digest_auth("Authorization", user.latin1(), password.latin1(),
                                   m_strAuthString.latin1());
    }
    // Don't do this as the authorization methods already add it!!!
    // header+="\r\n";
  }

  // the proxy might need authorization of it's own. do that now
  if( m_state.do_proxy ) {
    kdDebug(7103) << "http_open 3" << endl;
    if( m_strProxyUser != "" && m_strProxyPass != "" ) {
      if (ProxyAuthentication == AUTH_None || ProxyAuthentication == AUTH_Basic) {
	header += create_basic_auth("Proxy-Authorization", m_strProxyUser.latin1(), m_strProxyPass.latin1() );
      } else {
	if (ProxyAuthentication == AUTH_Digest) {
	  header += create_digest_auth("Proxy-Authorization",
				       m_strProxyUser.latin1(),
				       m_strProxyPass.latin1(),
				       m_strProxyAuthString.latin1());
	}
      }
    }
  }

  if (!moreData)
     header += "\r\n";  /* end header */

  kdDebug(7103) << "Sending header: \n===\n" << header << "\n===" << endl;
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
     if (!fgets(buffer, 4096, m_fcache))
     {
        // Error, delete cache entry
        error( ERR_CONNECTION_BROKEN, m_state.hostname );
        return false;
     }
     m_strMimeType = QString::fromUtf8( buffer).stripWhiteSpace();
     mimeType(m_strMimeType);
     return true;
  }

  // to get rid of those "Open with" dialogs...
  // however at least extensions should be checked
  m_strMimeType = "text/html";

  QCString locationStr; // In case we get a redirect.
  QCString cookieStr; // In case we get a cookie.

  // read in 4096 bytes at a time (HTTP cookies can be quite large.)
  int len = 0;
  char buffer[4097];
  bool unauthorized = false;
  bool cont = false;
  bool noRedirect = false; // No automatic redirection
  time_t cacheExpireDate = 0;

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
      kdDebug(7103) << "Got header (" << getpid() << "): --empty--" << endl;
      continue;
    }

    kdDebug(7103) << "Got header (" << getpid() << "): \"" << buffer << "\"" << endl;

    // are we allowd to resume?  this will tell us
    if (strncasecmp(buffer, "Accept-Ranges:", 14) == 0) {
      if (strncasecmp(trimLead(buffer + 14), "none", 4) == 0)
	m_bCanResume = false;
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

      //HACK to get the right mimetype of returns like "text/html; charset foo-blah"
      int semicolonPos = m_strMimeType.find( ';' );
      if ( semicolonPos != -1 )
        m_strMimeType = m_strMimeType.left( semicolonPos );
    }
		
    // whoops.. we received a warning
    else if (strncasecmp(buffer, "Warning:", 8) == 0) {
      warning(trimLead(buffer + 8));
    }
    else if (strncasecmp(buffer, "Pragma: no-cache", 16) == 0) {
      m_bCachedWrite = false; // Don't put in cache
    }
    else if (strncasecmp(buffer, "Expires:", 8) == 0) {
      const char *expire = trimLead( buffer+8);
//WABA
 kdDebug(7103) << "Expires =!" << expire << "!" << endl;
    }
    else if (strncasecmp(buffer, "Cache-Control:", 14) == 0) {
      QStringList cacheControls = QStringList::split(',',
                                     QString::fromLatin1(trimLead(buffer+14)));
      for(QStringList::ConstIterator it = cacheControls.begin();
          it != cacheControls.end();
          it++)
      {
         QString cacheControl = (*it).stripWhiteSpace();
 kdDebug(7103) << "Cache-Control =!" << cacheControl << "!" << endl;
         if (strncasecmp(cacheControl.latin1(), "no-cache", 8) == 0)
         {
            m_bCachedWrite = false; // Don't put in cache
         }
         else if (strncasecmp(cacheControl.latin1(), "no-store", 8) == 0)
         {
            m_bCachedWrite = false; // Don't put in cache
         }
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
      int code = atoi(buffer+9);

      // unauthorized access
      if ((code == 401) || (code == 407)) {
	unauthorized = true;
        m_bCachedWrite = false; // Don't put in cache
      }
      // server side errors
      else if ((code >= 500) && (code <= 599)) {
        if (m_request.method == HTTP_HEAD) {
           // Ignore error
        } else {
           errorPage();
        }
        m_bCachedWrite = false; // Don't put in cache
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
      }
      else if (code == 100)
      {
	// We got 'Continue' - ignore it
        cont = true;
      }
      else if ((code == 301) || (code == 307))
      {
	// 301 Moved permanently
        // 307 Temporary Redirect
        if (m_request.method == HTTP_POST)
        {
           errorPage();	
           m_bCachedWrite = false; // Don't put in cache
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
          kdDebug(7103) << "KeepAlive = false" << endl;
	} else if (strncasecmp(trimLead(buffer + 11), "Keep-Alive", 10)==0) {
#ifdef DO_SSL
          // Don't do persistant connections with SSL.
          if (!m_bUseSSL)
             m_bKeepAlive = true;
#else
          m_bKeepAlive = true;
#endif
          kdDebug(7103) << "KeepAlive = true" << endl;
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

  // DONE receiving the header!
  if (!cookieStr.isEmpty())
  {
     // Give cookies to the cookiejar.
     addCookies( m_request.url.url(), cookieStr );
  }

  // we need to reread the header if we got a '100 Continue'
  if ( cont )
    return readHeader();
  // we need to try to login again if we failed earlier
  else if (unauthorized) {
    http_closeConnection();
    if (m_strRealm.isEmpty())
      m_strRealm = m_state.hostname;

    if (!openPassDlg( i18n( "Authorization is required for <b>%1</b> at <b>%2</b>").arg( m_strRealm ).arg( m_request.hostname ) ,
    				  m_request.user, m_request.passwd, m_strRealm + "@" + m_request.hostname )) {
      error(ERR_ACCESS_DENIED, m_state.hostname);
      return false;
    }
    else
      cacheAuthentication(m_request.url, m_request.user, m_request.passwd, (int)Authentication);

    if ( !http_open())
      return false;

    return readHeader();
  }
  // We need to do a redirect
  else if (!locationStr.isEmpty() && !noRedirect)
  {
#if 0
    // WABA:
    // We either need to close the connection or read the rest of the contents!
    // Closing the connection is bad for performance.
    // We really should read the contents here!
    http_closeConnection();
    KURL u(m_request.url, locationStr);
    redirection(u.url());

    if ( !http_open() )
      return false;

    return readHeader();
#else
    kdDebug(7103) << "request.url is " << m_request.url.url() << " locationstr " << locationStr.data() << endl;

    KURL u(m_request.url, locationStr);
    if(u.isMalformed()) {
      error(ERR_MALFORMED_URL, u.url());
      return false;
    }

    redirection(u.url());
    m_bCachedWrite = false; // Turn off caching on re-direction (DA)
#endif
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
        m_strMimeType = QString::fromLatin1("application/gzip");
     }
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

  if (m_request.method == HTTP_HEAD)
     return true;

  // Do we want to cache this request?
  if (m_bCachedWrite)
  {
     // Check...
     createCacheEntry(m_strMimeType, cacheExpireDate); // Create a cache entry
     if (!m_fcache)
        m_bCachedWrite = false; // Error creating cache entry.
  }

  if (m_bCachedWrite)
    kdDebug(7103) << "Cache, adding \"" << m_request.url.url() << "\"" << endl;
  else
    kdDebug(7103) << "Cache, not adding \"" << m_request.url.url() << "\"" << endl;

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
  } else if ((encoding == "x-deflate") || (encoding == "deflate")) {
    encs.append(QString::fromLatin1("deflate"));
    // kdDebug(7103) << "Deflate not implemented.  Please write code. Pid = " << getpid() << " Encoding = \"" << encoding << "\"" << endl;
    // abort();
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
    f = AUTH_Basic;
    p += 5;
  } else if (strncasecmp (p, "Digest", 6) ==0 ) {
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
      m_strRealm=assign;
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
   kdDebug( 7103 ) << "POST: " << c_buffer << endl;

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
        return;
     }
  }
  if (!m_bKeepAlive)
     http_closeConnection();
  else
     kdDebug(7103) << "http_close: keep alive" << endl;
}

void HTTPProtocol::http_closeConnection()
{
  kdDebug(7103) << "http_closeConnection: closing (" << getpid() << ")" << endl;
  m_bKeepAlive = false; // Just in case.
  if ( m_fsocket )
    fclose( m_fsocket );
  m_fsocket = 0;
  if ( m_sock )
    ::close( m_sock );
  m_sock = 0;
}


const char *HTTPProtocol::getUserAgentString ()
{
  static QString prev_hostname = QString::null;
  static QString user_agent = "Konqueror ($Revision$)";

  // i'm not a big fan of this... however, i am very concerned that
  // the regular expression matching later one could prove to be very
  // CPU intensive.. especially when you consider that every single
  // HTTP request (even those for gifs and pngs) will call this
  // function.  so for now, we try to only match a pattern to a
  // hostname once... as long as the requests all come in order
  if ( prev_hostname == m_state.hostname )
    return strdup(user_agent.latin1());

  prev_hostname = m_state.hostname;

  // try to load the user set UserAgents
  if ( m_userAgentList.count() == 0 )
  {
    KConfig *config = new KConfig("kioslaverc");
    KConfigGroupSaver saver(config, "Browser Settings/UserAgent");

    int entries = config->readNumEntry( "EntriesCount", 0 );
    m_userAgentList.clear();
    for( int i = 0; i < entries; i++ )
    {
      QString key;
      key.sprintf( "Entry%d", i );
      QString entry = config->readEntry( key, "" );
      m_userAgentList.append( entry );
    }
    delete config;
  }

  // make sure we have at least *one*, though
  if ( m_userAgentList.count() == 0 )
    m_userAgentList.append( "*:Konqueror ($Revision$)" );

  // now, we need to do our pattern matching on the host name.
  QStringList::Iterator it(m_userAgentList.begin());
  for( ; it != m_userAgentList.end(); ++it)
  {
    QStringList split(QStringList::split( ':', (*it) ));

    // if our user agent is null, we go to the next one
    if ( split[1].isNull() )
      continue;

    QRegExp regexp(split[0], true, true);

    // we also make sure our regexp is valid
    if ( !regexp.isValid() )
      continue;

    // we look for a match
    if ( regexp.match(m_state.hostname) > -1 )
    {
      user_agent = split[1];

      // if the match was for '*', we keep trying.. otherwise, we are
      // done
      if ( split[0] != "*" )
        break;
    }
  }

#ifdef DO_MD5
  user_agent+="; Supports MD5-Digest";
#endif
#ifdef DO_GZIP
  user_agent+="; Supports gzip encoding";
#endif
#ifdef DO_SSL
  user_agent+="; Supports SSL/HTTPS";
#endif
  return strdup(user_agent.latin1());
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

  connected();
}

void HTTPProtocol::slave_status()
{
  bool connected = (m_sock != 0);
  if (connected && !http_isConnected())
  {
     http_closeConnection();
     connected = false;
  }
  kdDebug(7103) << "Got slave_status host = " << (m_state.hostname.latin1() ? m_state.hostname.latin1() : "[None]") << " [" << (connected ? "Connected" : "Not connected") << "]" << endl;
  slaveStatus( m_state.hostname, connected );
}

void HTTPProtocol::buildURL()
{
  m_request.url = QString::fromLatin1(mProtocol+":/");
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
  if (cacheControl.isEmpty()) 
     return HTTPProtocol::CC_Cache; // Default
  
  QString tmp = cacheControl.lower();   
  if (tmp == "cacheonly")
     return HTTPProtocol::CC_CacheOnly;
  if (tmp == "verify")
     return HTTPProtocol::CC_Verify;
  if (tmp == "reload")
     return HTTPProtocol::CC_Reload;
  return HTTPProtocol::CC_Cache; // "cache" and Default
}

// Returns only the file size, that's all kio_http can guess.
void HTTPProtocol::stat(const KURL& url)
{
  if (m_request.hostname.isEmpty())
     error( KIO::ERR_INTERNAL, "stat: No host specified!");

  m_request.method = HTTP_HEAD;
  m_request.path = url.path();
  m_request.query = url.query();
  m_request.cache = parseCacheControl(metaData("cache"));
  m_request.offset = 0;
  m_request.do_proxy = m_bUseProxy;
  m_request.url = url;

  if (http_open()) {

    if (readHeader())
    {
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

      atom.m_uds = KIO::UDS_SIZE;
      atom.m_long = m_iSize;
      entry.append( atom );

      statEntry( entry );

      http_close();
      finished();
    }
    else {
      http_close();
      // error already emitted
    }

  }
}

void HTTPProtocol::get( const KURL& url )
{
  if (m_request.hostname.isEmpty())
     error( KIO::ERR_INTERNAL, "http GET: No host specified!");

  m_request.method = HTTP_GET;
  m_request.path = url.path();
  m_request.query = url.query();
  m_request.cache = parseCacheControl(metaData("cache"));

  m_request.offset = 0;
  m_request.do_proxy = m_bUseProxy;
  m_request.url = url;
  m_request.passwd = url.pass();
  m_request.user = url.user();

  if (!http_open())
     return;

  if (!readHeader())
     return;

  if (!readBody())
     return;

  http_close();
  finished();
}

void HTTPProtocol::put( const KURL &url, int, bool, bool)
{
  if (m_request.hostname.isEmpty())
     error( KIO::ERR_INTERNAL, "http PUT: No host specified!");

  m_request.method = HTTP_PUT;
  m_request.path = url.path();
  m_request.query = QString::null;
  m_request.cache = CC_Reload;
  m_request.offset = 0;
  m_request.do_proxy = m_bUseProxy;
  m_request.url = url;

  if (!http_open())
     return;

  if (!readHeader())
     return;

  if (!readBody())
     return;

  http_close();
  finished();
}

void HTTPProtocol::post( const KURL& url)
{
  if (m_request.hostname.isEmpty())
     error( KIO::ERR_INTERNAL, "http POST: No host specified!");

  m_request.method = HTTP_POST;
  m_request.path = url.path();
  m_request.query = url.query();
  m_request.cache = CC_Reload;
  m_request.offset = 0;
  m_request.do_proxy = m_bUseProxy;
  m_request.url = url;

  if (!http_open())
     return;

  if (!readHeader())
     return;

  if (!readBody())
     return;

  http_close();
  finished();
}

void HTTPProtocol::mimetype( const KURL& url )
{
  kdDebug(7103) << "http: mimetype(" << url.url() << ")" << endl;
  if (m_request.hostname.isEmpty())
     //error( KIO::ERR_INTERNAL, "http MIMETYPE: No host specified!");
     error( KIO::ERR_UNKNOWN_HOST, "No host specified!");

  m_request.method = HTTP_HEAD;
  m_request.path = url.path();
  m_request.query = url.query();
  m_request.cache = CC_Cache;
  m_request.offset = 0;
  m_request.do_proxy = m_bUseProxy;
  m_request.url = url;

  if (!http_open())
     return;

  if (!readHeader())
     return;

  kdDebug(7103) << "http: mimetype = " << m_strMimeType << endl;

  http_close();
  finished();

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
      stream >> url >> m_request.user_headers;
      kdDebug() << "user_headers = " << m_request.user_headers.latin1() << endl;
      post( url );
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
  kdDebug(7103) << "Chunk header = \"" << m_bufReceive.data() << "\"" << endl;
  if (eof())
  {
     kdDebug(7103) << "EOF on Chunk header" << endl;
     return -1;
  }

  int chunkSize = strtol(m_bufReceive.data(), 0, 16);
  if ((chunkSize < 0) || (chunkSize > MAX_CHUNK_SIZE))
     return -1;

  kdDebug(7103) << "Chunk size = " << chunkSize << " bytes" << endl;

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
      kdDebug(7103) << "Chunk trailer = \"" << m_bufReceive.data() << "\"" << endl;
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

     kdDebug(7103) << "Read from chunk got " << bytesReceived << " bytes" << endl;

     if (bytesReceived == -1)
        return -1; // Failure.

     totalBytesReceived += bytesReceived;
     bytesToReceive -= bytesReceived;

     kdDebug(7103) << "Chunk has " << totalBytesReceived << " bytes, " << bytesToReceive << " bytes to go" << endl;
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
  MD5_CTX context;
  MD5_Init(&context);
#endif
  if (m_iSize > -1)
    m_iBytesLeft = m_iSize;
  else
    m_iBytesLeft = 1;


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
          MD5_Update(&context, (const unsigned char*)m_bufReceive.data(), bytesReceived);
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
    MD5_Update(&context, (const unsigned char*)big_buffer.data(),
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
  MD5_Final((unsigned char*)buf, &context); // Wrap everything up
  enc_digest = base64_encode_string(buf, 16);
  if ( useMD5 ) {
    int f;
    if ((f = m_sContentMD5.find("=")) <= 0) {
      f = m_sContentMD5.length();
    }

    if (m_sContentMD5.left(f) != enc_digest) {
      kdDebug(7103) << "MD5 checksum mismatch : got " << m_sContentMD5.left(f) << " , calculated " << enc_digest << endl;
//      error(ERR_CHECKSUM_MISMATCH, m_state.url.url());
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
HTTPProtocol::addCookies( const QString &url, const QCString &cookieHeader)
{
   QByteArray params;
   QDataStream stream(params, IO_WriteOnly);
   stream << url << cookieHeader;
   if (!m_dcopClient->send("kcookiejar", "kcookiejar",
	"addCookies(QString, QCString)", params))
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
      printf("DCOP function findCookies(...) return %s, expected %s\n",
		replyType.data(), "QString");
      return QString::null;
   }

   QString result;
   stream2 >> result;

   return result;
}

// !START SYNC!
// The following code should be kept in sync
// with the code in http_cache_cleaner.cpp

#define CACHE_REVISION "3\n"

FILE *
HTTPProtocol::checkCacheEntry( QString &CEF)
{
   const QChar seperator = '_';

   CEF = m_request.path;

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

   FILE *fs = fopen( CEF.latin1(), "r");
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
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      date = (time_t) strtoul(buffer, 0, 10);
      if (date && (date < currentDate))
         ok = false; // Expired
   }


   if (ok)
      return fs;

   fclose(fs);
   unlink( CEF.latin1());
   return 0;
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
   fputs(date.latin1(), m_fcache);      // Expire date
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

  if ( KProtocolManager::useProxy() ) {

    // Use the appropriate proxy depending on the protocol
    KURL ur (
      mProtocol == "ftp"
      ? KProtocolManager::ftpProxy()
      : KProtocolManager::httpProxy() );

    if (!ur.isEmpty())
    {
      kdDebug(7103) << "Using proxy " << ur.url() << endl;
      // Set "use proxy" to true if we got a non empty proxy URL
      m_bUseProxy = true;

      m_strProxyHost = ur.host();
      m_strProxyPort = ur.port();
      m_strProxyUser = ur.user();
      m_strProxyPass = ur.pass();

      m_strNoProxyFor = KProtocolManager::noProxyFor();
    }
  }

  m_bUseCache = KProtocolManager::useCache();
  if (m_bUseCache)
  {
     m_strCacheDir = KGlobal::dirs()->saveLocation("data", "kio_http/cache");
     m_maxCacheAge = KProtocolManager::maxCacheAge();
  }

}
