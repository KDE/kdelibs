#include "http.h"

#include <kio_manager.h>
#include <kio_rename_dlg.h>
#include <kio_skip_dlg.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <assert.h>

#include <k2url.h>

bool open_CriticalDlg( const char *_titel, const char *_message, const char *_but1, const char *_but2 = 0L );
bool open_PassDlg( const char *_head, string& _user, string& _pass );

void sig_handler( int );
void sig_handler2( int );

int main( int argc, char **argv )
{
  signal(SIGCHLD,sig_handler);
  signal(SIGSEGV,sig_handler2);

  //ProtocolManager manager;

  Connection parent( 0, 1 );
  
  HTTPProtocol http( &parent );
  http.dispatchLoop();

  debug( "kio_http : Done" );
}

void sig_handler2( int )
{
  debug( "kio_http : ###############SEG FILE#############" );
  exit(1);
}

void sig_handler( int )
{
  int pid;
  int status;
    
  while( 1 )
  {
    pid = waitpid( -1, &status, WNOHANG );
    if ( pid <= 0 )
    {
      // Reinstall signal handler, since Linux resets to default after
      // the signal occured ( BSD handles it different, but it should do
      // no harm ).
      signal( SIGCHLD, sig_handler );
      return;
    }
  }
}

/*
 * We'll use an alarm that will set this flag when transfer has timed out
 */
char sigbreak = 0;

void sig_alarm(int signo)
{
  sigbreak = 1;
}            

void setup_alarm(unsigned int timeout)
{
  sigbreak = 0;
  alarm(timeout);
  signal(SIGALRM, sig_alarm);
}


/************************** Authorization stuff: copied from wget-source *****/

/* Encode a zero-terminated string in base64.  Returns the malloc-ed
   encoded line.  This is useful for HTTP only.

   Note that the string may not contain NUL characters.  */
char* base64_encode_line( const char *s )
{
   /* Conversion table. */
   static char tbl[64] = {
      'A','B','C','D','E','F','G','H',
      'I','J','K','L','M','N','O','P',
      'Q','R','S','T','U','V','W','X',
      'Y','Z','a','b','c','d','e','f',
      'g','h','i','j','k','l','m','n',
      'o','p','q','r','s','t','u','v',
      'w','x','y','z','0','1','2','3',
      '4','5','6','7','8','9','+','/'
   };
   int len, i;
   char *res;
   unsigned char *p;

   len = strlen(s);
    res = (char *)malloc(4 * ((len + 2) / 3) + 1);
   p = (unsigned char *)res;
   /* Transform the 3x8 bits to 4x6 bits, as required by
      base64.  */
   for (i = 0; i < len; i += 3)
   {
      *p++ = tbl[s[0] >> 2];
      *p++ = tbl[((s[0] & 3) << 4) + (s[1] >> 4)];
      *p++ = tbl[((s[1] & 0xf) << 2) + (s[2] >> 6)];
      *p++ = tbl[s[2] & 0x3f];
      s += 3;
   }
   /* Pad the result if necessary... */
   if (i == len + 1)
      *(p - 1) = '=';
   else if (i == len + 2)
      *(p - 1) = *(p - 2) = '=';
   /* ...and zero-teminate it.  */
   *p = '\0';
   return res;
}

char *create_generic_auth (const char *prefix, const char *user, const char *passwd )
{
   char *wwwauth;

   if (user && passwd)
   {
      char *t1, *t2;

      t1 = (char *)malloc(strlen(user) + 1 + 2 * strlen(passwd) + 1);
      sprintf(t1, "%s:%s", user, passwd);
      t2 = base64_encode_line(t1);
      free(t1);
      // wwwauth = (char *)malloc(strlen(t2) + 24);
      // sprintf(wwwauth, "Authorization: Basic %s\r\n", t2);
      wwwauth = (char *)malloc(strlen(t2) + strlen(prefix) + 11 /* UPDATE WHEN FORMAT BELOW CHANGES !!! */);
      sprintf(wwwauth, "%s: Basic %s\r\n", prefix, t2);
      free(t2);
   }
   else
      wwwauth = NULL;
   return(wwwauth);
}

