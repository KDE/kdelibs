// $Id$

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LIBZ
#define DO_GZIP
#endif

#ifdef HAVE_SSL_H
#define DO_SSL
#endif

#include <sys/types.h>
#include <sys/wait.h>

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#ifdef DO_SSL
#include <ssl.h>
#include "/usr/local/include/err.h"
#endif

#ifdef DO_MD5
#include <md5.h>
#endif
#include "extern_md5.h"

#ifdef DO_GZIP
#include <fcntl.h>
#include <zlib.h>
#endif

#include "http.h"

#include <kurl.h>
#include <kio_manager.h>
#include <kio_rename_dlg.h>
#include <kio_skip_dlg.h>

bool open_CriticalDlg( const char *_titel, const char *_message, const char *_but1, const char *_but2 = 0L );
bool open_PassDlg( const char *_head, string& _user, string& _pass );

extern "C" {
  char *create_basic_auth (const char *header, const char *user, const char *passwd);
  char *create_digest_auth (const char *header, const char *user, const char *passwd, const char *auth_str);
  void sigsegv_handler(int);
  void sigchld_handler(int);
  void sigalrm_handler(int);
};

int main( int argc, char **argv )
{
  signal(SIGCHLD, sigchld_handler);
  //signal(SIGSEGV, sigsegv_handler);

  Connection parent( 0, 1 );

  HTTPProtocol http( &parent );
  http.dispatchLoop();
}

void sigsegv_handler(int signo)
{
  // Debug and printf should be avoided because they might
  // call malloc.. and get in a nice recursive malloc loop
  write(2, "kio_http : ###############SEG FAULT#############\n", 51);
  exit(1);
}

void sigchld_handler(int signo)
{
  int pid, status;

  while(true) {
    pid = waitpid(-1, &status, WNOHANG);
    if ( pid <= 0 ) {
      // Reinstall signal handler, since Linux resets to default after
      // the signal occured ( BSD handles it different, but it should do
      // no harm ).
      signal(SIGCHLD, sigchld_handler);
      return;
    }
  }
}

/*
 * We'll use an alarm that will set this flag when transfer has timed out
 */
char sigbreak = 0;

void sigalrm_handler(int signo)
{
  sigbreak = 1;
}

void setup_alarm(unsigned int timeout)
{
  sigbreak = 0;
  alarm(timeout);
  signal(SIGALRM, sigalrm_handler);
}

char *create_digest_auth (const char *header, const char *user, const char *passwd, const char *auth_str)
{
  string domain, realm, algorithm, nonce, opaque, qop;
  const char *p=auth_str;
  int i;

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
      fprintf(stderr, "Realm is :%s:\n", realm.c_str());
    } else if (strncasecmp(p, "algorith=\"", 10)==0) {
      p+=10;
      while (p[i] != '"' ) i++;
      algorithm.assign(p, i);
      fprintf(stderr, "Algorith is :%s:\n", algorithm.c_str());
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

#ifdef DO_MD5
  HASHHEX HA1;
  HASHHEX HA2 = "";
  HASHHEX Response;
  char szNonceCount[9] = "00000001";
  DigestCalcHA1("md5", user, realm.c_str(), passwd, nonce.c_str(), 0, HA1);
  DigestCalcResponse(HA1, nonce.c_str(), szNonceCount, 0, "", "GET", domain.c_str(), HA2, Response);

  t1 += "response=\"";
  t1 += Response;
  t1 += "\", ";
#endif

  if (opaque != "") {
    t1 += "opaque=\"";
    t1 += opaque.c_str();
    t1 += "\" ";
  }

  t1 += "\r\n";

  return strdup(t1.data());
}

char *create_basic_auth (const char *header, const char *user, const char *passwd)
{
  char *wwwauth;
  if (user && passwd) {
    char *t1, *t2;

    t1 = (char *)malloc(strlen(user) +1+strlen(passwd));
    bzero(t1, strlen(user)+1+strlen(passwd));
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

/* Domain suffix match. E.g. return 1 if host is "cuzco.inka.de" and
   nplist is "inka.de,hadiko.de" or if host is "localhost" and
   nplist is "localhost" */
   
int revmatch(const char *host, const char *nplist)
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
                return 1;
            }
            hptr-=2;
        }
    }

    return 0;
}

extern "C" {
  int verify_callback();
}

int verify_callback ()
{
  return 1;
} 

/*****************************************************************************/

