// $Id$

//#ifndef NDEBUG
//#define NDEBUG
//#endif

#include "ftp.h"

#include <errno.h>
#include <assert.h>

#include <kio_interface.h>
#include <kprotocolmanager.h>

#include <kdebug.h>
#include <ksock.h>
#include <kurl.h>

#define FTP_LOGIN "anonymous"
#define FTP_PASSWD "kfm-user@kde.org"

const char* strnextchr( const char * p , char c );
bool open_PassDlg( const QString&_head, QString& _user, QString& _pass );


Ftp::Ftp()
{
  dirfile = 0L;
  sControl = sData = sDatal = 0;
  ftplib_debug = 9;
  m_error = 0;
  m_errorText = "";
  m_bLoggedOn = false;
  m_bFtpStarted = false;
  m_bPersistent = true;
  kdebug(KDEBUG_INFO, 7102, "Ftp::Ftp()");
}


Ftp::~Ftp()
{
  ftpDisconnect();
}


/*
 * read a line of text
 *
 * return -1 on error or bytecount
 */
int Ftp::readline(char *buf,int max,netbuf *ctl)
{
  int x,retval = 0;
  char *end;
  int eof = 0;

  if ( max == 0 )
    return 0;
  do
  {
    if (ctl->cavail > 0)
    {
      x = (max >= ctl->cavail) ? ctl->cavail : max-1;
      end = (char*)memccpy(buf,ctl->cget,'\n',x);
      if (end != NULL)
	x = end - buf;
      retval += x;
      buf += x;
      *buf = '\0';
      max -= x;
      ctl->cget += x;
      ctl->cavail -= x;
      if (end != NULL)
	break;
    }
    if (max == 1)
    {
      *buf = '\0';
      break;
    }
    if (ctl->cput == ctl->cget)
    {
      ctl->cput = ctl->cget = ctl->buf;
      ctl->cavail = 0;
      ctl->cleft = FTP_BUFSIZ;
    }
    if (eof)
    {
      if (retval == 0)
	retval = -1;
      break;
    }
    if ((x = ::read(ctl->handle,ctl->cput,ctl->cleft)) == -1)
    {
      debug("read failed");
      retval = -1;
      break;
    }
    if (x == 0)
      eof = 1;
    ctl->cleft -= x;
    ctl->cavail += x;
    ctl->cput += x;
  }
  while (1);

  return retval;
}


/*
 * read a response from the server
 *
 * return false if first char doesn't match
 * return true if first char matches
 */
bool Ftp::readresp(char c)
{
  char match[5];
  if ( readline( rspbuf, 256, nControl ) == -1 )
  {
    if ( ftplib_debug > 1)
      kdebug( KDEBUG_ERROR, 0, "Could not read" );

    m_error = ERR_COULD_NOT_READ;
    m_errorText = "";
    return false;
  }
  if ( ftplib_debug > 1)
    kdebug(KDEBUG_INFO, 7102, "resp> %s",rspbuf);
  if ( rspbuf[3] == '-' )  {
    strncpy( match, rspbuf, 3 );
    match[3] = ' ';
    match[4] = '\0';
    do {
      if ( readline( rspbuf, 256, nControl ) == -1 ) {
	m_error = ERR_COULD_NOT_READ;
	m_errorText = "";
	return false;
      }
      if ( ftplib_debug > 1)
	kdebug(KDEBUG_INFO, 7102, "%s",rspbuf);
    }
    while ( strncmp( rspbuf, match, 4 ) );
  }
    	
  if ( rspbuf[0] == c )
    return true;

  return false;
}


/*
 * disconnect from remote
 */
void Ftp::ftpDisconnect( bool really )
{
  if ( m_bPersistent && !really )
    return;

  if ( m_bLoggedOn || m_bFtpStarted )
  {
    if( sControl != 0 )
    {
      ftpSendCmd( "quit", '2' );
      free( nControl );
      ::close( sControl );
      sControl = 0;
    }
  }

  m_bLoggedOn = false;
  m_bFtpStarted = false;
}


bool Ftp::ftpConnect( KURL& _url )
{
  QString dummy;
  return ftpConnect( _url.host(), _url.port(), _url.user(), _url.pass(), dummy );
}


/**
 * login on remote host
 */
