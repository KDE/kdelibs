#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LIBZ
#define DO_GZIP
#endif

#ifdef HAVE_SSL
#define DO_SSL
#define DO_MD5
#endif

#include <sys/stat.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#include <string>

#ifdef DO_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#ifdef DO_MD5
#include <openssl/md5.h>
#endif
#include "extern_md5.h"

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef DO_GZIP
#include <zlib.h>
#endif

#include "http.h"

#include <kio/skipdlg.h>
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

#include <kio/slaveinterface.h>
#include <kio/passdlg.h>

using namespace KIO;

// Maximum chunk size is 256K
#define MAX_CHUNK_SIZE (1024*256)

#define MAX_IPC_SIZE (1024*32)

template class QStack<char>;

extern "C" {
  char *create_basic_auth (const char *header, const char *user, const char *passwd);
  const char *create_digest_auth (const char *header, const char *user, const char *passwd, const char *auth_str);
  void sigalrm_handler(int);
#ifdef DO_SSL
  int verify_callback(int, X509_STORE_CTX *);
#endif
};

#if 0
int main( int argc, char **argv )
{
  KInstance instance( "kio_http" );

  kdebug( KDEBUG_INFO, 7103, "Starting %d", getpid());

  if (argc != 2)
  {
     fprintf(stderr, "Usage: kio_file UNIX-domain-socket\n");
  }
  KSocket sock(argv[1]);

  printf("kio_file: socket = %d\n", sock.socket());

  KIOConnection parent( sock.socket(), sock.socket() );

  HTTPProtocol http( &parent );
  http.dispatchLoop();

  kdebug( KDEBUG_INFO, 7103, "Done" );
}
#endif

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
      kdebug( KDEBUG_INFO, 7103, "Realm is :%s:", realm.c_str());
    } else if (strncasecmp(p, "algorith=\"", 10)==0) {
      p+=10;
      while (p[i] != '"' ) i++;
      algorithm.assign(p, i);
      kdebug( KDEBUG_INFO, 7103, "Algorithm is :%s:", algorithm.c_str());
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

HTTPProtocol::HTTPProtocol( KIO::Connection *_conn, const QCString &protocol ) 
  : SlaveBase( _conn )
{
  m_protocol = protocol;
  m_maxCacheAge = 0;
  m_cmd = CMD_NONE;
  m_fsocket = 0L;
  m_sock = 0;
  m_fcache = 0;
  m_bKeepAlive = false;
  m_iSize = 0;
  m_dcopClient = new DCOPClient();
  if (!m_dcopClient->attach())
  {
     kdebug( KDEBUG_INFO, 7103, "Can't connect with DCOP server.");
  }

  m_bCanResume = true; // most of http servers support resuming ?

  m_bUseProxy = KProtocolManager::self().useProxy();

  if ( m_bUseProxy ) {
    KURL ur ( KProtocolManager::self().httpProxy() );

    m_strProxyHost = ur.host();
    m_strProxyPort = ur.port();
    m_strProxyUser = ur.user();
    m_strProxyPass = ur.pass();

    m_strNoProxyFor = KProtocolManager::self().noProxyFor();
  }

  m_bUseCache = KProtocolManager::self().useCache();
  if (m_bUseCache)
  {
     m_strCacheDir = KGlobal::dirs()->saveLocation("data", "kio_http/cache");
     m_maxCacheAge = KProtocolManager::self().maxCacheAge();
  }

  m_bEOF=false;
#ifdef DO_SSL
  m_bUseSSL2=true; m_bUseSSL3=true; m_bUseTLS1=false;
  m_bUseSSL=false;
  meth=0; ctx=0; hand=0;
#endif

  m_sContentMD5 = "";
  Authentication = AUTH_None;
  ProxyAuthentication = AUTH_None;

  m_HTTPrev = HTTP_Unknown;

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
    kdebug( KDEBUG_INFO, 7103, "We've got a problem!");
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

void HTTPProtocol::http_openConnection()
{
  // try to ensure that the port is something reasonable
  unsigned short int port = m_request.port;
  if ( port == 0 ) {
#ifdef DO_SSL
    if (m_protocol=="https") {
      struct servent *sent = getservbyname("https", "tcp");
      if (sent) {
        port = ntohs(sent->s_port);
      } else
        port = DEFAULT_HTTPS_PORT;
    } else
#endif
      if ( (m_protocol=="http") || (m_protocol == "httpf") ) {
        struct servent *sent = getservbyname("http", "tcp");
	if (sent) {
	  port = ntohs(sent->s_port);
	} else
	  port = DEFAULT_HTTP_PORT;
      } else {
	kdebug( KDEBUG_INFO, 7103, "Got a weird protocol (%s), assuming port is 80", m_protocol.data()); 
	port = 80;
      }
  }
  m_request.port = port;

#if 0
  // Move to initialisation
  // make sure that we can support what we are asking for
  if (m_protocol() == "https") {
#ifdef DO_SSL
    m_bUseSSL=true;
#else
    error(ERR_UNSUPPORTED_PROTOCOL, i18n("You do not have OpenSSL/SSLeay installed, or you have not compiled kio_http with SSL support"));
    return false;
#endif
  }
#endif

  // do we want to use a proxy?
  m_request.do_proxy = m_bUseProxy;

  // if so, we had first better make sure that our host isn't on the
  // No Proxy list
  if (m_request.do_proxy && !m_strNoProxyFor.isEmpty())
      m_request.do_proxy = !revmatch(m_request.hostname, m_strNoProxyFor);

  if (m_sock)
  {
     bool closeDown = false;
     kdebug( KDEBUG_INFO, 7103, "http_open: connection still active (%d)", getpid());
     if (!m_state.do_proxy && !m_request.do_proxy)
     {
        if (m_state.hostname != m_request.hostname)
        {
           closeDown = true;
           kdebug( KDEBUG_INFO, 7103, "keep_alive: host does not match. (%s vs %s)",
                   m_state.hostname.ascii(), m_request.hostname.ascii());
        }
        else if (m_state.port != m_request.port)
        {
           closeDown = true;
           kdebug( KDEBUG_INFO, 7103, "keep_alive: port does not match. (%d vs %d)",
	           m_state.port, m_request.port);
        }
        else if (m_state.user != m_request.user)
        {
           closeDown = true;
           kdebug( KDEBUG_INFO, 7103, "keep_alive: user does not match. (%s vs %s)",
                   m_state.user.ascii(), m_request.user.ascii());
        }
        else if (m_state.passwd != m_request.passwd)
        {
           closeDown = true;
           kdebug( KDEBUG_INFO, 7103, "keep_alive: paswd does not match.");
        }
     }
     else if (m_request.do_proxy && m_state.do_proxy)
     {
        // Keep the connection to the proxy.
     }
     else 
     {
        closeDown = true;
        kdebug( KDEBUG_INFO, 7103, "keep_alive: proxy setting changed.");
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
  http_openConnection();

  m_fcache = 0;
  m_bCachedRead = false;
  m_bCachedWrite = false;
  if (m_bUseCache)
  {
#ifdef DO_SSL
     if (!m_request.reload && !m_bUseSSL)
#else
     if (!m_request.reload)
#endif
     {
        m_fcache = checkCacheEntry( m_state.cef );
        m_bCachedWrite = true;
     }

     if (m_fcache)
     {
        m_bCachedRead = true;
        return true;
     }
  }

  // Let's also clear out some things, so bogus values aren't used.
  m_sContentMD5 = "";
  m_HTTPrev = HTTP_Unknown;
  m_qContentEncodings.clear();
  m_qTransferEncodings.clear();
  m_bChunked = false;
  m_iSize = 0;

  // let's try to open up our socket if we don't have one already.
  if (!m_sock)
  {
    kdebug( KDEBUG_INFO, 7103, "http_open: making new connection (%d)", getpid());
    m_bKeepAlive = false;
    m_sock = ::socket(PF_INET,SOCK_STREAM,0);
    if (m_sock < 0) {
      m_sock = 0;
      error( ERR_COULD_NOT_CREATE_SOCKET, m_state.hostname );
      return false;
    }

    // do we still want a proxy after all that?
    if( m_state.do_proxy ) {
      kdebug( KDEBUG_INFO, 7103, "http_open 0");
      // yep... open up a connection to the proxy instead of our host
      if(!KSocket::initSockaddr(&m_proxySockaddr, m_strProxyHost, m_strProxyPort)) {
        error(ERR_UNKNOWN_PROXY_HOST, m_strProxyHost);
        return false;
      }

      if(::connect(m_sock, (struct sockaddr*)(&m_proxySockaddr), sizeof(m_proxySockaddr))) {
        error( ERR_COULD_NOT_CONNECT, m_strProxyHost );
        return false;
      }
    } else {
      // apparently we don't want a proxy.  let's just connect directly
      ksockaddr_in server_name;

      if(!KSocket::initSockaddr(&server_name, m_state.hostname, m_state.port)) {
        error( ERR_UNKNOWN_HOST, m_state.hostname );
        return false;
      }

      if(::connect(m_sock, (struct sockaddr*)( &server_name ), sizeof(server_name))) {
        error(ERR_COULD_NOT_CONNECT, m_state.hostname );
        return false;
      }
    }

    // Placeholder
    if (!openStream())
    {
      error( ERR_COULD_NOT_CONNECT, m_state.hostname );
      return false;
    }
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
      break;
  case HTTP_POST: 
      header = "POST ";
      moreData = true;
      break;
  case HTTP_HEAD: 
      header = "HEAD ";
      break;
  case HTTP_DELETE: 
      header = "DELETE "; 
      break;
  }
  
  // format the URI
  char c_buffer[64];
  memset(c_buffer, 0, 64);
  if(m_state.do_proxy) {
    sprintf(c_buffer, ":%u", m_state.port);
    header += "http://";
    header += m_state.hostname;
    header += c_buffer;
  }

  // Let the path be "/" if it is empty ( => true )
  { 
     QString encoded = m_request.path;
     if (encoded.isEmpty())
        encoded = "/";
     else
        KURL::encode(encoded);
     if (!m_request.query.isEmpty())
        encoded += "?" + m_request.query;
     
     header += encoded;
  }

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

  if ( m_request.offset > 0 ) {
    sprintf(c_buffer, "Range: bytes=%li-\r\n", m_request.offset);
    header += c_buffer;
    kdebug( KDEBUG_INFO, 7103, "kio_http : Range = %s", c_buffer);
  }

  if ( m_request.reload ) { /* No caching for reload */
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
  if (m_state.port != 0) {
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
    header += "Content-Type: application/x-www-form-urlencoded\r\n";
  }

  // check if we need to login
  if (m_state.passwd || m_state.user) {
    if (Authentication == AUTH_Basic) {
      header += create_basic_auth("Authorization", m_state.user, m_state.passwd);
    } else if (Authentication == AUTH_Digest) {
      header += create_digest_auth("Authorization", m_state.user,
				  m_state.passwd, m_strAuthString);
    }
    header+="\r\n";
  }

  // the proxy might need authorization of it's own. do that now
  if( m_state.do_proxy ) {
    kdebug( KDEBUG_INFO, 7103, "http_open 3");
    if( m_strProxyUser != "" && m_strProxyPass != "" ) {
      if (ProxyAuthentication == AUTH_None || ProxyAuthentication == AUTH_Basic) {
	header += create_basic_auth("Proxy-authorization", m_strProxyUser, m_strProxyPass);
      } else {
	if (ProxyAuthentication == AUTH_Digest) {
	  header += create_digest_auth("Proxy-Authorization",
				       m_strProxyUser,
				       m_strProxyPass,
				       m_strProxyAuthString);
	}
      }
    }
  }

  if (!moreData)
     header += "\r\n";  /* end header */

  kdebug( KDEBUG_INFO, 7103, "Sending header: \n===\n%s\n===", header.ascii());
  // now that we have our formatted header, let's send it!
  bool sendOk;
  sendOk = (write(header, header.length()) == (ssize_t) header.length());
  if (!sendOk) {
    kdebug( KDEBUG_INFO, 7103, "Connection broken! (%s)", m_state.hostname.ascii());
    if (m_bKeepAlive)
    {
       // With a Keep-Alive connection this can happen.
       // Just reestablish the connection.
       http_closeConnection();
       http_openConnection();
       sendOk = (write(header, header.length()) == (ssize_t) header.length());
    }
    if (!sendOk)
    {
       error( ERR_CONNECTION_BROKEN, m_state.hostname );
       return false;
    }
  }

  if (moreData)
     return sendBody();

  return true;
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

  QString locationStr; // In case we get a redirect.
  QCString cookieStr; // In case we get a cookie.

  // read in 4096 bytes at a time (HTTP cookies can be quite large.)
  int len = 0;
  char buffer[4097];
  bool unauthorized = false;
  bool cont = false;
  bool noRedirect = false; // No automatic redirection
  time_t cacheExpireDate = 0;

  gets(buffer, sizeof(buffer)-1);
  if (eof())
  {
     kdebug(KDEBUG_INFO, 7103, "readHeader: EOF while waiting for header start.");
     if (m_bKeepAlive) // Try to reestablish connection.
     {
        http_closeConnection();
        if ( !http_open() )
	   return false;
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
      kdebug( KDEBUG_INFO, 7103, "Got header (%d): --empty--", getpid());
      continue;
    }

    kdebug( KDEBUG_INFO, 7103, "Got header (%d): \"%s\"", getpid(), buffer);

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
    else if (strncasecmp(buffer, "Content-Type:", 13) == 0) {
      // Jacek: We can't send mimeType signal now,
      // because there may be another Content-Type to come
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
    else if (strncasecmp(buffer, "Cache-Control:", 14) == 0) {
      const char *cacheControl = trimLead( buffer+14);
      if (strncasecmp(cacheControl, "no-cache", 8) == 0)
         m_bCachedWrite = false; // Don't put in cache
      else if (strncasecmp(cacheControl, "no-store", 8) == 0)
         m_bCachedWrite = false; // Don't put in cache
    }
		
    // oh no.. i think we're about to get a page not found
    else if (strncasecmp(buffer, "HTTP/1.0 ", 9) == 0) {
      m_HTTPrev = HTTP_10;
      m_bKeepAlive = false;

      // unauthorized access
      if (strncmp(buffer + 9, "401", 3) == 0) {
	unauthorized = true;
      } else if (buffer[9] == '4' || buffer[9] == '5') {
#if 0
#warning To be fixed! error terminates the job!
	// Let's first send an error message
	// this will be moved to slotErrorPage(), when it will be written
	error(ERR_ACCESS_DENIED, m_state.hostname);
#endif
	// Tell that we will only get an error page here.
	errorPage();

	return false;
      }
    }
		
    // this is probably not a good sign either... sigh
    else if (strncasecmp(buffer, "HTTP/1.1 ", 9) == 0) {
      m_HTTPrev = HTTP_11;
      Authentication = AUTH_None;
#ifdef DO_SSL
      // Don't do persistant connections with SSL.
      if (!m_bUseSSL)
          m_bKeepAlive = true; // HTTP 1.1 has persistant connections.
#else
      m_bKeepAlive = true; // HTTP 1.1 has persistant connections by default.
#endif

      // Unauthorized access
      if ((strncmp(buffer + 9, "401", 3) == 0) || (strncmp(buffer + 9, "407", 3) == 0)) {
	unauthorized = true;
      }
      else if (buffer[9] == '4' || buffer[9] == '5') {
	// Tell that we will only get an error page here.
	errorPage();
      }
      else if (strncmp(buffer + 9, "100", 3) == 0) {
	// We got 'Continue' - ignore it
	cont = true;
      }
      else if ((strncmp(buffer + 9, "301", 3) == 0) ||
               (strncmp(buffer + 9, "307", 3) == 0))
      {
	// 301 Moved permanently
        // 307 Temporary Redirect
        if (m_request.method == HTTP_POST)
        {
           errorPage();	
           noRedirect = true;
        }
      }
      else if ((strncmp(buffer + 9, "302", 3) == 0) ||
               (strncmp(buffer + 9, "303", 3) == 0))
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
    else if (strncmp(buffer, "Location:", 9) == 0 ) {
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
      addEncoding(trimLead(buffer + 17), &m_qContentEncodings);
    }

    // continue only if we know that we're HTTP/1.1
    else if (m_HTTPrev == HTTP_11) {
      // let them tell us if we should stay alive or not
      if (strncasecmp(buffer, "Connection:", 11) == 0) {
	if (strncasecmp(trimLead(buffer + 11), "Close", 5) == 0) {
	  m_bKeepAlive = false;
          kdebug( KDEBUG_INFO, 7103, "KeepAlive = false");
	} else if (strncasecmp(trimLead(buffer + 11), "Keep-Alive", 10)==0) {
#ifdef DO_SSL
          // Don't do persistant connections with SSL.
          if (!m_bUseSSL)
             m_bKeepAlive = true;
#else
          m_bKeepAlive = true;
#endif
          kdebug( KDEBUG_INFO, 7103, "KeepAlive = true");
	}
	
      }
      			
      // what kind of encoding do we have?  transfer?
      else if (strncasecmp(buffer, "Transfer-Encoding:", 18) == 0) {
	// If multiple encodings have been applied to an entity, the
	// transfer-codings MUST be listed in the order in which they
	// were applied.
	addEncoding(trimLead(buffer + 18), &m_qTransferEncodings);
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

    if (!open_PassDlg(m_strRealm, m_request.user, m_request.passwd)) {
      error(ERR_ACCESS_DENIED, m_state.hostname);
      return false;
    }

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
    KURL u(m_request.url, locationStr);
    redirection(u.url());
#endif
  }

  // FINALLY, let the world know what kind of data we are getting
  // and that we do indeed have a header
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
    kdebug( KDEBUG_INFO, 7103, "Cache, adding \"%s\"", m_request.url.url().ascii());
  else
    kdebug( KDEBUG_INFO, 7103, "Cache, not adding \"%s\"", m_request.url.url().ascii());

  return true;
}


void HTTPProtocol::addEncoding(QString encoding, QStack<char> *encs)
{
  // Identity is the same as no encoding
  if (encoding.lower() == "identity") {
    return;
  } else if (encoding.lower() == "chunked") {
    m_bChunked = true;
//    encs->push("chunked");
    // Anyone know of a better way to handle unknown sizes possibly/ideally with unsigned ints?
    if ( m_cmd != CMD_COPY )
      m_iSize = 0;
  } else if ((encoding.lower() == "x-gzip") || (encoding.lower() == "gzip") || (encoding.lower() == "x-deflate") || (encoding.lower() == "deflate")) {
    encs->push(strdup(encoding.lower()));
    if ( m_cmd != CMD_COPY )
      m_iSize = 0;
  } else {
    kdebug( KDEBUG_INFO, 7103, "Unknown encoding encountered.  Please write code. Pid = %d Encoding = \"%s\"", getpid(), encoding.ascii());
    fflush(stderr);
    abort();
  }
}

void HTTPProtocol::configAuth(const char *p, bool b)
{
  HTTP_AUTH f;
  char * strAuth=0, *assign=0;
  int i;

  while( *p == ' ' ) p++;
  if ( strncmp( p, "Basic", 5 ) == 0 ) {
    f = AUTH_Basic;
    p += 5;
  } else if (strncmp (p, "Digest", 6) ==0 ) {
    p += 6;
    f = AUTH_Digest;
    strAuth = strdup(p);
  } else {
    kdebug( KDEBUG_INFO, 7103, "Invalid Authorization type requested");
    kdebug( KDEBUG_INFO, 7103, "buffer: %s", p);
    fflush(stderr);
    abort();
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
   QString header = "Content-Length: ";
   memset(c_buffer, 0, 64);
   sprintf(c_buffer, "%i\r\n", length);
   header += c_buffer;
   header += "\r\n"; /* end header */

   QByteArray *buffer;
   while ( (buffer = bufferList.take(0)))
   {
      write(buffer->data(), buffer->size());
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
        kdebug( KDEBUG_INFO, 7103, "deleting cache entry: %s", filename.ascii());
        unlink( filename.ascii());
        return;
     }
  }
  if (!m_bKeepAlive)
     http_closeConnection();
  else
     kdebug( KDEBUG_INFO, 7103, "http_close: keep alive");
}

void HTTPProtocol::http_closeConnection()
{
  kdebug( KDEBUG_INFO, 7103, "http_closeConnection: closing (%d)", getpid());
  m_bKeepAlive = false; // Just in case.
  if ( m_fsocket )
    fclose( m_fsocket );
  m_fsocket = 0;
  if ( m_sock )
    ::close( m_sock );
  m_sock = 0;
}

#if 1
#warning To be removed ?
// David: No, to be implemented as stat() - it's ok to return
// only the size if that's all kio_http can guess.
#else
void HTTPProtocol::slotGetSize(const char *_url)
{
  KURL usrc(_url);
  if (usrc.isMalformed()) {
    error(ERR_MALFORMED_URL, _url);
    m_cmd = CMD_NONE;
    return;
  }

  if (!isValidProtocol(&usrc)) {
    error(ERR_INTERNAL, "kio_http got non http/https/httpf url");
    m_cmd = CMD_NONE;
    return;
  }

  m_cmd = CMD_GET_SIZE;

  m_bIgnoreErrors = false;
  if (http_open(usrc, 0, false)) {

    if (readHeader())
      totalSize( m_iSize );

    http_close();

    finished();
  }

  m_cmd = CMD_NONE;
  return;
}
#endif

const char *HTTPProtocol::getUserAgentString ()
{
  // This is kio_http, but what we want to show the world
  // is probably more "Konqueror".
  QString user_agent("Konqueror ($Revision$)");
#ifdef DO_MD5
  user_agent+="; Supports MD5-Digest";
#endif
#ifdef DO_GZIP
  user_agent+="; Supports gzip encoding";
#endif
#ifdef DO_SSL
  user_agent+="; Supports SSL/HTTPS";
#endif
  return strdup(user_agent.ascii());
}

void HTTPProtocol::openConnection(const QString& host, int port, const QString& user, const QString& pass)
{
  m_request.hostname = host;

  // try to ensure that the port is something reasonable
  if ( port == 0 ) {
#ifdef DO_SSL
    if (m_protocol=="https") {
      struct servent *sent = getservbyname("https", "tcp");
      if (sent) {
        port = ntohs(sent->s_port);
      } else
        port = DEFAULT_HTTPS_PORT;
    } else
#endif
      if ( (m_protocol=="http") || (m_protocol == "httpf") ) {
        struct servent *sent = getservbyname("http", "tcp");
	if (sent) {
	  port = ntohs(sent->s_port);
	} else
	  port = DEFAULT_HTTP_PORT;
      } else {
	kdebug( KDEBUG_INFO, 7103, "Got a weird protocol (%s), assuming port is 80", m_protocol.data()); 
	port = 80;
      }
  }
  m_request.port = port;
  m_request.user = user;
  m_request.passwd = pass;
  m_request.do_proxy = m_bUseProxy;
}

void HTTPProtocol::closeConnection( )
{
  m_request.hostname = QString::null;
  m_request.port = 0;
  m_request.user = QString::null;
  m_request.passwd = QString::null;
  m_request.do_proxy = QString::null;
  http_closeConnection();
}


void HTTPProtocol::buildURL()
{
  m_request.url = KURL();
  m_request.url.setProtocol( m_protocol);
  m_request.url.setUser( m_request.user );
  m_request.url.setPass( m_request.passwd );
  m_request.url.setHost( m_request.hostname );
  m_request.url.setPort( m_request.port );
  m_request.url.setPath( m_request.path );
  m_request.url.setQuery( m_request.query );
}

void HTTPProtocol::get( const QString& path, const QString& query, bool reload )
{
  if (m_request.hostname.isEmpty())
     error( KIO::ERR_INTERNAL, "http GET: No host specified!");

  m_request.method = HTTP_GET;
  m_request.path = path;
  m_request.query = query;
  m_request.reload = reload;
  m_request.offset = 0;
  m_request.do_proxy = m_bUseProxy;
  buildURL();

  ready();

  if (!http_open())
     return;
  
  if (!readHeader())
     return;

  if (!readBody())
     return;

  http_close();
  finished();
}

void HTTPProtocol::put( const QString& path, int, bool, bool)
{
  if (m_request.hostname.isEmpty())
     error( KIO::ERR_INTERNAL, "http PUT: No host specified!");

  m_request.method = HTTP_PUT;
  m_request.path = path;
  m_request.query = QString::null;
  m_request.reload = true;
  m_request.offset = 0;
  m_request.do_proxy = m_bUseProxy;
  buildURL();

  ready();

  if (!http_open())
     return;
  
  if (!readHeader())
     return;

  if (!readBody())
     return;

  http_close();
  finished();
}

void HTTPProtocol::post( const QString& path, const QString& query)
{
  if (m_request.hostname.isEmpty())
     error( KIO::ERR_INTERNAL, "http POST: No host specified!");

  m_request.method = HTTP_POST;
  m_request.path = path;
  m_request.query = query;
  m_request.reload = true;
  m_request.offset = 0;
  m_request.do_proxy = m_bUseProxy;
  buildURL();

  ready();

  if (!http_open())
     return;
  
  if (!readHeader())
     return;

  if (!readBody())
     return;

  http_close();
  finished();
}

void HTTPProtocol::mimetype( const QString& path )
{
  kdebug( KDEBUG_INFO, 7103, "http: mimetype(%s)", path.ascii());
  if (m_request.hostname.isEmpty())
     error( KIO::ERR_INTERNAL, "http MIMETYPE: No host specified!");

  m_request.method = HTTP_HEAD;
  m_request.path = path;
  m_request.query = QString::null;
  m_request.reload = false;
  m_request.offset = 0;
  m_request.do_proxy = m_bUseProxy;
  buildURL();

  ready();

  if (!http_open())
     return;
  
  if (!readHeader())
     return;

  kdebug( KDEBUG_INFO, 7103, "http: mimetype = %s",  m_strMimeType.ascii());

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
	QString path;
	QString query;
	stream >> path >> query;
	post( path, query );
      }
      break;
   default:
       assert(0);
   }
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
    kdebug(KDEBUG_INFO, 7103, "gets() failure on Chunk header");
    return -1;
  }
  // We could have got the CRLF of the previous chunk.
  // If so, try again.
  if (m_bufReceive[0] == '\0')
  {
     if (!gets(m_bufReceive.data(), m_bufReceive.size()-1))
     {
        kdebug(KDEBUG_INFO, 7103, "gets() failure on Chunk header");
        return -1;
     }
  }
  kdebug(KDEBUG_INFO, 7103, "Chunk header = \"%s\"", m_bufReceive.data());
  if (eof())
  {
     kdebug(KDEBUG_INFO, 7103, "EOF on Chunk header");
     return -1;
  }

  int chunkSize = strtol(m_bufReceive.data(), 0, 16);
  if ((chunkSize < 0) || (chunkSize > MAX_CHUNK_SIZE))
     return -1;

  kdebug(KDEBUG_INFO, 7103, "Chunk size = %d bytes", chunkSize);

  if (chunkSize == 0)
  {
    // Last chunk.
    // Skip trailers.
    do {
      // Skip trailer of last chunk.
      if (!gets(m_bufReceive.data(), m_bufReceive.size()-1))
      {
        kdebug(KDEBUG_INFO, 7103, "gets() failure on Chunk trailer");
        return -1;
      }
      kdebug(KDEBUG_INFO, 7103, "Chunk trailer = \"%s\"", m_bufReceive.data());
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

     kdebug(KDEBUG_INFO, 7103, "Read from chunk got %d bytes", bytesReceived);

     if (bytesReceived == -1)
        return -1; // Failure.

     totalBytesReceived += bytesReceived;
     bytesToReceive -= bytesReceived;

     kdebug(KDEBUG_INFO, 7103, "Chunk has %d bytes, %d bytes to go",
	totalBytesReceived, bytesToReceive);
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
     kdebug(KDEBUG_WARN, 7103, "Unbounded datastream on a Keep Alive connection!");
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
 * (if we are in the process of a Get request).
 */
bool HTTPProtocol::readBody( )
{
  QByteArray array;

  // Check if we need to decode the data.
  // If we are in copy mode the use only transfer decoding.
  bool decode = !m_qTransferEncodings.isEmpty() ||
                !m_qContentEncodings.isEmpty();

  bool useMD5 = !m_sContentMD5.isEmpty();

  // we are getting the following URL
  gettingFile(m_request.url.url());

  totalSize( m_iSize );
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
     return true;
  }

#ifdef DO_MD5
  MD5_CTX context;
  MD5_Init(&context);
#endif
  if (m_iSize)
    m_iBytesLeft = m_iSize;
  else
    m_iBytesLeft = 1;


  // this is the main incoming loop.  gather everything while we can...
  while (!eof()) {
    int bytesReceived;
    if (m_bChunked)
       bytesReceived = readChunked();
    else if (m_iSize)
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
    char *enc;
    // decode all of the transfer encodings
    while (!m_qTransferEncodings.isEmpty()) {
      enc = m_qTransferEncodings.pop();
      if (!enc)
	break;
      if ( strstr(enc, "gzip") ) {
	decodeGzip();
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
    while (!m_qContentEncodings.isEmpty()) {
      enc = m_qContentEncodings.pop();
      if (!enc)
	break;
      if ( strstr(enc, "gzip") ) {
	decodeGzip();
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
      kdebug( KDEBUG_INFO, 7103, "MD5 checksum mismatch : got %s , calculated %s", debugString(m_sContentMD5.left(f)),enc_digest);
      error(ERR_CHECKSUM_MISMATCH, m_state.url.url());
    } else {
      kdebug( KDEBUG_INFO, 7103, "MD5 checksum present, and hey it matched what I calculated.");
    }
  } else {
    kdebug( KDEBUG_INFO, 7103, "No MD5 checksum found.  Too Bad.");
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
  return true;
}

void HTTPProtocol::error( int _err, const QString &_text )
{
  m_bKeepAlive = false;
  http_close();
  return SlaveBase::error( _err, _text );
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
      kdebug( KDEBUG_WARN, 7103, "Can't communicate with cookiejar!" );
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
      kdebug( KDEBUG_WARN, 7103, "Can't communicate with cookiejar!" );
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

#define CACHE_REVISION "2\n"

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
   QCString u = m_request.url.url().ascii();
   for(int i = u.length(); i--;)
   {
      hash = (hash * 12211 + u[i]) % 2147483563;
   }

   QString hashString;
   hashString.sprintf("%08lx", hash);

   CEF = CEF + hashString;

   CEF = dir + "/" + CEF;

   FILE *fs = fopen( CEF.ascii(), "r");
   if (!fs)
      return 0;

   char buffer[40];
   bool ok = true;

  // CacheRevision
  if (ok && (!fgets(buffer, 40, fs)))
      ok = false;
   if (ok && (strcmp(buffer, CACHE_REVISION) != 0))
      ok = false;

   time_t date;
   time_t currentDate = time(0);

   // Creation Date
   if (ok && (!fgets(buffer, 40, fs)))
      ok = false;
   if (ok)
   {
      date = (time_t) strtoul(buffer, 0, 10);
      if (m_maxCacheAge && (difftime(currentDate, date) > m_maxCacheAge))
         ok = false; // Expired
   }

   // Expiration Date
   if (ok && (!fgets(buffer, 40, fs)))
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
   unlink( CEF.ascii());
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
   (void) ::mkdir( dir.ascii(), 0700 );

   QString filename = m_state.cef + ".new";  // Create a new cache entry

   m_fcache = fopen( filename.ascii(), "w");
   if (!m_fcache)
      return; // Error.

   fputs(CACHE_REVISION, m_fcache);    // Revision

   QString date;
   date.setNum( time(0) );
   fputs(date.ascii(), m_fcache);      // Creation date
   fputc('\n', m_fcache);

   date.setNum( expireDate );
   fputs(date.ascii(), m_fcache);      // Expire date
   fputc('\n', m_fcache);

   fputs(mimetype.ascii(), m_fcache);  // Mimetype
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
      kdebug( KDEBUG_WARN, 7103, "writeCacheEntry: writing %d bytes failed.", nbytes );
      fclose(m_fcache);
      m_fcache = 0;
      QString filename = m_state.cef + ".new";
      unlink( filename.ascii());
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
      if (::rename( filename.ascii(), m_state.cef.ascii()) == 0)
      {
         kdebug( KDEBUG_WARN, 7103, "closeCacheEntry: cache written correctly! (%s)", m_state.cef.ascii());
         return; // Success
      }
      kdebug( KDEBUG_WARN, 7103, "closeCacheEntry: error renaming cache entry.");
   }
   kdebug( KDEBUG_WARN, 7103, "closeCacheEntry: error closing cache entry.");
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

   int result = ::stat(cleanFile.ascii(), &stat_buf);
   if (result == -1)
   {
      int fd = creat( cleanFile.ascii(), 0666);
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
      utime(cleanFile.ascii(), 0);
      QCString dcopService;
      QString error;
      KService::startServiceByDesktopPath("http_cache_cleaner.desktop",
              QString::null, dcopService, error);
   }
}

extern "C" {
    SlaveBase *init_http() {
        return new HTTPProtocol(0, "http");
    }
    SlaveBase *init_httpf() {
        return new HTTPProtocol(0, "httpf");
    }
    SlaveBase *init_https() {
        return new HTTPProtocol(0, "https");
    }
}     
