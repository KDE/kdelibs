// $Id$

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LIBZ
#define DO_GZIP
#endif

#ifdef HAVE_SSL_H
//#define DO_SSL
#endif

#undef DO_SSL

#include <sys/types.h>
#include <sys/wait.h>

#include <assert.h>
#include <list.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <string>

#ifdef DO_SSL
#define MD5_CTX SSLeay_MD5_CTX
#include <ssl.h>
#include "/usr/local/include/err.h"
#undef MD5_CTX
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

#include <kio_rename_dlg.h>
#include <kio_skip_dlg.h>
#include <klocale.h>
#include <kprotocolmanager.h>
#include <ksock.h>
#include <kurl.h>

bool open_CriticalDlg( const char *_titel, const char *_message, const char *_but1, const char *_but2 = 0L );
bool open_PassDlg( const QString& _head, QString& _user, QString& _pass );

extern "C" {
  char *create_basic_auth (const char *header, const char *user, const char *passwd);
  char *create_digest_auth (const char *header, const char *user, const char *passwd, const char *auth_str);
  char *trimLead(char *);
  void sigalrm_handler(int);
#ifdef DO_SSL
  int verify_callback();
#endif
};

int main( int, char ** )
{
  signal(SIGCHLD, IOProtocol::sigchld_handler);
//  signal(SIGSEGV, IOProtocol::sigsegv_handler);

  Connection parent( 0, 1 );

  HTTPProtocol http( &parent );
  http.dispatchLoop();
}

char * trimLead (char *orig_string) {
  static unsigned int i=0; // I don't increment the string
                           // so that this can be called over
                           // and over
  while ( (*(orig_string+i) == ' ') || (*(orig_string+i) == ' ') )
    i++;
  return orig_string+i;
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
char *create_digest_auth (const char *header, const char *user, const char *passwd, const char *auth_str)
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
      fprintf(stderr, "Realm is :%s:\n", realm.c_str());
    } else if (strncasecmp(p, "algorith=\"", 10)==0) {
      p+=10;
      while (p[i] != '"' ) i++;
      algorithm.assign(p, i);
      fprintf(stderr, "Algorithm is :%s:\n", algorithm.c_str());
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

  return strdup(t1.data());
}
#else
char *create_digest_auth (const char *, const char *, const char *, const char *)
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

    t1 = (char *)malloc(strlen(user) +1+strlen(passwd));
    memset(t1, 0, strlen(user)+1+strlen(passwd));
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
int verify_callback ()
{
  return 1;
}
#endif

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

  m_bUseProxy = KProtocolManager::self().useProxy();

  if ( m_bUseProxy ) {
    KURL ur ( KProtocolManager::self().httpProxy() );

    m_strProxyHost = ur.host();
    m_strProxyPort = ur.port();
    m_strProxyUser = ur.user();
    m_strProxyPass = ur.pass();

    m_strNoProxyFor = KProtocolManager::self().noProxyFor();
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
}