bool Ftp::ftpConnect( const char *_host, unsigned short int _port, const char *_user, const char *_pass, QString& _path )
{
  m_bPersistent = KProtocolManager::self().persistentConnections();

  if ( m_bLoggedOn )
    if ( m_bPersistent ) {
      if ( m_host != _host )
        ftpDisconnect( true );
      else
      // this should check whether there is still opened data connection.
      // is it enough ?  Should we check also the control connection ?
      if ( ftpOpenDataConnection() ) {
	return true;
      } else {
	m_bLoggedOn = false;
      }

    } else {
      assert( !m_bLoggedOn );
    }


  _path = "";

  m_bFtpStarted = ftpConnect2( _host, _port );

  if ( !m_bFtpStarted ) {
    if ( !m_error ) {
      m_error = ERR_COULD_NOT_CONNECT;
      m_errorText = _host;
    }
    return false;
  }

  QString user;
  QString passwd;

  if( _user && strlen( _user ) > 0 ) {
    user = _user;
    if ( _pass && strlen( _pass ) > 0 ) {
      passwd = _pass;
    } else {
      passwd = "";
    }
  } else {
    user = FTP_LOGIN;
    passwd = FTP_PASSWD;
  }

  if ( ftplib_debug > 2 ) {
    kdebug(KDEBUG_INFO, 7102, "Connected ...." );
  }

  QString redirect = "";
  m_bLoggedOn = ftpLogin( user, passwd, redirect );
  if ( !m_bLoggedOn ) {
    if ( ftplib_debug > 2 ) {
      kdebug(KDEBUG_INFO, 7102, "Could not login" );
    }

    m_error = ERR_COULD_NOT_LOGIN;
    m_errorText = _host;
    return false;
  }

  // We could login and got a redirect ?
  if ( !redirect.isEmpty() ) {
    if ( redirect.right(1) != "/" ) {
      redirect += "/";
    }
    _path = redirect;

    if ( ftplib_debug > 2 ) {
      kdebug(KDEBUG_INFO, 7102, "REDIRECTION '%s'", redirect.ascii());
    }
  }

  m_bLoggedOn = true;
  return true;
}


/*
 * ftpOpen - connect to remote server
 *
 * return 1 if connected, 0 if not
 */
bool Ftp::ftpConnect2( const char *host, unsigned short int _port )
{
  struct sockaddr_in sin;
  struct servent *pse;
  int on = 1;

  m_host = "";

  memset( &sin, 0, sizeof( sin ) );

  if ( _port == 0 && ( pse = getservbyname( "ftp", "tcp" ) ) == NULL )
    _port = 21;
  else if ( _port == 0 )
    _port = ntohs(pse->s_port);

  if (!KSocket::initSockaddr(&sin, host, _port)) {
    m_error = ERR_UNKNOWN_HOST;
    m_errorText = host;
    return false;
  }

  sControl = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
  if ( sControl == 0 ) {
    m_error = ERR_COULD_NOT_CREATE_SOCKET;
    m_errorText = host;
  }
  if ( setsockopt( sControl, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on) ) == -1 )
  {
    ::close( sControl );
    m_error = ERR_COULD_NOT_CREATE_SOCKET;
    m_errorText = host;
  }

  if ( ::connect(sControl, (struct sockaddr *)&sin, sizeof(sin)) == -1)
  {
    ::close( sControl );
    m_error = ERR_COULD_NOT_CONNECT;
    m_errorText = host;
    return false;
  }

  nControl = (netbuf*)calloc(1,sizeof(netbuf));
  if (nControl == NULL)
  {
    ::close( sControl );
    m_error = ERR_OUT_OF_MEMORY;
    m_errorText = "";
    return false;
  }
  nControl->handle = sControl;

  if ( readresp( '2' ) == 0 )
  {
    ::close( sControl );
    free( nControl );
    return false;
  }

  m_host = host;

  return true;
}


/*
 * ftpLogin - log in to remote server
 *
 * return 1 if logged in, 0 otherwise
 */