char *create_www_auth(const char *user, const char *passwd)
{
  return create_generic_auth("Authorization",user, passwd);
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
        if ( *hptr != *nptr ) 
        {
            hptr = shptr; 
            // Try to find another domain or host in the list
            while ( --nptr>=nplist && *nptr!=',' && *nptr!=' ') 
                ;            
            while ( --nptr>=nplist && (*nptr==',' || *nptr==' '))
                ;
        } else {
            if ( nptr==nplist || nptr[-1]==',' || nptr[-1]==' ') 
            { 
                return 1;
            }
            hptr--;
            nptr--;
        }
    }
    
    return 0;
}

/*****************************************************************************/

HTTPProtocol::HTTPProtocol( Connection *_conn ) : IOProtocol( _conn )
{
  m_cmd = CMD_NONE;
  m_fsocket = 0L;
  m_sock = 0;
  m_bUseProxy = false;
  m_bIgnoreJobErrors = false;
  m_bIgnoreErrors = false;
  m_iSavedError = 0;
  m_iSize = 0;

  m_bCanResume = true; // most of http servers support resuming ?

  
  HTTP = HTTP_Unknown;
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

bool HTTPProtocol::http_open( K2URL &_url, const char* _post_data, int _post_data_size, bool _reload, unsigned long _offset )
{
  string url = _url.url();

  int port = _url.port();
  if ( port == -1 )
    port = 80;
  
  m_sock = ::socket(PF_INET,SOCK_STREAM,0);
  if ( m_sock < 0 )
  {
    error( ERR_COULD_NOT_CREATE_SOCKET, url.c_str() );
    return false;
  }

  int do_proxy = m_bUseProxy;
  if ( do_proxy && !m_strNoProxyFor.empty() ) 
    do_proxy = !revmatch( _url.host(), m_strNoProxyFor.c_str() );    

  if( do_proxy )
  {
    if( ::connect( m_sock, (struct sockaddr*)(&m_proxySockaddr), sizeof( m_proxySockaddr ) ) )
    {
      error( ERR_COULD_NOT_CONNECT, m_strProxyHost.c_str() );
      return false;
    }
  }
  else
  {
    struct sockaddr_in server_name;

    if( !initSockaddr( &server_name, _url.host(), port ) )
    {
      error( ERR_UNKNOWN_HOST, _url.host() );
      return false;
    }

    if( ::connect( m_sock, (struct sockaddr*)( &server_name ), sizeof( server_name ) ) )    
    {
      error( ERR_COULD_NOT_CONNECT, _url.host() );
      return false;
    }
  }
  
  m_fsocket = fdopen( m_sock, "r+" );
  if( !m_fsocket )
  {
    error( ERR_COULD_NOT_CONNECT, _url.host() );
    return false;
  }

  string command;
	
  if ( _post_data )
  {
    _reload = true;     /* no caching allowed */
    command = "POST ";
  }
  else
    command = "GET ";

  if( do_proxy )
  {
    char buffer[ 64 ];
    sprintf( buffer, ":%i", port );
    command += "http://";
    command += _url.host();
    command += buffer;
  }

  // Let the path be "/" if it is empty ( => true )
  string tmp = _url.encodedPathAndQuery( 0, true );
  command += tmp;
  
  command += " HTTP/1.1\r\n"; /* start header */
  command += "Connection: Close\r\n"; // Duh, we don't want keep-alive stuff quite yet.
  command += "User-Agent: "; /* User agent */
  command += getUserAgentString();
  command += "\r\n";

  if ( _offset > 0 )
  {
    char buffer[ 64 ];
    sprintf( buffer, "Range: bytes=%li-\r\n", _offset );
    command += buffer;
    debug( "kio_http : Range = %s", buffer );
  }

  if ( _reload ) /* No caching for reload */
  { 
    command += "Pragma: no-cache\r\n"; /* for HTTP/1.0 caches */
    command += "Cache-control: no-cache\r\n"; /* for HTTP/>=1.1 caches */
  }

  // Charset negotiation:
  if ( !m_strCharsets.empty() )
    command += "Accept-Charset: " + m_strCharsets + "\r\n";
	   
  // Language negotiation:
  if ( !m_strLanguages.empty() )
    command += "Accept-Language: " + m_strLanguages + "\r\n";
  
  command += "Host: "; /* support for virtual hosts and required by HTTP 1.1*/
  command += _url.host();
  if ( _url.port() != 0 )
  {
    char buffer[ 64 ];
    sprintf( buffer, ":%i", port );
    command += buffer;
  }
  command += "\r\n";
  
  if ( _post_data )
  {
    command += "Content-Type: application/x-www-form-urlencoded\r\nContent-Length: ";
    char buffer[ 64 ];
    sprintf( buffer, "%i\r\n", _post_data_size );
    command += buffer;
  }
 
  if( strlen( _url.user() ) != 0 )
  {
    char *www_auth = create_www_auth( _url.user(), _url.pass() );
    command += www_auth;
    free(www_auth);
  }
  
  if( do_proxy )
  {
    if( m_strProxyUser != "" && m_strProxyPass != "" )
    {
      char *www_auth = create_generic_auth("Proxy-authorization", m_strProxyUser.c_str(), m_strProxyPass.c_str() );
      command += www_auth;
      free(www_auth);
    }
  }

  command += "\r\n";  /* end header */
    
  int n;
repeat1:
  if ( ( n = write( m_sock, command.c_str(), command.size() ) ) != (int)command.size() )
  {
    if ( n == -1 && errno == EINTR )
      goto repeat1;    
    error( ERR_CONNECTION_BROKEN, _url.host() );
    return false;
  }
repeat2:
  if ( _post_data && ( n = write( m_sock, _post_data, _post_data_size ) != _post_data_size ) )
  {
    if ( n == -1 && errno == EINTR )
      goto repeat2;
    error( ERR_CONNECTION_BROKEN, _url.host() );
    return false;
  }
    
  // Jacek:
  // to get rid of those "Open with" dialogs...
  // however at least extensions should be checked
  m_strMimeType = "text/html";

  string realm;
  bool unauthorized = false;

  char buffer[ 1024 ];
  int len = 1;
  char* ret = 0L;
  while( len && ( ret = fgets( buffer, 1024, m_fsocket ) ) )
  { 
    len = strlen( buffer );
    while( len && (buffer[ len-1 ] == '\n' || buffer[ len-1 ] == '\r') )
      buffer[ --len ] = 0;
debug( "kio_http : Header: %s", buffer );
    if ( strncmp( buffer, "Accept-Ranges: none", 19 ) == 0 )
      m_bCanResume = false;
    
    
    else if ( strncmp( buffer, "Content-length: ", 16 ) == 0 || strncmp( buffer, "Content-Length: ", 16 ) == 0 )
      m_iSize = atol( buffer + 16 );
    else if ( strncmp( buffer, "Content-Type: ", 14 ) == 0 || strncmp( buffer, "Content-type: ", 14 ) == 0 )
    {
      // Jacek: We can't send mimeType signal now,
      // because there may be another Content-Type to come
      m_strMimeType = buffer + 14;
    }
    else if ( strncasecmp( buffer, "HTTP/1.0 ", 9 ) == 0 )
    {
      HTTP = HTTP_10;
      // Unauthorized access
      if ( strncmp( buffer + 9, "401", 3 ) == 0 )
	unauthorized = true;
      //Jacek: server error codes added (5xx)
      else if ( buffer[9] == '4' ||  buffer[9] == '5' )
      {
	// Tell that we will only get an error page here.
	errorPage();
      }
    } else if ( strncasecmp( buffer, "HTTP/1.1 ", 9 ) == 0 ) {
      HTTP = HTTP_11;
      // Unauthorized access
      if ( strncmp( buffer + 9, "401", 3 ) == 0 )
	unauthorized = true;
      //Jacek: server error codes added (5xx)
      else if ( buffer[9] == '4' ||  buffer[9] == '5' )
      {
	// Tell that we will only get an error page here.
	errorPage();
      }
    }
    // In fact we should do redirection only if we got redirection code
    else if ( strncmp( buffer, "Location:", 9 ) == 0 ) {
      http_close();
      fflush(stderr);
      K2URL u( _url, buffer + 10 );
      string url = u.url();
      redirection( url.c_str() );
      return http_open( u, _post_data, _post_data_size, _reload, _offset );
    } else if ( strncmp( buffer, "WWW-Authenticate:", 17 ) == 0 ) {
      const char *p = buffer + 17;
      while( *p == ' ' ) p++;
      if ( strncmp( p, "Basic", 5 ) != 0 ) continue;
      p += 5;
      while( *p == ' ' ) p++;
      if ( strncmp( p, "realm=\"", 7 ) != 0 ) continue;
      p += 7;
      int i = 0;
      while( p[i] != '"' ) i++;
      realm.assign( p, i );
    } else if (HTTP == HTTP_11) {
      if ( strncasecmp( buffer, "Transfer-Encoding: ", 19) == 0) {
	// If multiple encodings have been applied to an entity, the transfer-
	// codings MUST be listed in the order in which they were applied.
	QString tEncoding = buffer+19;
	if (tEncoding == "chunked") {
	  m_qTransferEncodings.push("chunked");
	  // Anyone know of a better way to handle unknown sizes possibly/ideally with unsigned ints?
	  m_iSize = 0;
	} else if (tEncoding == "gzip") {
	  m_qTransferEncodings.push("gzip");
	  m_iSize = 0;
	} else if (tEncoding == "identity") {
	  continue;  // Identy is the same as no encoding.. AFAIK
	} else {
	  fprintf(stderr, "Unknown encoding, or multiple encodings encountered.  Please write code.\n");
	  fflush(stderr);
	  abort();
	}
      } else if (strncasecmp(buffer, "Content-Encoding: ", 18) == 0) {
	QString tEncoding = buffer+18;
	if (tEncoding == "chunked") {
	  m_qContentEncodings.push("chunked");
	  // Anyone know of a better way to handle unknown sizes possibly/ideally with unsigned ints?
	  m_iSize = 0;
	} else if (tEncoding == "gzip") {
	  m_qContentEncodings.push("gzip");
	  m_iSize = 0;
	} else if (tEncoding == "identity") {
	  continue;  // Identy is the same as no encoding.. AFAIK
	} else {
	  fprintf(stderr, "Unknown encoding, or multiple encodings encountered.  Please write code.\n");
	  fflush(stderr);
	  abort();
	}
      }
    }
  }
  if ( unauthorized )
  {
    http_close();
    string user = _url.user();
    string pass = _url.pass();
    if ( realm.empty() )
      realm = _url.host();
    if ( !open_PassDlg( realm.c_str(), user, pass ) )
    {
      string url = _url.url();
      error( ERR_ACCESS_DENIED, url.c_str() );
      return false;
    }
    
    K2URL u( _url );
    u.setUser( user.c_str() );
    u.setPass( pass.c_str() );
    return http_open( u, _post_data, _post_data_size, _reload, _offset );
  }

  mimeType( m_strMimeType.c_str() );
  return true;
}


void HTTPProtocol::http_close()
{
  if ( m_sock )
    close( m_sock );
  m_sock = 0;
}


void HTTPProtocol::slotGetSize( const char *_url )
{
  string url = _url;
  
  K2URL usrc( _url );
  if ( usrc.isMalformed() )
  {
    error( ERR_MALFORMED_URL, url.c_str() );
    m_cmd = CMD_NONE;
    return;
  }

  if ( strcmp( usrc.protocol(), "http" ) != 0 )
  {
    error( ERR_INTERNAL, "kio_http got non http url" );
    m_cmd = CMD_NONE;
    return;
  }

  m_cmd = CMD_GET_SIZE;

  m_bIgnoreErrors = false;  
  if ( !http_open( usrc, 0L, 0, false ) )
  {
    m_cmd = CMD_NONE;
    return;
  }
  
  totalSize( m_iSize );
  http_close();

  finished();

  m_cmd = CMD_NONE;

}

string HTTPProtocol::getUserAgentString ()
{
  return "Konqueror/1.1";
}


void HTTPProtocol::slotGet( const char *_url )
{
  unsigned int old_len=0;
  string url = _url;
  
  K2URL usrc( _url );
  if ( usrc.isMalformed() )
  {
    error( ERR_MALFORMED_URL, url.c_str() );
    m_cmd = CMD_NONE;
    return;
  }

  if ( strcmp( usrc.protocol(), "http" ) != 0 )
  {
    error( ERR_INTERNAL, "kio_http got non http url" );
    m_cmd = CMD_NONE;
    return;
  }

  m_cmd = CMD_GET;

  m_bIgnoreErrors = false;  
  if ( !http_open( usrc, 0L, 0, false ) )
  {
    m_cmd = CMD_NONE;
    return;
  }
  
  ready();

  gettingFile( _url );
  
  int processed_size = 0;
  time_t t_start = time( 0L );
  time_t t_last = t_start;
  
  char buffer[ 2048 ];
  long nbytes=0;
  long sz =0;

  while (!feof(m_fsocket)) {
    nbytes = fread(buffer, 1, 2048, m_fsocket);
    if (nbytes > 0) {
      if (m_qTransferEncodings.isEmpty()) {
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

  
  http_close();


  if (!big_buffer.isNull()) {
    const char *enc;
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
  while (m_bLastChunk !=true && offset < big_buffer.size()) {
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
  // Note I haven't found an implementation
  // that correctly handles this stuff.  Apache
  // sends both Transfer-Encoding and Content-Length
  // headers, which is a no-no because the content
  // could really be bigger than the content-length
  // header implies.. like with gzip.
  // eek. This is no fun for progress indicators.
}

size_t HTTPProtocol::sendData()
{
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
  
  K2URL usrc( _source );
  if ( usrc.isMalformed() )
  {
    error( ERR_MALFORMED_URL, source.c_str() );
    m_cmd = CMD_NONE;
    return;
  }

  K2URL udest( _dest );
  if ( udest.isMalformed() )
  {
    error( ERR_MALFORMED_URL, dest.c_str() );
    m_cmd = CMD_NONE;
    return;
  }

  if ( strcmp( usrc.protocol(), "http" ) != 0 )
  {
    error( ERR_INTERNAL, "kio_http got non http protocol as source in copy command" );
    m_cmd = CMD_NONE;
    return;
  }

  list<K2URL> lst;
  if ( !K2URL::split( _dest, lst )  )
  {
    error( ERR_MALFORMED_URL, dest.c_str() );
    m_cmd = CMD_NONE;
    return;
  }
  if ( lst.size() > 1 )
  {
    error( ERR_NOT_FILTER_PROTOCOL, "http" );
    m_cmd = CMD_NONE;
    return;
  }
  
  string exec = ProtocolManager::self()->find( lst.back().protocol() );

  if ( exec.empty() )
  {
    error( ERR_UNSUPPORTED_PROTOCOL, lst.back().protocol() );
    m_cmd = CMD_NONE;
    return;
  }

  // Is the right most protocol a filesystem protocol ?
  if ( ProtocolManager::self()->outputType( lst.back().protocol() ) != ProtocolManager::T_FILESYSTEM )
  {
    error( ERR_PROTOCOL_IS_NOT_A_FILESYSTEM, lst.back().protocol() );
    m_cmd = CMD_NONE;
    return;
  }
      
  m_cmd = CMD_GET;

  list<string> files;
  files.push_back( _source );
  
  Slave slave( exec.c_str() );
  if ( slave.pid() == -1 )
  {
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
  for( ; fit != files.end(); fit++ )
  { 
    bool overwrite = false;
    bool skip_copying = false;
    bool resume = false;
    unsigned long offset = 0;

    K2URL u1( fit->c_str() );
    if ( u1.isMalformed() )
    {
      error( ERR_MALFORMED_URL, source.c_str() );
      m_cmd = CMD_NONE;
      return;
    }
      
      
    list<K2URL> l( lst );

    string filename = u1.filename();
    if ( filename.empty() ) {
      filename = "index.html";
      l.back().addPath( filename.c_str() );
    }
    
    string d;
    list<K2URL>::iterator it = l.begin();
    for( ; it != l.end(); it++ )
      d += it->url();
    
    // Repeat until we got no error
    do
    { 
      job.clearError();

      m_bIgnoreErrors = true;
      if ( !http_open( u1, 0L, 0, false, offset ) ) {
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
	if( tmpit == files.end() )
	{
	  open_CriticalDlg( "Error", tmp.c_str(), "Cancel" );
	  http_close();
	  clearError();
	  error( ERR_USER_CANCELED, "" );
	  m_cmd = CMD_NONE;
	  return;
	}
	
	if ( !open_CriticalDlg( "Error", tmp.c_str(), "Continue", "Cancel" ) )
	{
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

      // This is a hack, since total size should be the size of all files together
      // while we transmit only the size of the current file here.
      //totalSize( m_iSize + offset);

      canResume( m_bCanResume ); // this will emit sigCanResume( m_bCanResume )

      copyingFile( fit->c_str(), d.c_str() );
    
      job.put( d.c_str(), -1, overwrite_all || overwrite,
	       resume_all || resume, m_iSize + offset );

      while( !job.isReady() && !job.hasFinished() )
	job.dispatch();

      // Did we have an error ?
      if ( job.hasError() )
      {
	int currentError = job.errorId();

	debug( "kio_http : ################# COULD NOT PUT %d",currentError);
	if ( /* m_bGUI && */ currentError == ERR_WRITE_ACCESS_DENIED )
	{
	  // Should we skip automatically ?
	  if ( auto_skip )
	  {
	    job.clearError();
	    skip_copying = true;
	    continue;
	  }
	  string tmp2 = l.back().url();
	  SkipDlg_Result r;
	  r = open_SkipDlg( tmp2.c_str(), ( files.size() > 1 ) );
	  if ( r == S_CANCEL )
	  {
	    http_close();
	    error( ERR_USER_CANCELED, "" );
	    m_cmd = CMD_NONE;
	    return;
	  }
	  else if ( r == S_SKIP )
	  {
	    // Clear the error => The current command is not repeated => skipped
	    job.clearError();
	    skip_copying = true;
	    continue;
	  }
	  else if ( r == S_AUTO_SKIP )
	  {
	    // Clear the error => The current command is not repeated => skipped
	    job.clearError();
	    skip_copying = true;
	    continue;
	  }
	  else
	    assert( 0 );
	}
	// Can we prompt the user and ask for a solution ?
	else if ( /* m_bGUI && */ currentError == ERR_DOES_ALREADY_EXIST ||
		  currentError == ERR_DOES_ALREADY_EXIST_FULL )
	{    
	  // Should we skip automatically ?
	  if ( auto_skip )
	  {
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

	    string tmp2 = l.back().url();
	    r = open_RenameDlg( fit->c_str(), tmp2.c_str(), m, n );
	  }
	  if ( r == R_CANCEL ) 
	  {
	    http_close();
	    error( ERR_USER_CANCELED, "" );
	    m_cmd = CMD_NONE;
	    return;
	  }
	  else if ( r == R_RENAME )
	  {
	    K2URL u( n.c_str() );
	    // The Dialog should have checked this.
	    if ( u.isMalformed() )
	      assert( 0 );
	    // Change the destination name of the current file
// 	    l = lst;
// 	    l.back().addPath( filename.c_str() );
    
// 	    list<K2URL>::iterator it = l.begin();
// 	    for( ; it != l.end(); it++ )
// 	      d += it->url();

	    d = u.path();
	    // Dont clear error => we will repeat the current command
	  }
	  else if ( r == R_SKIP )
	  {
	    // Clear the error => The current command is not repeated => skipped
	    job.clearError();
	  }
	  else if ( r == R_AUTO_SKIP )
	  {
	    // Clear the error => The current command is not repeated => skipped
	    job.clearError();
	    auto_skip = true;
	  }
	  else if ( r == R_OVERWRITE )
	  {
	    overwrite = true;
	    // Dont clear error => we will repeat the current command
	  }
	  else if ( r == R_OVERWRITE_ALL )
	  {
	    overwrite_all = true;
	    // Dont clear error => we will repeat the current command
	  }
	  else if ( r == R_RESUME )
	  {
	    resume = true;
	    offset = getOffset( l.back().url() );
	    // Dont clear error => we will repeat the current command
	  }
	  else if ( r == R_RESUME_ALL )
	  {
	    resume_all = true;
	    offset = getOffset( l.back().url() );
	    // Dont clear error => we will repeat the current command
	  }
	  else
	    assert( 0 );
	}
	// No need to ask the user, so raise an error and finish
	else
	{    
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
    while( !feof( m_fsocket ) )
    {
      setup_alarm( ProtocolManager::self()->getReadTimeout() ); // start timeout
      long n = fread( buffer, 1, 2048, m_fsocket );

      // !!! slow down loop for local testing
//       for ( int tmpi = 0; tmpi < 800000; tmpi++ ) ;

      if ( n == -1 && !sigbreak )
      {
	http_close();
	error( ERR_CONNECTION_BROKEN, usrc.host() );
	m_cmd = CMD_NONE;
	return;
      }
   
      if ( n > 0 )
      {
	job.data( buffer, n );

	processed_size += n;
	time_t t = time( 0L );
	if ( t - t_last >= 1 )
        {
	  processedSize( processed_size );
	  speed( processed_size / ( t - t_start ) );
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
    if ( t - t_start >= 1 )
    {
      speed( processed_size / ( t - t_start ) );
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
  if ( m_bIgnoreErrors )
  {
    m_iSavedError = _err;
    m_strSavedError = _txt;
    return true;
  }
  else
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
  assert( 0 );
}