#ifdef DO_SSL
void HTTPProtocol::initSSL() {
  m_bUseSSL2=true; m_bUseSSL3=true; m_bUseTLS1=false;
  m_bUseSSL=false;
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
    fprintf(stderr, "We've got a problem!\n");
    fflush(stderr);
  }
  SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, verify_callback);
  hand=SSL_new(ctx);
}
#endif

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
bool HTTPProtocol::http_open(KURL &_url, int _post_data_size, bool _reload,
                             unsigned long _offset )
{
  // let's store our current state
  m_state.url    = _url;
  m_state.reload = _reload;
  m_state.offset = _offset;
  m_state.postDataSize = _post_data_size;

  // Let's also clear out some things, so bogus values aren't used.
  m_sContentMD5 = "";
  m_HTTPrev = HTTP_Unknown;

  // try to ensure that the port is something reasonable
  unsigned short int port = _url.port();
  if ( port == 0 ) {
#ifdef DO_SSL
    if (_url.protocol()=="https") {
      struct servent *sent = getservbyname("https", "tcp");
      if (sent) {
        port = ntohs(sent->s_port);
      } else
        port = DEFAULT_HTTPS_PORT;
    } else
#endif
      if ( (_url.protocol()=="http") || (_url.protocol() == "httpf") ) {
        struct servent *sent = getservbyname("http", "tcp");
if (sent) {
	  port = ntohs(sent->s_port);
	} else
	  port = DEFAULT_HTTP_PORT;
      } else {
	fprintf(stderr, "Got a weird protocol (%s), assuming port is 80\n", _url.protocol().ascii()); fflush(stderr);
	port = 80;
      }
  }

  // make sure that we can support what we are asking for
  if (_url.protocol() == "https") {
#ifdef DO_SSL
    m_bUseSSL=true;
#else
    error(ERR_UNSUPPORTED_PROTOCOL, i18n("You do not have OpenSSL/SSLeay installed, or you have not compiled kio_http with SSL support"));
#endif
  }

  // okay, we know now that our URL is at least half-way decent.  let's
  // try to open up our socket
  m_sock = ::socket(PF_INET,SOCK_STREAM,0);
  if (m_sock < 0) {
    error( ERR_COULD_NOT_CREATE_SOCKET, _url.url() );
    return false;
  }

  // do we want to use a proxy?
  bool do_proxy = m_bUseProxy;

  // if so, we had first better make sure that our host isn't on the
  // No Proxy list
  if (do_proxy && !m_strNoProxyFor.isEmpty()) 
    do_proxy = !revmatch(_url.host(), m_strNoProxyFor);    
  
  // do we still want a proxy after all that?
  if( do_proxy ) {
    qDebug( "http_open 0");
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
    struct sockaddr_in server_name;

    if(!KSocket::initSockaddr(&server_name, _url.host(), port)) {
      error( ERR_UNKNOWN_HOST, _url.host() );
      return false;
    }

    if(::connect(m_sock, (struct sockaddr*)( &server_name ), sizeof(server_name))) {
      error(ERR_COULD_NOT_CONNECT, _url.host());
      return false;
    }
  }

  // Placeholder
  if (!openStream())
    error( ERR_COULD_NOT_CONNECT, _url.host() );

  // this will be the entire header
  QString header;

  // determine if this is a POST or GET method
  if (_post_data_size > 0) {
    _reload = true;     /* no caching allowed */
    header = "POST ";
  } else
    header = "GET ";

  // format the URI
  char c_buffer[64];
  memset(c_buffer, 0, 64);
  if(do_proxy) {
    sprintf(c_buffer, ":%u", port);
    header += "http://";
    header += _url.host();
    header += c_buffer;
  }

  // Let the path be "/" if it is empty ( => true )
  header += _url.encodedPathAndQuery(0, true);

  header += " HTTP/1.1\r\n"; /* start header */
  header += "Connection: Close\r\n"; // Duh, we don't want keep-alive stuff quite yet.
  header += "User-Agent: "; /* User agent */
  header += getUserAgentString();
  header += "\r\n";

  if ( _offset > 0 ) {
    sprintf(c_buffer, "Range: bytes=%li-\r\n", _offset);
    header += c_buffer;
    qDebug( "kio_http : Range = %s", c_buffer);
  }

  if ( _reload ) { /* No caching for reload */
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
  header += _url.host();
  if (_url.port() != 0) {
    memset(c_buffer, 0, 64);
    sprintf(c_buffer, ":%u", port);
    header += c_buffer;
  }
  header += "\r\n";

  if (_post_data_size > 0 ) {
    header += "Content-Type: application/x-www-form-urlencoded\r\nContent-Length: ";
    memset(c_buffer, 0, 64);
    sprintf(c_buffer, "%i\r\n", _post_data_size);
    header += c_buffer;
  }

  // check if we need to login
  if (_url.pass() ||_url.user()) {
    if (Authentication == AUTH_Basic) {
      header += create_basic_auth("Authorization", _url.user(),_url.pass());
    } else if (Authentication == AUTH_Digest) {
      header+= create_digest_auth("Authorization", _url.user(),
				  _url.pass(), m_strAuthString);
    }
    header+="\r\n";
  }

  // the proxy might need authorization of it's own. do that now
  if( do_proxy ) {
    qDebug( "http_open 3");
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
  header += "\r\n";  /* end header */

  // now that we have our formatted header, let's send it!
  if (write(header, header.length()) == -1) {
    error( ERR_CONNECTION_BROKEN, _url.host() );
    return false;
  }

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
  // to get rid of those "Open with" dialogs...
  // however at least extensions should be checked
  m_strMimeType = "text/html";
  // read in 1024 bytes at a time
  int len = 1;
  char buffer[1024];
  bool unauthorized = false;
  while(len && (gets(buffer, sizeof(buffer)))) { 
    // strip off \r and \n if we have them
    len = strlen(buffer);
    while(len && (buffer[len-1] == '\n' || buffer[len-1] == '\r'))
      buffer[--len] = 0;
    
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
      error(ERR_WARNING, trimLead(buffer + 8));
    }
		
    // oh no.. i think we're about to get a page not found
    else if (strncasecmp(buffer, "HTTP/1.0 ", 9) == 0) {
      m_HTTPrev = HTTP_10;

      // unauthorized access
      if (strncmp(buffer + 9, "401", 3) == 0) {
	unauthorized = true;
      } else if (buffer[9] == '4' || buffer[9] == '5') {
	// Let's first send an error message
	// this will be moved to slotErrorPage(), when it will be written
	http_close();
	error(ERR_ACCESS_DENIED, m_state.url.url());

	// Tell that we will only get an error page here.
	errorPage();

	return false;
      }
    }
		
    // this is probably not a good sign either... sigh
    else if (strncasecmp(buffer, "HTTP/1.1 ", 9) == 0) {
      m_HTTPrev = HTTP_11;
      Authentication = AUTH_None;

      // Unauthorized access
      if ((strncmp(buffer + 9, "401", 3) == 0) || (strncmp(buffer + 9, "407", 3) == 0)) {
	  unauthorized = true;
      }
      else if (buffer[9] == '4' || buffer[9] == '5') {
	// Tell that we will only get an error page here.
	errorPage();
      }
    }

    // In fact we should do redirection only if we got redirection code
    else if (strncmp(buffer, "Location:", 9) == 0 ) {
      http_close();
      KURL u(m_state.url, trimLead(buffer + 9));
      redirection(u.url());

      if ( !http_open(u, m_state.postDataSize, m_state.reload, m_state.offset) )
	return false;

      return readHeader();
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
	  /*m_bPersistant=false*/;
	} else if (strncasecmp(trimLead(buffer + 11), "Keep-Alive", 10)==0) {
	  /*m_bPersistant=true*/;
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
    memset(buffer, 0, 1024);
  }

  // DONE receiving the header!

  // we need to try to login again if we failed earlier
  if (unauthorized) {
    http_close();
    QString user = m_state.url.user();
    QString pass = m_state.url.pass();
    if (m_strRealm.isEmpty())
      m_strRealm = m_state.url.host();
    
    if (!open_PassDlg(m_strRealm, user, pass)) {
      error(ERR_ACCESS_DENIED, m_state.url.url());
      return false;
    }

    KURL u(m_state.url);
    u.setUser(user);
    u.setPass(pass);

    if ( !http_open(u, m_state.postDataSize, m_state.reload, m_state.offset) )
      return false;

    return readHeader();
  }
  
  // FINALLY, let the world know what kind of data we are getting
  // and that we do indeed have a header
  mimeType(m_strMimeType);

  return true;
}

void HTTPProtocol::addEncoding(QString encoding, QStack<char> *encs)
{
  // Identy is the same as no encoding
  if (encoding.lower() == "identity") {
    return;
  } else if (encoding.lower() == "chunked") {
    encs->push("chunked");
    // Anyone know of a better way to handle unknown sizes possibly/ideally with unsigned ints?
    if ( m_cmd != CMD_COPY )
      m_iSize = 0;
  } else if ((encoding.lower() == "x-gzip") || (encoding.lower() == "gzip") || (encoding.lower() == "x-deflate") || (encoding.lower() == "deflate")) {
    encs->push(strdup(encoding.lower()));
    if ( m_cmd != CMD_COPY )
      m_iSize = 0;
  } else {
    fprintf(stderr, "Unknown encoding encountered.  Please write code.\n");
    fflush(stderr);
    abort();
  }
}


bool HTTPProtocol::isValidProtocol (const char *p)
{
  if (strcasecmp(p, "https")==0) // Secure HTTP
    return true;
  if (strcasecmp(p, "httpf")==0) // Try to use WebDAV
    return true;
  if (strcasecmp(p, "http")==0)  // Standard HTTP
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
      assign=(char *)malloc(i);
      memcpy((void *)assign, (const void *)p, i);
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

void HTTPProtocol::http_close()
{
  if ( m_sock )
    close( m_sock );
  m_sock = 0;
}


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


const char *HTTPProtocol::getUserAgentString ()
{
  QString user_agent("kio_http ($Revision$)");
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

/**
 * This is one of the "big" functions in an ioslave -- slotGet() is
 * responsible for "getting" data from the remote server.  In the case
 * of HTTP, that means that this function will get called for the vast
 * majority of requests.  Pretty much the only major request that isn't
 * serviced through this function is the POST requests (handled by
 * slotPut).  This is called in response to the client sending a
 * KIOJob::get(const char* _url)
 *
 * The basic procedure is *very* simple now with a lot of the actual work
 * being done elsewhere:
 * 
 * 1) Make sure that this URL is valid
 * 2) Let the world know that we are doing a Get
 * 3) Start the process going with an http_open()
 * 4) Read the header
 * 5) Call slotDataEnd to get the data
 * 6) Close the connection
 */
void HTTPProtocol::slotGet( const char *_url )
{
  // transform this URL into a KURL for easy manipulating
  KURL usrc(_url);
  
  // make sure it is a "good" URL in more ways than one
  if (usrc.isMalformed()) {
    error(ERR_MALFORMED_URL, strdup(_url));
    m_cmd = CMD_NONE;
    return;
  }

  if (!isValidProtocol(&usrc)) {
    error(ERR_INTERNAL, "kio_http got non http/https/httpf url");
    m_cmd = CMD_NONE;
    return;
  }

  m_cmd = CMD_GET;
  
  m_bIgnoreErrors = false;  
  if (http_open(usrc, 0, false)) {
    if ( readHeader() )
      slotDataEnd();

    http_close();
    finished();
  }

  m_cmd = CMD_NONE;
}

/**
 * This is one of the other "big" functions in an ioslave -- slotPut()
 * is responsible for those requests where data needs to be sent or "put"
 * on the remote server.  In the case of HTTP, this means (mostly) POST
 * requests.  It's possible that WebDAV requests will also start from
 * here later.. but that's not the case now. This is called in response
 * to the client sending a KIOJob::put(...)
 *
 * 1) Make sure that this URL is valid
 * 2) Let the world know that we are doing a Put
 * 3) Start the process going with an http_open()
 * 4a)If we have something to post then call ready immediately to get
      the response
 * 4b)Otherwise we read the header
 * 5) Call slotDataEnd to get the content data
 * 6) Close the connection
 */
void HTTPProtocol::slotPut(const char *_url, int /*_mode*/, 
			   bool /*_overwrite*/,
                           bool /*_resume*/, int _len)
{
  // transform this URL into a KURL for easy manipulating
  KURL usrc(_url);

  // make sure it is a "good" URL in more ways than one
  if (usrc.isMalformed()) {
    error(ERR_MALFORMED_URL, strdup(_url));
    m_cmd = CMD_NONE;
    return;
  }

  if (!isValidProtocol(&usrc)) {
    error(ERR_INTERNAL, "kio_http got non http/https/httpf url");
    m_cmd = CMD_NONE;
    return;
  }

  // let's not be shy about our intentions... besides, we'll
  // need this later
  m_cmd = CMD_PUT;

  m_bIgnoreErrors = false;  
  if (http_open(usrc, _len, false)) {
    
    if ( _len > 0 )
      ready();
    else if ( readHeader() )
      slotDataEnd();

    http_close();
    finished();
  }
  m_cmd = CMD_NONE;
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
	memset(chunk_id, 0, 2);
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
      memset(chunk_id, 0, 2);
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

size_t HTTPProtocol::sendData( HTTPIOJob *job )
{
  // This was rendered necesary b/c
  // the IPC stuff can't handle
  // chunks much larger than 2048.

  IOProtocol *ioJob = job ? job:this;

  size_t sent=0;
  size_t bufferSize = 2048;
  size_t sz = big_buffer.size();
  processedSize(sz);
  totalSize(sz);
  while (sent+bufferSize < sz) {
    ioJob->data(big_buffer.data()+sent, bufferSize);
    sent+=bufferSize;
  }
  if (sent < sz)
    ioJob->data(big_buffer.data()+sent, (sz-sent));

  ioJob->dataEnd();
  m_cmd = CMD_NONE;
  return sz;
}

void HTTPProtocol::slotCopy( const char *_source, const char *_dest )
{
  QStringList lst;
  lst.append( _source );
  
  slotCopy( lst, _dest);
}

void HTTPProtocol::slotCopy( QStringList& _source, const char *_dest )
{
  //  IOProtocol::slotCopy(_source, _dest);

  fflush(stderr);

  KURL udest( _dest );
  if ( udest.isMalformed() ) {
    error( ERR_MALFORMED_URL, _dest );
    m_cmd = CMD_NONE;
    return;
  }

  if ( KURL::split( _dest ).count() != 1 )
  {
    error( ERR_NOT_FILTER_PROTOCOL, "http" );
    m_cmd = CMD_NONE;
    return;
  }
  
  QString exec = KProtocolManager::self().executable( udest.protocol() );

  if ( exec.isEmpty() ) {
    error( ERR_UNSUPPORTED_PROTOCOL, udest.protocol() );
    m_cmd = CMD_NONE;
    return;
  }

  // Is the right most protocol a filesystem protocol ?
  if ( KProtocolManager::self().outputType( udest.protocol() ) != KProtocolManager::T_FILESYSTEM ) {
    error( ERR_PROTOCOL_IS_NOT_A_FILESYSTEM, udest.protocol() );
    m_cmd = CMD_NONE;
    return;
  }
      
  m_cmd = CMD_COPY;

  Slave slave( exec );
  if ( slave.pid() == -1 ) {
    error( ERR_CANNOT_LAUNCH_PROCESS, exec );
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
  totalFiles( _source.count() );

  m_bIgnoreJobErrors = true;
  
  QStringList::Iterator fit = _source.begin();
  for( ; fit != _source.end(); fit++ ) { 
    bool overwrite = false;
    bool skip_copying = false;
    bool resume = false;
    unsigned long offset = 0;

    KURL u1( *fit );
    if ( u1.isMalformed() ) {
      error( ERR_MALFORMED_URL, *fit );
      m_cmd = CMD_NONE;
      return;
    }
      
    if (!isValidProtocol(&u1)) {
      error( ERR_INTERNAL, "kio_http got non http/https/httpf protocol as source in copy command" );
      m_cmd = CMD_NONE;
      return;
    }

    KURL ud( udest );

    if ( u1.filename().isEmpty() )
      ud.addPath( "index.html" );

    if ( ud.filename(false).isEmpty() ) {
      ud.addPath( u1.filename() );
    }

    QString d = ud.url();
    
    // Repeat until we got no error
    do { 
      job.clearError();

      m_bIgnoreErrors = true;
      if ( !http_open( u1, 0, false, offset ) ) {
	m_bIgnoreErrors = false;
	/* if ( !m_bGUI )
	{
          http_close();
	  releaseError();
	  m_cmd = CMD_NONE;
	  return;
	} */
	
	QString tmp = "Could not read\n";
	tmp += *fit;

	QStringList::Iterator tmpit = fit;
	tmpit++;
	if( tmpit == _source.end() ) {
	  open_CriticalDlg( "Error", tmp.latin1(), "Cancel" );
	  http_close();
	  clearError();
	  error( ERR_USER_CANCELED, "" );
	  m_cmd = CMD_NONE;
	  return;
	}
	
	if ( !open_CriticalDlg( "Error", tmp.latin1(), "Continue", "Cancel" ) ) {
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

      if ( readHeader() )
	{
	  // This is a hack, since total size should be the size of all files together
	  // while we transmit only the size of the current file here.
	  //	  totalSize( m_iSize + offset);

	  canResume( m_bCanResume ); // this will emit sigCanResume( m_bCanResume )

	  copyingFile( *fit, d );
    
	  job.put( d, -1, overwrite_all || overwrite,
		   resume_all || resume, m_iSize + offset );

	  while( !job.isReady() && !job.hasFinished() )
	    job.dispatch();

	  // Did we have an error ?
	  if ( job.hasError() ) {
	    int currentError = job.errorId();
	    
	    qDebug( "kio_http : ################# COULD NOT PUT %d",currentError);
	    if ( /* m_bGUI && */ currentError == ERR_WRITE_ACCESS_DENIED ) {
	      // Should we skip automatically ?
	      if ( auto_skip ) {
		job.clearError();
		skip_copying = true;
		continue;
	      }
	      QString tmp2 = ud.url();
	      SkipDlg_Result r;
	      r = open_SkipDlg( tmp2, ( _source.count() > 1 ) );
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
	      QString n;
	      
	      if ( KProtocolManager::self().autoResume() && m_bCanResume &&
		   currentError != ERR_DOES_ALREADY_EXIST_FULL ) {
		r = R_RESUME_ALL;
	      }
	      else {
		RenameDlg_Mode m;
		
		// ask for resume only if transfer can be resumed and if it is not
		// already fully downloaded
		if ( _source.count() > 1 ) {
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
		
		QString tmp2 = ud.url();
		r = open_RenameDlg( *fit, tmp2, m, n );
	      }
	      if ( r == R_CANCEL ) {
		http_close();
		error( ERR_USER_CANCELED, "" );
		m_cmd = CMD_NONE;
		return;
	      } else if ( r == R_RENAME ) {
		KURL u( n );
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
		offset = getOffset( ud.url() );
		// Dont clear error => we will repeat the current command
	      } else if ( r == R_RESUME_ALL ) {
		resume_all = true;
		offset = getOffset( ud.url() );
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
    }
    while( job.hasError() );
    
    if ( skip_copying )
      continue;
    
    slotDataEnd( &job );
    
    job.dataEnd();
    http_close();
    
    while( !job.hasFinished() )
      job.dispatch();
    //	  finished();
  }

  finished();
  m_cmd = CMD_NONE;
}

/**
 * This function is called in response to a client KIOJob::data(..)
 * request.  In practice, this is during a client "put" event.  The
 * procedure is like:
 * 
 * 1) Client sends KIOJob::put(..)
 * 2) HTTPProtocol::slotPut(...) gets called and, in the process,
 *    calls HTTPProtocol::http_open(...).
 * 3) HTTPProtocol::http_open(...) calls IOJob::ready()
 * 4) The client's slotReady() is called.  This function will execute
 *    KIOJob::data(..) with the data to send to the server
 * 5) This function is called with that data.  It sends it to the
 *    remote server.
 */
void HTTPProtocol::slotData(void *_p, int _len)
{
	// this *should* be for a PUT method.  make sure
	if (m_cmd != CMD_PUT) {
		abort();
		return;
	}

	// good.  now send our data to the remote server
	if (write(_p, _len) == -1) {
		error(ERR_CONNECTION_BROKEN, m_state.url.host());
		return;
	}
}

/**
 * This function is our "receive" function.  It is responsible for
 * downloading the message (not the header) from the HTTP server.  It
 * is called either as a response to a client's KIOJob::dataEnd()
 * (meaning that the client is done sending data) or by 'http_open()'
 * (if we are in the process of a Get request).
 */
void HTTPProtocol::slotDataEnd( HTTPIOJob *job )
{
        IOProtocol *ioJob = job ? job:this;

	// Check if we need to decode the data.
	// If we are in copy mode the use only transfer decoding.
	bool decode = !m_qTransferEncodings.isEmpty() ||
	              ( !m_qContentEncodings.isEmpty() &&
                        m_cmd != CMD_COPY );

	bool useMD5 = !m_sContentMD5.isEmpty();

	// we are getting the following URL
	gettingFile(m_state.url.url());

	totalSize( m_iSize );
	// get the starting time.  this is used later to compute the transfer
	// speed.
	time_t t_start = time(0L);
	time_t t_last = t_start;

	long nbytes = 0, sz = 0;
	char buffer[2048];
#ifdef DO_MD5
	MD5_CTX context;
	MD5Init(&context);
#endif
	// this is the main incoming loop.  gather everything while we can...
	while (!eof()) {
		// 2048 bytes seems to be a nice number of bytes to receive
		// at a time
		nbytes = read(buffer, 2048);

		// make sure that this wasn't an error, first
		if (nbytes == -1) {
			// erg.  oh well, log an error and bug out
			error(ERR_CONNECTION_BROKEN, m_state.url.host());
			m_cmd = CMD_NONE;
			break;
		}

		// i guess that nbytes == 0 isn't an error.. but we certainly
		// won't work with it!
		if (nbytes == 0)
			continue;

		// check on the encoding.  can we get away with it as is?
		if ( !decode ) {
#if DO_MD5
			if (useMD5)
				MD5Update(&context, (const unsigned char*)buffer, nbytes);
#endif
			// yep, let the world know that we have some data
			ioJob->data(buffer, nbytes);
			sz += nbytes;
			processedSize( sz );
			time_t t = time( 0L );
			if ( t - t_last >= 1 )
			{
			  speed( sz / ( t - t_start ) );
			  t_last = t;
			}
		} else {
			// nope.  slap this all onto the end of a big buffer
			// for later use
			unsigned int old_len = 0;
			old_len = big_buffer.size();
			big_buffer.resize(old_len + nbytes);
			memcpy(big_buffer.data() + old_len, buffer, nbytes);
		}
	}

	// if we have something in big_buffer, then we know that we have
	// encoded data.  of course, we need to do something about this
	if (!big_buffer.isNull()) {
		char *enc;
		// decode all of the transfer encodings
		while (!m_qTransferEncodings.isEmpty())	{
			enc = m_qTransferEncodings.pop();
			if (!enc)
				break;
			if ( strstr(enc, "gzip") )
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
		MD5Update(&context, (const unsigned char*)big_buffer.data(),
		          big_buffer.size());
#endif
		
		// now decode all of the content encodings
		while (!m_qContentEncodings.isEmpty()) {
			enc = m_qContentEncodings.pop();
			if (!enc)
				break;
			if ( strstr(enc, "gzip") )
				decodeGzip();
			else if (strncasecmp(enc, "chunked", 7)==0) {
				decodeChunked();
			}
		}
		sz = sendData(job);
	}

	// this block is all final MD5 stuff
#ifdef DO_MD5
	char buf[18], *enc_digest;
	MD5Final((unsigned char*)buf, &context); // Wrap everything up
	enc_digest = base64_encode_string(buf, 18);
	if ( useMD5 ) {
		int f;
		if ((f = m_sContentMD5.find("=")) <= 0)
			f = m_sContentMD5.length();

		if (m_sContentMD5.left(f) != enc_digest) {
			error(ERR_CHECKSUM_MISMATCH, m_state.url.url());
		} else {
			fprintf(stderr, "MD5 checksum present, and hey it matched what I calculated.\n");
		}
	} else 
		fprintf(stderr, "No MD5 checksum found.  Too Bad.\n");

	fflush(stderr);
	free(enc_digest);
#endif

	// FINALLY, we compute our final speed and let everybody know that we
	// are done
	t_last = time(0L);
	if (t_last - t_start)
	  speed(sz / (t_last - t_start));
	else;
	  speed(0);

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