bool Ftp::ftpLogin( const char *_user, const char *_pass, QString& _redirect )
{
  assert( !m_bLoggedOn );

  QString user = _user;
  QString pass = _pass;

  if ( !user.isEmpty() ) {
    QCString tempbuf = "user ";
    tempbuf += user;

    rspbuf[0] = '\0';

    if ( !ftpSendCmd( tempbuf, '3' ) ) {
      if ( ftplib_debug > 2 )
	kdebug(KDEBUG_INFO, 7102, "1> %s", rspbuf );

      if ( rspbuf[0] == '2' )
	return true; /* no password required */
      return false;
    }

    if ( pass.isEmpty() ) {
      QString tmp;
      tmp = user;
      tmp += "@";
      tmp += m_host;

      if ( !open_PassDlg( tmp, user, pass ) )
	return false;
    }
    kdebug( KDEBUG_INFO, 7102, "New pass is '%s'", pass.ascii());

    tempbuf = "pass ";
    tempbuf += pass;

    if ( !ftpSendCmd( tempbuf, '2' ) ) {
      kdebug(0, KDEBUG_INFO, "Wrong password");
      return false;
    }
  }

  kdebug( KDEBUG_INFO, 7102, "Login ok");

  // Okay, we're logged in. If this is IIS 4, switch dir listing style to Unix:
  // Thanks to jk@soegaard.net (Jens Kristian Søgaard) for this hint
  if( ftpSendCmd( "syst", '2' ) )
    if( !strncmp( rspbuf, "215 Windows_NT version", 22 ) ) // should do for any version
      ftpSendCmd( "site dirstyle", '2' );
  else
    kdebug(KDEBUG_WARN, 0, "syst failed");

  // Not interested in the current working directory ? => return with success
  if ( _redirect.isEmpty() )
    return true;

  kdebug( KDEBUG_INFO, 7102, "Searching for pwd");

  // Get the current working directory
  if ( !ftpSendCmd( "pwd", '2' ) )
    return false;

  if ( ftplib_debug > 2 )
    kdebug(KDEBUG_INFO, 7102, "2> %s", rspbuf );

  char *p = rspbuf;
  while ( isdigit( *p ) ) p++;
  while ( *p == ' ' || *p == '\t' ) p++;
  if ( *p != '"' )
    return true;
  char *p2 = strchr( p + 1, '"' );
  if ( p2 == 0L )
    return true;
  *p2 = 0;
  _redirect = p + 1;
  return true;
}


/*
 * ftpSendCmd - send a command and wait for expected response
 *
 * return 1 if proper response received, 0 otherwise
 */
bool Ftp::ftpSendCmd( const QCString& cmd, char expresp )
{
  assert( sControl > 0 );

  QCString buf = cmd;
  buf += "\r\n";

  if ( ftplib_debug > 2 )
    kdebug(KDEBUG_INFO, 7102, "%s", cmd.data() );

  if ( ::write( sControl, buf.data(), buf.length() ) <= 0 )  {
    m_error = ERR_COULD_NOT_WRITE;
    m_errorText = "";
    return false;
  }

  return readresp( expresp );
}

/*
 * ftpOpenPASVDataConnection - set up data connection, using PASV mode
 *
 * return 1 if successful, 0 otherwise
 * doesn't set error message, since non-pasv mode will always be tried if
 * this one fails
 */
bool Ftp::ftpOpenPASVDataConnection()
{
  int i[6], j;
  unsigned char n[6];
  int on=1;
  union {
    struct sockaddr sa;
    struct sockaddr_in in;
  } sin;
  struct linger lng = { 1, 120 };

  m_bPasv = true;
  sDatal = socket( AF_INET, SOCK_STREAM, 0 );
  if ( (setsockopt( sDatal,SOL_SOCKET,SO_REUSEADDR,(char*)&on, sizeof(on) ) == -1)
       || (sDatal < 0) )
  {
    ::close( sDatal );
    return false;
  }

  /* Let's PASsiVe*/
  if (!(ftpSendCmd("PASV",'2')))
  {
    ::close( sDatal );
    return false;
  }

  if (sscanf(rspbuf, "%*[^(](%d,%d,%d,%d,%d,%d)",&i[0], &i[1], &i[2], &i[3], &i[4], &i[5]) != 6)
  {
    ::close( sDatal );
    return false;
  }

  for (j=0; j<6; j++)
  {
    n[j] = (unsigned char) (i[j] & 0xff);
  }

  memset( &sin,0, sizeof(sin) );
  sin.in.sin_family = AF_INET;
  memcpy( &sin.in.sin_addr, &n[0], (size_t) 4 );
  memcpy( &sin.in.sin_port, &n[4], (size_t) 2 );

  if( ::connect( sDatal, &sin.sa, sizeof(sin) ) == -1)
  {
    ::close( sDatal );
    return false;
  }

  if ( setsockopt(sDatal, SOL_SOCKET,SO_KEEPALIVE, (char *) &on, (int) sizeof(on)) < 0 )
    if ( ftplib_debug > 1 ) debug("Keepalive not allowed");
  if ( setsockopt(sDatal, SOL_SOCKET,SO_LINGER, (char *) &lng,(int) sizeof (lng)) < 0 )
    if ( ftplib_debug > 1 ) debug("Linger mode was not allowed.");
  return 1;
}