HTTPProtocol::HTTPProtocol( Connection *_conn ) : IOProtocol( _conn )
{
  m_cmd = CMD_NONE;
  m_fsocket = 0L;
  m_sock = 0;
  m_bIgnoreJobErrors = false;
  m_bIgnoreErrors = false;
  m_iSavedError = 0;
  m_iSize = 0;

  m_bCanResume = true; // most of http servers support resuming ?

  m_bUseProxy = ProtocolManager::self()->getUseProxy();

  if ( m_bUseProxy ) {
    KURL ur ( ProtocolManager::self()->getHttpProxy().data() );

    m_strProxyHost = ur.host();
    m_strProxyPort = ur.port();
    m_strProxyUser = ur.user();
    m_strProxyPass = ur.pass();

    m_strNoProxyFor = ProtocolManager::self()->getNoProxyFor().data();
  }

  m_bEOF=false;
#ifdef DO_SSL
  m_bUseSSL2=true; m_bUseSSL3=true; m_bUseTLS1=false;
  m_bUseSSL=false;
  if (m_bUseSSL2 && m_bUseSSL3)
    meth=SSLv23_client_method();
  else if (m_bUseSSL3)
    meth=SSLv3_client_method();
  else
    meth=SSLv2_client_method();
#ifdef SIGPIPE
  signal(SIGPIPE,SIG_IGN);
#endif
  SSLeay_add_all_algorithms();
  SSLeay_add_ssl_algorithms();
  ctx=SSL_CTX_new(meth);
  if (ctx == NULL) {
    fprintf(stderr, "We've got a problem!\n");
    fflush(stderr);
  }
  SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, verify_callback);
  hand=SSL_new(ctx);
  
#endif

  m_sContentMD5 = "";
  Authentication = AUTH_None;
  ProxyAuthentication = AUTH_None;

  m_HTTPrev = HTTP_Unknown;
}

bool HTTPProtocol::initSockaddr( struct sockaddr_in *server_name, const char *hostname, int port)
{
  struct hostent *hostinfo;
  server_name->sin_family = AF_INET;
  server_name->sin_port = htons( port );

  hostinfo = gethostbyname( hostname );

  if ( hostinfo == 0L )
    return false;

  server_name->sin_addr = *(struct in_addr*) hostinfo->h_addr;
  return true;
}