/*
 * ftpOpenDataConnection - set up data connection
 *
 * return 1 if successful, 0 otherwise
 */
bool Ftp::ftpOpenDataConnection()
{
  assert( m_bLoggedOn );

  union
  {
    struct sockaddr sa;
    struct sockaddr_in in;
  } sin;

  struct linger lng = { 0, 0 };
  ksize_t l;
  char buf[64];
  int on = 1;

  ////////////// First try PASV mode

  if (ftpOpenPASVDataConnection())
    return true;

  ////////////// Fallback : non-PASV mode
  m_bPasv = false;

  l = sizeof(sin);
  if ( getsockname( sControl, &sin.sa, &l ) < 0 )
    return false;
  sDatal = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
  if ( sDatal == 0 )
  {
    m_error = ERR_COULD_NOT_CREATE_SOCKET;
    m_errorText = "";
    return false;
  }
  if ( setsockopt( sDatal, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on) ) == -1 )
  {
    ::close( sDatal );
    m_error = ERR_COULD_NOT_CREATE_SOCKET;
    m_errorText = "";
    return false;
  }
  if ( setsockopt( sDatal, SOL_SOCKET, SO_LINGER, (char*)&lng, sizeof(lng) ) == -1 )
  {
    ::close( sDatal );
    m_error = ERR_COULD_NOT_CREATE_SOCKET;
    m_errorText = "";
    return false;
  }

  sin.in.sin_port = 0;
  if ( bind( sDatal, &sin.sa, sizeof(sin) ) == -1 )
  {
    ::close( sDatal );
    sDatal = 0;
    m_error = ERR_COULD_NOT_BIND;
    m_errorText = m_host;
    return false;
  }

  if ( listen( sDatal, 1 ) < 0 )
  {
    m_error = ERR_COULD_NOT_LISTEN;
    m_errorText = m_host;
    ::close( sDatal );
    sDatal = 0;
    return 0;
  }

  if ( getsockname( sDatal, &sin.sa, &l ) < 0 )
    return false;

  sprintf(buf,"port %d,%d,%d,%d,%d,%d",
	  (unsigned char)sin.sa.sa_data[2],(unsigned char)sin.sa.sa_data[3],
	  (unsigned char)sin.sa.sa_data[4],(unsigned char)sin.sa.sa_data[5],
	  (unsigned char)sin.sa.sa_data[0],(unsigned char)sin.sa.sa_data[1]);

  return ftpSendCmd( buf, '2' );
}


/*
 * accept_connect - wait for incoming connection
 *
 * return -2 on error or timeout
 * otherwise returns socket descriptor
 */
int Ftp::ftpAcceptConnect(void)
{
  struct sockaddr addr;
  int sData;
  ksize_t l;
  fd_set mask;

  FD_ZERO(&mask);
  FD_SET(sDatal,&mask);

  if ( m_bPasv )
    return sDatal;
  if ( select( sDatal + 1, &mask, NULL, NULL, 0L ) == 0)
  {
    ::close( sDatal );
    return -2;
  }

  l = sizeof(addr);
  if ( ( sData = accept( sDatal, &addr, &l ) ) > 0 )
    return sData;

  ::close( sDatal );
  return -2;
}


bool Ftp::ftpPort()
{
  if ( !ftpSendCmd( "type A", '2' ) )
  {
    m_error = ERR_COULD_NOT_CONNECT;
    m_errorText = "";
    return false;
  }
  if ( !ftpOpenDataConnection() )
  {
    m_error = ERR_COULD_NOT_CONNECT;
    m_errorText = "";
    return false;
  }

  return true;
}


bool Ftp::ftpOpenCommand( const char *_command, const char *_path, char _mode, unsigned long _offset )
{
  QCString buf = "type ";
  buf += _mode;

  if ( !ftpSendCmd( buf.data(), '2' ) )
  {
    m_error = ERR_COULD_NOT_CONNECT;
    m_errorText = "";
    return false;
  }
  if ( !ftpOpenDataConnection() )
  {
    m_error = ERR_COULD_NOT_CONNECT;
    m_errorText = "";
    return false;
  }

  QCString tmp;

  // Special hack for the list command. We try to change to this
  // directory first to see whether it really is a directory.
  if ( strcmp( _command, "list" ) == 0 )
  {
    tmp = "cwd ";
    tmp += ( _path ) ? _path : "/";

    if ( !ftpSendCmd( tmp.data(), '2' ) )
    {
      if ( !m_error && rspbuf[0] == '5' )
      {
	m_error = ERR_IS_FILE;
	m_errorText = _path;
      }
      else if ( !m_error )
      {
        m_error = ERR_DOES_NOT_EXIST;
	m_errorText = _path;
      }
      return false;
    }
  } else if ( _offset > 0 ) {
    // send rest command if offset > 0, this applies to retr and stor commands
    char buf[100];
    sprintf(buf, "rest %ld", _offset);
    if ( !ftpSendCmd( buf, '3' ) ) {
      if ( ! m_error )
	{
	  m_error = ERR_CANNOT_RESUME;
	  m_errorText = _path;
	  return false;
	}
    }
  }

  tmp = _command;

  // only add path if it's not a list command
  // we are changing into this directory anyway, so it's enough just to send "list"
  // and this also works for symlinks
  if ( _path != 0L && strcmp( _command, "list" )) {
    tmp += " ";
    tmp += _path;
  }

  if ( !ftpSendCmd( tmp.data(), '1' ) ) {
    // We can not give any error code here since the error depends on the command
    return false;
  }

  if ( ( sData = ftpAcceptConnect() ) < 0 )
  {
    if ( !m_error )
    {
      m_error = ERR_COULD_NOT_ACCEPT;
      m_errorText = "";
      return false;
    }
  }

  return true;
}


bool Ftp::ftpCloseCommand()
{
  if( sData != 0 )
  {
    shutdown( sData, 2 );
    ::close( sData );
    sData = 0;
  }

  if( sDatal != 0 )
  {
    ::close( sDatal );
    sDatal = 0;
  }
  return true;
}


/*
 * ftpMkdir - create a directory at server
 *
 * return 1 if successful, 0 otherwise
 */
bool Ftp::ftpMkdir( const char *path )
{
  assert( m_bLoggedOn );

  QCString buf = "mkd ";
  buf += path;

  return ftpSendCmd( buf.data() , '2' );
}


/*
 * ftpChdir - change path at remote
 *
 * return 1 if successful, 0 otherwise
 */
bool Ftp::ftpChdir( const char *path)
{
  assert( m_bLoggedOn );

  QString buf = "cwd ";
  buf += path;

  return ftpSendCmd( buf.data(), '2' );
}


bool Ftp::ftpRmdir( const char *path)
{
  assert( m_bLoggedOn );

  QCString buf = "rmd ";
  buf += path;

  return ftpSendCmd( buf.data() ,'2' );
}


/*
 * ftpRename - rename a file at remote
 *
 * return 1 if successful, 0 otherwise
 */
bool Ftp::ftpRename( const char *src, const char *dst)
{
  assert( m_bLoggedOn );

  QCString cmd;
  cmd = "RNFR ";
  cmd += src;
  if ( !ftpSendCmd( cmd, '3') )
    return false;
  cmd = "RNTO ";
  cmd += dst;
  if ( !ftpSendCmd( cmd, '2' ) )
    return false;
  return true;
}


/*
 * ftpDelete - delete a file at remote
 *
 * return 1 if successful, 0 otherwise
 */
bool Ftp::ftpDelete( const char *fnm )
{
  assert( m_bLoggedOn );

  QCString cmd = "DELE ";
  cmd += fnm;
  return ftpSendCmd( cmd, '2' );
}


/*
 * ftpChmod - do chmod on a remote file
 *
 * return 1 if successful, 0 otherwise
 */
bool Ftp::ftpChmod( const char *src, int mode )
{
  assert( m_bLoggedOn );

  QCString cmd;
  cmd = "SITE CHMOD ";

  char buf[10];

  // we need to do bit AND 777 to get permissions
  sprintf(buf, "%o ", mode & 511 );

  cmd += buf;
  cmd += src;

  if ( !ftpSendCmd( cmd, '2' ) )
    return false;
  return true;
}