int HTTPProtocol::openStream() {
#ifdef DO_SSL
  if (m_bUseSSL) {
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
    return SSL_write(hand, buf, nbytes);
#endif
  return ::write(m_sock, buf, nbytes);
}

char *HTTPProtocol::gets (char *s, int size)
{
  int len=0;
  char *buf=s, mybuf[2]={0,0};
  while (len < size) {
    read(mybuf, 1);
    memcpy(buf, mybuf, 1);
    if (*buf == '\n')
      break;
    len++; buf++;
  }
  *buf=0;
  return s;
}

ssize_t HTTPProtocol::read (void *b, size_t nbytes)
{
  ssize_t ret;
#ifdef DO_SSL
  if (m_bUseSSL) {
    ret=SSL_read(hand, b, nbytes);
    if (ret==0) m_bEOF=true;
    return ret;
  }
#endif
  ret=fread(b, 1, nbytes, m_fsocket);
  if (!ret) m_bEOF=feof(m_fsocket);
  return ret;
}

bool HTTPProtocol::eof()
{
  return m_bEOF;
}

// It's reasonably safe to assume that it's a vaild protocol..
// altho it might be a good idea to add an assert trap somewhere
bool HTTPProtocol::http_open( KURL &_url, const char* _post_data, int _post_data_size, bool _reload, unsigned long _offset )
{
  int do_proxy, port = _url.port(), len=1;
  char c_buffer[64], f_buffer[1024];
  bool unauthorized = false;

  if ( port == -1 ) {
#ifdef DO_SSL
    if (strncasecmp(_url.protocol(), "https", 5)==0)
      port = DEFAULT_HTTPS_PORT;
    else
#endif
    if ((strncasecmp(_url.protocol(), "http", 4)==0) || (strncasecmp(_url.protocol(), "httpf", 5)==0))
	    port = DEFAULT_HTTP_PORT;

    else {
      fprintf(stderr, "Got a werid protocol (%s), assuming port is 80\n", _url.protocol());
      fflush(stderr);
      port=80;
    }
  }

#ifdef DO_SSL
  if (strncasecmp(_url.protocol(), "https", 5)==0)
    m_bUseSSL=true;
#endif

  m_sock = ::socket(PF_INET,SOCK_STREAM,0);
  if ( m_sock < 0 )  {
    error( ERR_COULD_NOT_CREATE_SOCKET, _url.url() );
    return false;
  }

  do_proxy = m_bUseProxy;
  bzero(c_buffer, 64);

  if ( do_proxy && !m_strNoProxyFor.empty() ) 
    do_proxy = !revmatch( _url.host(), m_strNoProxyFor.c_str() );    

  if( do_proxy ) {
    debug( "http_open 0");
    if( !initSockaddr( &m_proxySockaddr, m_strProxyHost.c_str(), m_strProxyPort ) ) {
	error( ERR_UNKNOWN_PROXY_HOST, m_strProxyHost.c_str() );
	return false;
      }

    if( ::connect( m_sock, (struct sockaddr*)(&m_proxySockaddr), sizeof( m_proxySockaddr ) ) ) {
      error( ERR_COULD_NOT_CONNECT, m_strProxyHost.c_str() );
      return false;
    }
  } else {
    struct sockaddr_in server_name;

    if( !initSockaddr( &server_name, _url.host(), port ) ) {
      error( ERR_UNKNOWN_HOST, _url.host() );
      return false;
    }

    if( ::connect( m_sock, (struct sockaddr*)( &server_name ), sizeof( server_name ) ) ) {
      error( ERR_COULD_NOT_CONNECT, _url.host() );
      return false;
    }
  }

  // Placeholder
  if (!openStream())
    error( ERR_COULD_NOT_CONNECT, _url.host() );

  string command;

  if ( _post_data ) {
    _reload = true;     /* no caching allowed */
    command = "POST ";
  }
  else
    command = "GET ";

  if( do_proxy ) {
    sprintf(c_buffer, ":%i", port);
    command += "http://";
    command += _url.host();
    command += c_buffer;
  }

  // Let the path be "/" if it is empty ( => true )
  QString tmp = _url.encodedPathAndQuery( 0, true );
  command += tmp;

  command += " HTTP/1.1\r\n"; /* start header */
  command += "Connection: Close\r\n"; // Duh, we don't want keep-alive stuff quite yet.
  command += "User-Agent: "; /* User agent */
  command += getUserAgentString();
  command += "\r\n";

  if ( _offset > 0 ) {
    sprintf(c_buffer, "Range: bytes=%li-\r\n", _offset);
    command += c_buffer;
    debug( "kio_http : Range = %s", c_buffer);
  }

  if ( _reload ) /* No caching for reload */ {
    command += "Pragma: no-cache\r\n"; /* for HTTP/1.0 caches */
    command += "Cache-control: no-cache\r\n"; /* for HTTP >=1.1 caches */
  }

#ifdef DO_GZIP
  // Content negotiation
  command += "Accept-Encoding: gzip, identity\r\n";
#endif

  // Charset negotiation:
  if ( !m_strCharsets.empty() )
    command += "Accept-Charset: " + m_strCharsets + "\r\n";
	   
  // Language negotiation:
  if ( !m_strLanguages.empty() )
    command += "Accept-Language: " + m_strLanguages + "\r\n";
  
  command += "Host: "; /* support for virtual hosts and required by HTTP 1.1 */
  command += _url.host();
  if (_url.port() != 0) {
    bzero(c_buffer, 64);
    sprintf(c_buffer, ":%i", port);
    command += c_buffer;
  }
  command += "\r\n";

  if (_post_data ) {
    command += "Content-Type: application/x-www-form-urlencoded\r\nContent-Length: ";
    bzero(c_buffer, 64);
    sprintf(c_buffer, "%i\r\n", _post_data_size);
    command += c_buffer;
  }

  if (_url.pass() ||_url.user()) {
    if (Authentication == AUTH_Basic){
      command += create_basic_auth("Authorization", _url.user(), _url.pass());
    } else if (Authentication == AUTH_Digest) {
      command+= create_digest_auth("Authorization", _url.user(), _url.pass(), m_strAuthString.c_str());
    }
    command+="\r\n";
  }

  if( do_proxy ) {
    debug( "http_open 3");
    if( m_strProxyUser != "" && m_strProxyPass != "" ) {
      if (ProxyAuthentication == AUTH_None || ProxyAuthentication == AUTH_Basic)
	command += create_basic_auth("Proxy-authorization", m_strProxyUser.c_str(), m_strProxyPass.c_str());
      else if (ProxyAuthentication == AUTH_Digest)
	command += create_digest_auth("Proxy-Authorization", m_strProxyUser.c_str(), m_strProxyPass.c_str(), m_strProxyAuthString.c_str());
    }
  }

  command += "\r\n";  /* end header */

  int n;
repeat1:
  if ( ( n = write(command.c_str(), command.size() ) ) != (int)command.size() ) {
    if ( n == -1 && errno == EINTR )
      goto repeat1;    
    error( ERR_CONNECTION_BROKEN, _url.host() );
    return false;
  }
repeat2:
  if ( _post_data && ( n = write(_post_data, _post_data_size ) != _post_data_size ) ) {
    if ( n == -1 && errno == EINTR )
      goto repeat2;
    error( ERR_CONNECTION_BROKEN, _url.host() );
    return false;
  }
    
  // Jacek:
  // to get rid of those "Open with" dialogs...
  // however at least extensions should be checked
  m_strMimeType = "text/html";

  while( len && (gets( f_buffer, 1024) ) ) { 
    len = strlen( f_buffer );
    while( len && (f_buffer[ len-1 ] == '\n' || f_buffer[ len-1 ] == '\r') )
      f_buffer[ --len ] = 0;

    debug( "kio_http : Header: %s", f_buffer );
    if ( strncmp( f_buffer, "Accept-Ranges: none", 19 ) == 0 )
      m_bCanResume = false;
    

    else if ( strncmp( f_buffer, "Content-length: ", 16 ) == 0 || strncmp( f_buffer, "Content-Length: ", 16 ) == 0 )
      m_iSize = atol( f_buffer + 16 );
    else if ( strncmp( f_buffer, "Content-Type: ", 14 ) == 0 || strncmp( f_buffer, "Content-type: ", 14 ) == 0 ) {
      // Jacek: We can't send mimeType signal now,
      // because there may be another Content-Type to come
      m_strMimeType = f_buffer + 14;
    }
    else if ( strncasecmp( f_buffer, "HTTP/1.0 ", 9 ) == 0 ) {
      m_HTTPrev = HTTP_10;
      // Unauthorized access
      if ( strncmp( f_buffer + 9, "401", 3 ) == 0 ) {
	unauthorized = true;
      } else if ( f_buffer[9] == '4' ||  f_buffer[9] == '5' ) {
	// Let's first send an error message
	// this will be moved to slotErrorPage(), when it will be written
	http_close();
	error( ERR_ACCESS_DENIED, _url.url() );
	return false;

	// Tell that we will only get an error page here.
	errorPage();
      }
    } else if ( strncasecmp(f_buffer, "HTTP/1.1 ", 9) == 0 ) {
      m_HTTPrev = HTTP_11;
      Authentication = AUTH_None;
      // Unauthorized access
      if ( (strncmp(f_buffer+9, "401", 3)==0) || (strncmp(f_buffer+9, "407", 3)==0) ) {
	unauthorized = true;
      } else if ( f_buffer[9] == '4' ||  f_buffer[9] == '5' ) {
	// Tell that we will only get an error page here.
	errorPage();
      }
    }
    // In fact we should do redirection only if we got redirection code
    else if ( strncmp(f_buffer, "Location:", 9) == 0 ) {
      http_close();
      KURL u( _url, f_buffer + 10 );
      redirection( u.url() );
      return http_open( u, _post_data, _post_data_size, _reload, _offset );
    } else if ( strncasecmp(f_buffer, "WWW-Authenticate:", 17) == 0 ) {
      configAuth(f_buffer+17, false);
    } else if ( strncasecmp(f_buffer, "Proxy-Authenticate:", 19) ==0 ) {
      configAuth(f_buffer+19, true);
    } else if (m_HTTPrev == HTTP_11) {
      if (strncasecmp(f_buffer, "Connection: ", 12) == 0) {
	if (strncasecmp(f_buffer+12, "Close", 5)==0)
	  /*m_bPersistant=false*/;
      } else if (strncasecmp(f_buffer, "Transfer-Encoding: ", 19) == 0) {
	// If multiple encodings have been applied to an entity, the transfer-
	// codings MUST be listed in the order in which they were applied.
	addEncoding(f_buffer+19, &m_qTransferEncodings);
      } else if (strncasecmp(f_buffer, "Content-Encoding: ", 18) == 0) {
	addEncoding(f_buffer+18, &m_qContentEncodings);
      } else if (strncasecmp(f_buffer, "Content-MD5: ", 13)==0) {
	m_sContentMD5 = strdup(f_buffer+13);
      }
    }
    bzero(f_buffer, 1024);
  }
  if (unauthorized) {
    http_close();
    string user = _url.user();
    string pass = _url.pass();
    if (m_strRealm.empty())
      m_strRealm = _url.host();
    if ( !open_PassDlg(m_strRealm.c_str(), user, pass) ) {
      error( ERR_ACCESS_DENIED, _url.url() );
      return false;
    }
    
    KURL u( _url );
    u.setUser( user.c_str() );
    u.setPass( pass.c_str() );
    return http_open( u, _post_data, _post_data_size, _reload, _offset );
  }

  mimeType( m_strMimeType.c_str() );
  return true;
}


void HTTPProtocol::addEncoding(QString encoding, QStack<char> *encs)
{
  if (encoding.lower() == "chunked") {
    encs->push("chunked");
    // Anyone know of a better way to handle unknown sizes possibly/ideally with unsigned ints?
    m_iSize = 0;
  } else if (encoding.lower() == "gzip") {
    encs->push("gzip");
    m_iSize = 0;
  } else if (encoding.lower() == "identity") {
    return;  // Identy is the same as no encoding
  } else {
    fprintf(stderr, "Unknown encoding encountered.  Please write code.\n");
    fflush(stderr);
    abort();
  }
}


bool HTTPProtocol::isValidProtocol (const char *p)
{
  if (strncasecmp(p, "http", 4)==0)  // Standard HTTP
    return true;
  if (strncasecmp(p, "https", 5)==0) // Secure HTTP
    return true;
  if (strncasecmp(p, "httpf", 5)==0) // Try to use WebDAV
    return true;
  return false;
}


bool HTTPProtocol::isValidProtocol (KURL *u)
{
  if (u)
    return isValidProtocol(u->protocol());
  else
    return false;
}


void HTTPProtocol::configAuth(const char *p, bool b)
{
  HTTP_AUTH f;
  char * strAuth=0;
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
    fprintf(stderr, "Invalid Authorization type requested\n");
    fprintf(stderr, "buffer: %s\n", p);
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
      m_strRealm.assign( p, i );
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

void HTTPProtocol::http_close()
{
  if ( m_sock )
    close( m_sock );
  m_sock = 0;
}


void HTTPProtocol::slotGetSize( const char *_url )
{
  KURL usrc( _url );
  if ( usrc.isMalformed() ) {
    error( ERR_MALFORMED_URL, _url );
    m_cmd = CMD_NONE;
    return;
  }

  if (!isValidProtocol(&usrc)) {
    error( ERR_INTERNAL, "kio_http got non http/https/httpf url" );
    m_cmd = CMD_NONE;
    return;
  }

  m_cmd = CMD_GET_SIZE;

  m_bIgnoreErrors = false;  
  if ( !http_open( usrc, 0L, 0, false ) ) {
    m_cmd = CMD_NONE;
    return;
  }
  
  totalSize( m_iSize );
  http_close();

  finished();

  m_cmd = CMD_NONE;
}


const char *HTTPProtocol::getUserAgentString ()
{
  QString user_agent("Konqueror/1.9.032899.3");
#ifdef DO_MD5
  user_agent+="; Supports MD5-Digest";
#endif
#ifdef DO_GZIP
  user_agent+="; Supports gzip encoding";
#endif
#ifdef DO_SSL
  user_agent+="; Supports SSL/HTTPS";
#endif
  return user_agent.data();
}


void HTTPProtocol::slotGet( const char *_url )
{
  unsigned int old_len=0;
  
  KURL usrc( _url );
  if ( usrc.isMalformed() ) {
    error( ERR_MALFORMED_URL, strdup(_url) );
    m_cmd = CMD_NONE;
    return;
  }

  if (!isValidProtocol(&usrc)) {
    error( ERR_INTERNAL, "kio_http got non http/https/httpf url" );
    m_cmd = CMD_NONE;
    return;
  }

  m_cmd = CMD_GET;

  m_bIgnoreErrors = false;  
  if ( !http_open( usrc, 0L, 0, false ) ) {
    m_cmd = CMD_NONE;
    return;
  }

  ready();

  gettingFile( _url );

  time_t t_start = time( 0L );
  time_t t_last = t_start;

  char buffer[ 2048 ];
  long nbytes=0, sz=0;

#ifdef DO_MD5
  char buf[18], *enc_digest;
  MD5_CTX context;
  MD5Init(&context);
#endif
  while (!eof()) {
    nbytes = read(buffer, 2048);
    if (nbytes > 0) {
      if (m_qTransferEncodings.isEmpty() && m_qContentEncodings.isEmpty()) {
#ifdef DO_MD5
	if (m_sContentMD5.c_str()) {
	  MD5Update(&context, (const unsigned char*)buffer, nbytes);
	}
#endif
	data(buffer, nbytes);
	sz+=nbytes;
      } else {
	old_len=big_buffer.size();
	big_buffer.resize(old_len+nbytes);
	memcpy(big_buffer.data()+old_len, buffer, nbytes);
      }
    }
    if (nbytes == -1) {
      //error( ERR_CONNECTION_BROKEN, usrc.host() );
      //m_cmd = CMD_NONE;
      break;
    }
  }

  http_close();  // Must we close the connection?

  if (!big_buffer.isNull()) {
    char *enc;
    while (!m_qTransferEncodings.isEmpty()) {
      enc = m_qTransferEncodings.pop();
      if (!enc)
	break;
      if (strncasecmp(enc, "gzip", 4)==0)
	decodeGzip();
      else if (strncasecmp(enc, "chunked", 7)==0) {
	decodeChunked();
      }
    }
    // From HTTP 1.1 Draft 6:
    // The MD5 digest is computed based on the content of the entity-body,
    // including any content-coding that has been applied, but not including
    // any transfer-encoding applied to the message-body. If the message is
    // received with a transfer-encoding, that encoding MUST be removed
    // prior to checking the Content-MD5 value against the received entity.
#ifdef DO_MD5
    MD5Update(&context, (const unsigned char*)big_buffer.data(), big_buffer.size());
#endif
    while (!m_qContentEncodings.isEmpty()) {
      enc = m_qContentEncodings.pop();
      if (!enc)
	break;
      if (strncasecmp(enc, "gzip", 4)==0)
	decodeGzip();
      else if (strncasecmp(enc, "chunked", 7)==0) {
	decodeChunked();
      }
    }
    sz = sendData();
  }

#ifdef DO_MD5
  MD5Final((unsigned char*)buf, &context); // Wrap everything up
  enc_digest = base64_encode_string(buf, 18);
  if (m_sContentMD5 != "" ) {
    int f;
    if ((f=m_sContentMD5.find("="))<=0)
      f=m_sContentMD5.length();
    if (strncmp(enc_digest, m_sContentMD5.c_str(), f)) {
      fprintf(stderr, "MD5 Checksums don't match.. oops?!:%d:%s:%s:\n", f,enc_digest, m_sContentMD5.c_str());
    } else
      fprintf(stderr, "MD5 checksum present, and hey it matched what I calculated.\n");
  } else 
    fprintf(stderr, "No MD5 checksum found.  Too Bad.\n");
  fflush(stderr);
  free(enc_digest);
#endif

  t_last = time(0L);
  if (t_last - t_start)
    speed( sz / ( t_last - t_start ) );
  else
    speed(0);
  finished();
  if (nbytes == -1) {
    error( ERR_CONNECTION_BROKEN, usrc.host() );
    m_cmd = CMD_NONE;
    return;
  }
}


void HTTPProtocol::decodeChunked()
{
  char chunk_id[2]={0,0};
  bool m_bLastChunk = false;
  long m_iLeftInChunk=-1;
  size_t offset=0;

  QByteArray ar;

  // A bytesleft of -1 indicates that a chunk has been read,
  // and we should check for a new one.  If the next chunk is of
  // zero length, we're done with the document.
  // Technically we're supposed to decode the end result of the first pass
  // and keep doing so until we've got a completely decoded document, but
  // I don't think that's really necesary
  // **
  // The chunk format basically consists of
  // length in hex+\n+data
  while (!m_bLastChunk && (offset < big_buffer.size())) {
    if (m_iLeftInChunk == -1) {
      QString s_length;
      while ((chunk_id[0] != '\r' && chunk_id[0] != '\n')) {
	bzero(chunk_id, 2);
	memcpy(chunk_id, big_buffer.data()+offset, 1); offset++;
	if (offset >= big_buffer.size()) {
	  m_iLeftInChunk=0; m_bLastChunk=true;
	} else
	  s_length.append(chunk_id);
      }
      
      offset++;

      // One extra read to catch the LF
      // Use strtol to convert from hex to dec, thanks to libwww
      // for that tidbit, but we should do error checking like
      // they do, alas I put too much trust in the web servers
      if (s_length.data()) {
	m_iLeftInChunk=strtol(s_length.data(), 0, 16);
      }
      if (m_iLeftInChunk <= 0) {
	m_bLastChunk=true;
	m_iLeftInChunk=0;
	break;
      }

      // Expand the buffer, and append the new data
      long old_size = ar.size();
      ar.resize(ar.size()+m_iLeftInChunk);
      memcpy(ar.data()+old_size, big_buffer.data()+offset, m_iLeftInChunk);

      // And then make sure to "seek" forward past any trailing garbage
      offset+=m_iLeftInChunk+2;
      m_iLeftInChunk = -1;
      m_bLastChunk = false;
      bzero(chunk_id,2);
    }
  }
  // Is this necesary?
  big_buffer.resize(0);
  big_buffer=ar;
  big_buffer.detach();
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
  unlink(filename); // If you want to inspect the raw data, comment this line out
#endif
}

size_t HTTPProtocol::sendData()
{
  // This was rendered necesary b/c
  // the IPC stuff can't handle
  // chunks much larger than 2048.

  size_t sent=0;
  size_t sz = big_buffer.size();
  processedSize(sz);
  totalSize(sz);
  while (sent+2048 < sz) {
    data(big_buffer.data()+sent, 2048);
    sent+=2048;
  }
  if (sent < sz)
    data(big_buffer.data()+sent, (sz-sent));
  dataEnd();;
  m_cmd = CMD_NONE;
  return sz;
}

void HTTPProtocol::slotCopy( const char *_source, const char *_dest )
{
  string source = _source;
  string dest = _dest;
  KURL usrc;
  
  usrc=KURL(_dest);
  if ( usrc.isMalformed() ) {
    error( ERR_MALFORMED_URL, dest.c_str() );
    m_cmd = CMD_NONE;
    return;
  }

  usrc=KURL( _source );
  if ( usrc.isMalformed() ) {
    error( ERR_MALFORMED_URL, source.c_str() );
    m_cmd = CMD_NONE;
    return;
  }

  if (!isValidProtocol(&usrc)) {
    error( ERR_INTERNAL, "kio_http got non http/https/httpf protocol as source in copy command" );
    m_cmd = CMD_NONE;
    return;
  }

  KURLList lst;
  if ( !KURL::split( _dest, lst )  ) {
    error( ERR_MALFORMED_URL, dest.c_str() );
    m_cmd = CMD_NONE;
    return;
  }
  if ( lst.count() > 1 ) {
    error( ERR_NOT_FILTER_PROTOCOL, "http" );
    m_cmd = CMD_NONE;
    return;
  }
  
  string exec = ProtocolManager::self()->find( lst.getLast()->protocol() );

  if ( exec.empty() ) {
    error( ERR_UNSUPPORTED_PROTOCOL, lst.getLast()->protocol() );
    m_cmd = CMD_NONE;
    return;
  }

  // Is the right most protocol a filesystem protocol ?
  if ( ProtocolManager::self()->outputType( lst.getLast()->protocol() ) != ProtocolManager::T_FILESYSTEM ) {
    error( ERR_PROTOCOL_IS_NOT_A_FILESYSTEM, lst.getLast()->protocol() );
    m_cmd = CMD_NONE;
    return;
  }
      
  m_cmd = CMD_GET;

  list<string> files;
  files.push_back( _source );
  
  Slave slave( exec.c_str() );
  if ( slave.pid() == -1 ) {
    error( ERR_CANNOT_LAUNCH_PROCESS, exec.c_str() );
    m_cmd = CMD_NONE;
    return;
  }
  
  HTTPIOJob job( &slave, this );

  int processed_size = 0;
    
  /*****
   * Copy files
   *****/

  time_t t_start = time( 0L );
  time_t t_last = t_start;

  bool overwrite_all = false;
  bool auto_skip = false;
  bool resume_all = false;

  int processed_files = 0;
  totalDirs( 0 );
  totalFiles( files.size() );

  m_bIgnoreJobErrors = true;
  
  list<string>::iterator fit = files.begin();
  for( ; fit != files.end(); fit++ ) { 
    bool overwrite = false;
    bool skip_copying = false;
    bool resume = false;
    unsigned long offset = 0;

    KURL u1( fit->c_str() );
    if ( u1.isMalformed() ) {
      error( ERR_MALFORMED_URL, source.c_str() );
      m_cmd = CMD_NONE;
      return;
    }
      
      
    KURLList l( lst );

    QString filename = u1.filename();
    if ( filename.isEmpty() ) {
      filename = "index.html";
      l.getLast()->addPath( filename );
    }
    
    QString d;
    KURL * it = l.first();
    for( ; it ; it = l.next() )
      d += it->url();
    
    // Repeat until we got no error
    do { 
      job.clearError();

      m_bIgnoreErrors = true;
      debug( "slotCopy 0");
      if ( !http_open( u1, 0L, 0, false, offset ) ) {
	debug( "slotCopy 1");
	m_bIgnoreErrors = false;
	/* if ( !m_bGUI )
	{
          http_close();
	  releaseError();
	  m_cmd = CMD_NONE;
	  return;
	} */
	
	string tmp = "Could not read\n";
	tmp += fit->c_str();

	list<string>::iterator tmpit = fit;
	tmpit++;
	if( tmpit == files.end() ) {
	  debug( "slotCopy 12");
	  open_CriticalDlg( "Error", tmp.c_str(), "Cancel" );
	  debug( "slotCopy 13");
	  http_close();
	  clearError();
	  error( ERR_USER_CANCELED, "" );
	  m_cmd = CMD_NONE;
	  return;
	}
	
	debug( "slotCopy 2");
	if ( !open_CriticalDlg( "Error", tmp.c_str(), "Continue", "Cancel" ) ) {
	  http_close();
	  clearError();
	  error( ERR_USER_CANCELED, "" );
	  m_cmd = CMD_NONE;
	  return;
	}
	else {
	  clearError();
	  continue;
	}
      }
      else
	m_bIgnoreErrors = false;

      debug( "slotCopy 3");
      // This is a hack, since total size should be the size of all files together
      // while we transmit only the size of the current file here.
      totalSize( m_iSize + offset);

      canResume( m_bCanResume ); // this will emit sigCanResume( m_bCanResume )

      copyingFile( fit->c_str(), d );
    
      job.put( d, -1, overwrite_all || overwrite,
	       resume_all || resume, m_iSize + offset );

      while( !job.isReady() && !job.hasFinished() )
	job.dispatch();

      // Did we have an error ?
      if ( job.hasError() ) {
	int currentError = job.errorId();

	debug( "kio_http : ################# COULD NOT PUT %d",currentError);
	if ( /* m_bGUI && */ currentError == ERR_WRITE_ACCESS_DENIED ) {
	  // Should we skip automatically ?
	  if ( auto_skip ) {
	    job.clearError();
	    skip_copying = true;
	    continue;
	  }
	  QString tmp2 = l.getLast()->url();
	  SkipDlg_Result r;
	  r = open_SkipDlg( tmp2, ( files.size() > 1 ) );
	  if ( r == S_CANCEL ) {
	    http_close();
	    error( ERR_USER_CANCELED, "" );
	    m_cmd = CMD_NONE;
	    return;
	  } else if ( r == S_SKIP ) {
	    // Clear the error => The current command is not repeated => skipped
	    job.clearError();
	    skip_copying = true;
	    continue;
	  } else if ( r == S_AUTO_SKIP ) {
	    // Clear the error => The current command is not repeated => skipped
	    job.clearError();
	    skip_copying = true;
	    continue;
	  } else
	    assert( 0 );
	}
	// Can we prompt the user and ask for a solution ?
	else if ( /* m_bGUI && */ currentError == ERR_DOES_ALREADY_EXIST ||
		  currentError == ERR_DOES_ALREADY_EXIST_FULL ) {    
	  // Should we skip automatically ?
	  if ( auto_skip ) {
	    job.clearError();
	    continue;
	  }

	  RenameDlg_Result r;
	  string n;

	  if ( ProtocolManager::self()->getAutoResume() && m_bCanResume &&
	       currentError != ERR_DOES_ALREADY_EXIST_FULL ) {
	    r = R_RESUME_ALL;
	  }
	  else {
	    RenameDlg_Mode m;

	    // ask for resume only if transfer can be resumed and if it is not
	    // already fully downloaded
	    if ( files.size() > 1 ) {
	      if ( m_bCanResume && currentError != ERR_DOES_ALREADY_EXIST_FULL )
		m = (RenameDlg_Mode)(M_MULTI | M_SKIP | M_OVERWRITE | M_RESUME);
	      else
		m = (RenameDlg_Mode)(M_MULTI | M_SKIP | M_OVERWRITE);
	    } else {
	      if ( m_bCanResume && currentError != ERR_DOES_ALREADY_EXIST_FULL )
		m = (RenameDlg_Mode)( M_SINGLE | M_OVERWRITE | M_RESUME);
	      else
		m = (RenameDlg_Mode)( M_SINGLE | M_OVERWRITE);
	    }

	    QString tmp2 = l.getLast()->url();
	    r = open_RenameDlg( fit->c_str(), tmp2, m, n );
	  }
	  if ( r == R_CANCEL ) {
	    http_close();
	    error( ERR_USER_CANCELED, "" );
	    m_cmd = CMD_NONE;
	    return;
	  } else if ( r == R_RENAME ) {
	    KURL u( n.c_str() );
	    // The Dialog should have checked this.
	    if ( u.isMalformed() )
	      assert( "The URL is malformed, something fucked up, you should never see this!" );
	    // Change the destination name of the current file
// 	    l = lst;
// 	    l.getLast()->addPath( filename.c_str() );
    
// 	    list<KURL>::iterator it = l.begin();
// 	    for( ; it != l.end(); it++ )
// 	      d += it->url();

	    d = u.path();
	    // Dont clear error => we will repeat the current command
	  } else if ( r == R_SKIP ) {
	    // Clear the error => The current command is not repeated => skipped
	    job.clearError();
	  } else if ( r == R_AUTO_SKIP ) {
	    // Clear the error => The current command is not repeated => skipped
	    job.clearError();
	    auto_skip = true;
	  } else if ( r == R_OVERWRITE ) {
	    overwrite = true;
	    // Dont clear error => we will repeat the current command
	  } else if ( r == R_OVERWRITE_ALL ) {
	    overwrite_all = true;
	    // Dont clear error => we will repeat the current command
	  } else if ( r == R_RESUME ) {
	    resume = true;
	    offset = getOffset( l.getLast()->url().data() );
	    // Dont clear error => we will repeat the current command
	  } else if ( r == R_RESUME_ALL ) {
	    resume_all = true;
	    offset = getOffset( l.getLast()->url().data() );
	    // Dont clear error => we will repeat the current command
	  } else
	    assert( "Unhandled command!" );
	}
	// No need to ask the user, so raise an error and finish
	else {    
	  http_close();
	  error( currentError, job.errorText() );
	  m_cmd = CMD_NONE;
	  return;
	}
      }
    }
    while( job.hasError() );

    if ( skip_copying )
      continue;

    if ( offset > 0 ) {
      // set offset
      processed_size += offset;
      debug( "kio_http : Offset = %ld", offset );
    }

    /**
     * Now we can really copy the stuff
     */
    char buffer[ 2048 ];
    int read_size = 0;
    while( !feof( m_fsocket ) ) {
      setup_alarm( ProtocolManager::self()->getReadTimeout() ); // start timeout
      long n = fread( buffer, 1, 2048, m_fsocket );

      // !!! slow down loop for local testing
//       for ( int tmpi = 0; tmpi < 800000; tmpi++ ) ;

      if ( n == -1 && !sigbreak ) {
	http_close();
	error( ERR_CONNECTION_BROKEN, usrc.host() );
	m_cmd = CMD_NONE;
	return;
      }
   
      if ( n > 0 ) {
	job.data( buffer, n );

	processed_size += n;
	read_size += n;
	time_t t = time( 0L );
	if ( t - t_last >= 1 ) {
	  processedSize( processed_size );
	  speed( read_size / ( t - t_start ) );
	  t_last = t;
	}
      }
    }

    job.dataEnd();
  
    http_close();

    while( !job.hasFinished() )
      job.dispatch();

    time_t t = time( 0L );
    
    processedSize( processed_size );
    if ( t - t_start >= 1 ) {
      speed( read_size / ( t - t_start ) );
      t_last = t;
    }
    processedFiles( ++processed_files );
  }
  
  debug( "kio_http : Copied files %s", dest.c_str() );

  finished();

  m_cmd = CMD_NONE;
}


void HTTPProtocol::jobError( int _errid, const char *_txt )
{
  if ( !m_bIgnoreJobErrors )
    error( _errid, _txt );
}


bool HTTPProtocol::error( int _err, const char *_txt )
{
  if ( m_bIgnoreErrors ) {
    m_iSavedError = _err;
    m_strSavedError = _txt;
    return true;
  } else
    return IOProtocol::error( _err, _txt );
}

/*************************************
 *
 * HTTPIOJob
 *
 *************************************/


HTTPIOJob::HTTPIOJob( Connection *_conn, HTTPProtocol *_HTTP ) : IOJob( _conn )
{
  m_pHTTP = _HTTP;
}

  
void HTTPIOJob::slotError( int _errid, const char *_txt )
{
  IOJob::slotError( _errid, _txt );
  
  m_pHTTP->jobError( _errid, _txt );
}


/*************************************
 *
 * Utilities
 *
 *************************************/

void openFileManagerWindow( const char * )
{
  assert( "???" );
}