FtpEntry* Ftp::stat( KURL& _url )
{
  QString redirect;

  if( !ftpConnect( _url.host(), _url.port(), _url.user(), _url.pass(), redirect ) )
    // The error is already set => we just return
    return 0L;

  KURL url( _url );
  if ( !redirect.isEmpty() && !_url.hasPath() ) {
    url.setPath( redirect );
  }

  FtpEntry* e = ftpStat( _url );

  ftpDisconnect();

  return e;
}


FtpEntry* Ftp::ftpStat( KURL& _url )
{
  static FtpEntry fe;
  m_error = 0;

  kdebug( KDEBUG_INFO, 7102, "ftpStat : %s", _url.url().ascii());

  QString path = _url.path();
  if ( path.isEmpty() || path == "/" ) {
    fe.access = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    fe.type = S_IFDIR;
    fe.link = "";
    fe.name = "";
    fe.owner = "root";
    fe.group = "root";
    fe.date = 0;
    fe.size = 0;
    return &fe;
  }

  QString directory = _url.directory();
  if( !ftpOpenCommand( "list", directory, 'A' ) ) {
    kdebug(KDEBUG_ERROR, 0, "COULD NOT LIST");
    return 0L;
  }

  dirfile = fdopen( sData, "r" );
  if( !dirfile )
    return 0L;

  kdebug(KDEBUG_INFO, 7102, "Starting of list was ok");

  QString search = _url.filename();
  assert( search != "" && search != "/" );

  bool found = false;
  FtpEntry *e;
  while( ( e = readdir() ) ) //&& !found ) !!! fix - when not at and, don't read any response
  {
    if ( m_error ) {
      kdebug(KDEBUG_ERROR, 0, "FAILED: Read %d %s", m_error, errorText().ascii());
      return 0L;
    }

    if ( search == e->name ) {
      found = true;
      fe = *e;
    }

    kdebug(KDEBUG_INFO, 7102, "%s", e->name.ascii());
  }

  if ( !ftpCloseDir() )
    return 0L;

  if ( !found )
    return 0L;

  return &fe;
}


bool Ftp::opendir( KURL& _url )
{
  QString path( _url.path(-1) );
  bool haspath = _url.hasPath();
  QString redirect;

  if( !ftpConnect( _url.host(), _url.port(), _url.user(), _url.pass(), redirect ) )
    // The error is already set => we just return
    return false;

  // Did we get a redirect and did not we specify a path ourselfs ?
  if ( path != redirect && !haspath )
    redirection( _url.url() );
  else
    redirect = path;

  kdebug(KDEBUG_INFO, 7102, "hunting for path '%s'", redirect.ascii());

  KURL url( _url );
  url.setPath( redirect );

  return ftpOpenDir( url );
}


bool Ftp::ftpOpenDir( KURL& _url )
{
  QString path( _url.path(-1) );

  if( !ftpOpenCommand( "list", path, 'A' ) ) {
    kdebug(KDEBUG_ERROR, 0, "COULD NOT LIST %d %s", m_error, errorText().ascii() );
    return false;
  }

  dirfile = fdopen( sData, "r" );
  if( !dirfile )
    return false;

  kdebug(KDEBUG_INFO, 7102, "Starting of list was ok");

  return true;
}


FtpEntry *Ftp::readdir()
{
  char buffer[1024];

  while( fgets( buffer, sizeof(buffer), dirfile ) != 0 ) {
    FtpEntry* e = ftpParseDir( buffer );
    if ( e )
      return e;
  }
  return 0L;
}


FtpEntry* Ftp::ftpParseDir( char* buffer )
{
  QString tmp;

  static FtpEntry de;
  const char *p_access, *p_junk, *p_owner, *p_group;
  const char *p_size, *p_date_1, *p_date_2, *p_date_3, *p_name;
  if ((p_access = strtok(buffer," ")) != 0)
    if ((p_junk = strtok(NULL," ")) != 0)
      if ((p_owner = strtok(NULL," ")) != 0)
	if ((p_group = strtok(NULL," ")) != 0)
	  if ((p_size = strtok(NULL," ")) != 0)
	  {
	    // A special hack for "/dev". A listing may look like this:
	    // crw-rw-rw-   1 root     root       1,   5 Jun 29  1997 zero
	    // So we just ignore the number in front of the ",". Ok, its a hack :-)
	    if ( strchr( p_size, ',' ) != 0L )
	      if ((p_size = strtok(NULL," ")) == 0)
		return 0L;
	    if ((p_date_1 = strtok(NULL," ")) != 0)
	      if ((p_date_2 = strtok(NULL," ")) != 0)
		if ((p_date_3 = strtok(NULL," ")) != 0)
		  if ((p_name = strtok(NULL,"\r\n")) != 0)
		  {
		    if ( p_access[0] == 'l' )
		    {
		      tmp = p_name;
		      int i = tmp.findRev( " -> " );
		      if ( i != -1 ) {
			de.link = p_name + i + 4;
			tmp.truncate( i );
			p_name = tmp;
		      }
		      else
			de.link = "";
		    }
		    else
		      de.link = "";

		    de.access = 0;
		    de.type = S_IFREG;
		    if ( p_access[0] == 'd' )
		      de.type = S_IFDIR;
		    else if ( p_access[0] == 's' )
		      de.type = S_IFSOCK;
		    else if ( p_access[0] == 'b' )
		      de.type = S_IFBLK;
		    else if ( p_access[0] == 'c' )
		      de.type = S_IFCHR;
		    else if ( p_access[0] == 'l' )
		      de.type = S_IFDIR;
        // links on ftp sites are often links to dirs, and we have no way to check that
        // let's do like Netscape : assume dirs (David)
        // we don't set S_IFLNK here.  de.link says it.
		
		    if ( p_access[1] == 'r' )
		      de.access |= S_IRUSR;
		    if ( p_access[2] == 'w' )
		      de.access |= S_IWUSR;
		    if ( p_access[3] == 'x' )
		      de.access |= S_IXUSR;
		    if ( p_access[4] == 'r' )
		      de.access |= S_IRGRP;
		    if ( p_access[5] == 'w' )
		      de.access |= S_IWGRP;
		    if ( p_access[6] == 'x' )
		      de.access |= S_IXGRP;
		    if ( p_access[7] == 'r' )
		      de.access |= S_IROTH;
		    if ( p_access[8] == 'w' )
		      de.access |= S_IWOTH;
		    if ( p_access[9] == 'x' )
		      de.access |= S_IXOTH;

		    de.owner	= p_owner;
		    de.group	= p_group;
		    de.size	= atoi(p_size);
		    // QString tmp( p_name );
		    de.name	= p_name; /* tmp.stripWhiteSpace(); */

                    // Parsing the date is somewhat tricky
                    // Examples : "Oct  6 22:49", "May 13  1999"

                    // First get current time - we need the current year
                    time_t currentTime = time( 0L );
                    struct tm * tmptr = gmtime( &currentTime );
                    //kdebug( KDEBUG_INFO, 7102, "Current time :%s", asctime( tmptr ) );
                    // Reset time fields
                    tmptr->tm_sec = 0;
                    tmptr->tm_min = 0;
                    tmptr->tm_hour = 0;
                    // Get day number (always second field)
                    tmptr->tm_mday = atoi( p_date_2 );
                    // Get month from first field
                    // NOTE : no, we don't want to use KLocale here
                    // It seems all FTP servers use the English way
                    static const char * s_months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                                       "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
                    for ( int c = 0 ; c < 12 ; c ++ )
                      if ( !strcmp( p_date_1, s_months[c]) )
                      {
                        tmptr->tm_mon = c;
                        break;
                      }

                    // Parse third field
                    if ( strlen( p_date_3 ) == 4 ) // 4 digits, looks like a year
                      tmptr->tm_year = atoi( p_date_3 ) - 1900;
                    else
                    {
                      // otherwise, it's the current year,
                      // and p_date_3 contains probably a time
                      char * semicolon;
                      if ( ( semicolon = strchr( p_date_3, ':' ) ) )
                      {
                        *semicolon = '\0';
                        tmptr->tm_min = atoi( semicolon + 1 );
                        tmptr->tm_hour = atoi( p_date_3 );
                      }
                      else
                        kdebug( KDEBUG_WARN, 7102, "Can't parse third field %s", p_date_3 );
                    }

                    //kdebug( KDEBUG_INFO, 7102, asctime( tmptr ) );
                    de.date = mktime( tmptr );
		    return( &de );
		  }
	  }
  return 0L;
}


bool Ftp::closedir()
{
  if( dirfile )
  {
    ftpCloseDir();
    fclose( dirfile );
    dirfile = NULL;

    ftpDisconnect();
  }
  return true;
}


bool Ftp::ftpCloseDir()
{
  kdebug(KDEBUG_INFO, 7102, "... closing");

  if ( !readresp( '2' ) ) {
    kdebug(KDEBUG_INFO, 7102, "Did not get transfer complete message");
    return false;
  }

  return ftpCloseCommand();
}


bool Ftp::open( KURL& _url, Ftp::Mode mode )
{
  QString redirect;

  if( !ftpConnect( _url.host(), _url.port(), _url.user(), _url.pass(), redirect ) )
    // The error is already set => just quit
    return false;

  return ftpOpen( _url, mode );
}


bool Ftp::ftpOpen( KURL& _url, Ftp::Mode mode, unsigned long offset )
{
  if( mode & READ ) {
    ftpSize( _url.path(),'I'); // try to find the size of the file

    if ( !ftpOpenCommand( "retr", _url.path(), 'I', offset ) ) {
      if ( ! m_error )
	{
	  kdebug(KDEBUG_WARN, 0, "Can't open for reading");
	  m_error = ERR_CANNOT_OPEN_FOR_READING;
	  m_errorText = _url.url();
	}
      return false;
    }

    // Read the size from the response string
    if ( strlen( rspbuf ) > 4 && m_size == 0 ) {
      // char *p = strchr( rspbuf, '(' );
      // Patch from Alessandro Mirone <alex@greco2.polytechnique.fr>
      const char *p = rspbuf;
      const char *oldp = 0L;
      while ( *( p = strnextchr( p , '(' ) ) == '(' )
      {
	oldp = p;
	p++;
      }
      p = oldp;
      // end patch
    }

    m_bytesLeft = m_size - offset;
    return true;
  }
  else if( mode & WRITE ) {

    if( !ftpOpenCommand( "stor", _url.path(), 'I', offset ) ) {
      if ( !m_error )
	{
	  m_error = ERR_COULD_NOT_WRITE;
	  m_errorText = _url.url();
	}
      return false;
    }

    return true;
  }

  // Never reached
  assert( 0 );
  return true;
}


bool Ftp::ftpClose()
{
  kdebug(KDEBUG_INFO, 7102, "... closing");

  // first close, then read response ( should be 226 )

  bool tmp = ftpCloseCommand();

  if ( !readresp( '2' ) ) {
      kdebug(KDEBUG_INFO, 7102, "Did not get transfer complete message");
      return false;
    }

  return tmp;
}


bool Ftp::ftpResume( unsigned long offset )
{
  char buf[64];
  sprintf(buf, "rest %ld", offset);
  if ( !ftpSendCmd( buf, '3' ) ) {
    m_error = ERR_CANNOT_RESUME;
    return false;
  }

  return true;
}


/** Use the SIZE command to get the file size. David.
    Warning : the size depends on the transfer mode, hence the second arg. */
bool Ftp::ftpSize( const char *path, char mode)
{
  QCString buf;
  buf.sprintf("type %c", mode);
  if ( !ftpSendCmd( buf, '2' ) ) {
      m_error = ERR_COULD_NOT_CONNECT;
      m_errorText = "";
      return false;
  }

  buf="SIZE ";
  buf+=path;
  if (!ftpSendCmd(buf,'2')) {
    m_size = 0;
    return false;
  }

  m_size = atol(rspbuf+4); // skip leading "213 " (response code)

  return true;
}


bool Ftp::close()
{
  ftpCloseCommand();

  ftpDisconnect();

  return true;
}


size_t Ftp::size()
{
  return m_size;
}


bool Ftp::atEOF()
{
  return( m_bytesLeft <= 0 );
}


size_t Ftp::read(void *buffer, long len)
{
  size_t n = ::read( sData, buffer, len );
  m_bytesLeft -= n;
  return n;
}


size_t Ftp::write(void *buffer, long len)
{
  return( ::write( sData,buffer,len ) );
}


bool Ftp::mkdir( KURL& _url )
{
  QString redirect;

  if( !ftpConnect( _url.host(), _url.port(), _url.user(), _url.pass(), redirect ) )
    return false;

  bool res = ftpMkdir( _url.path() );

  ftpDisconnect();

  if ( !res ) {
    if ( !m_error ) {
      m_error = ERR_COULD_NOT_MKDIR;
      m_errorText = _url.url();
    }
    return false;
  }

  return true;
}


// Patch from Alessandro Mirone <alex@greco2.polytechnique.fr>
// Little helper function
const char* strnextchr( const char * p , char c )
{
  while( *p != c && *p != 0L ) {
    p++;
  }
  return p;
}
// end patch
