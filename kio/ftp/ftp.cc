/*  This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// $Id$

#include "ftp.h"
#include <kio/passdlg.h>

#include <errno.h>
#include <assert.h>
#include <string.h>

#include <kprotocolmanager.h>

#include <kdebug.h>
#include <ksock.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <netdb.h>
#include <sys/time.h>
#include <ctype.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#define FTP_LOGIN "anonymous"
#define FTP_PASSWD "kde-user@kde.org"

//const char* strnextchr( const char * p , char c );

using namespace KIO;

Ftp::Ftp( Connection *connection )
    : SlaveBase( connection )
{
  dirfile = 0L;
  sControl = sData = sDatal = 0;
  m_bLoggedOn = false;
  m_bFtpStarted = false;
  m_bPersistent = true;
  kDebugInfo( 7102, "Ftp::Ftp()");
}


Ftp::~Ftp()
{
  disconnect( true );
}

#ifndef HAVE_MEMCCPY
extern "C" {
void *memccpy(void *dest, const void *src, int c, size_t n)
{
    char *d = (char*)dest;
    const char *s = (const char*)src;

    while (n-- > 0)
    if ((*d++ = *s++) == c)
      return d;

  return NULL;
}
}
#endif

/*
 * read a line of text
 *
 * return -1 on error, bytecount otherwise
 */
int Ftp::ftpReadline(char *buf,int max,netbuf *ctl)
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
      kDebugError( 7102, "read failed" );
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

/**
 * read a response from the server
 *
 * @param expresp expected response ('\0' for disabling builtin check)
 * @return false if first char doesn't match @p expresp
 *  true if first char matches @p expresp
 */
bool Ftp::readresp(char expresp)
{
  char match[5];
  if ( ftpReadline( rspbuf, 256, nControl ) == -1 )
  {
    kDebugError( 7102, "Could not read" );
    error( ERR_COULD_NOT_READ, "" );
    return false;
  }
  kDebugInfo( 7102, "resp> %s",rspbuf);
  if ( rspbuf[3] == '-' )  {
    strncpy( match, rspbuf, 3 );
    match[3] = ' ';
    match[4] = '\0';
    do {
      if ( ftpReadline( rspbuf, 256, nControl ) == -1 ) {
        error( ERR_COULD_NOT_READ, "" );
	return false;
      }
      kDebugInfo( 7102, "%s",rspbuf);
    }
    while ( strncmp( rspbuf, match, 4 ) );
  }
    	
  if ( rspbuf[0] == expresp || !expresp )
    return true;

  return false;
}

void Ftp::closeConnection()
{
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
  ready();
}

/*
 * disconnect from remote
 */
void Ftp::disconnect( bool really )
{
  if ( !m_bPersistent || really )
      closeConnection();
}


void Ftp::openConnection( const QString& _host, int _port, const QString& _user, const QString& _pass )
{
  m_bPersistent = KProtocolManager::self().persistentConnections();

  if ( m_bLoggedOn )
    if ( m_bPersistent ) {
      if ( m_host != _host || m_user != _user ) {
        kDebugInfo( 7102, "%s %s -> %s %s  ---> Disconnecting. The scheduler should have taken care of it !", m_host.ascii(), m_user.ascii(), _host.ascii(), _user.ascii());
        disconnect( true );
      } else
      // this should check whether there is still opened data connection.
      // is it enough ?  Should we check also the control connection ?
      if ( ftpOpenDataConnection() )
          return;
      else
          m_bLoggedOn = false;
    }

  assert( !m_bLoggedOn );

  m_redirect = "";

  if (!connect( _host, _port ))
    return; // error emitted by connect

  m_bFtpStarted = true;

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

  kDebugInfo( 7102, "Connected ...." );

  m_bLoggedOn = ftpLogin( user, passwd );
  if ( !m_bLoggedOn ) {
    kDebugError( 7102, "Could not login" );
    error( ERR_COULD_NOT_LOGIN, _host );
    return;
  }

  // We could login and got a redirect ?
  if ( !m_redirect.isEmpty() && m_redirect != "/") {
    /*
    if ( m_redirect.right(1) != "/" ) {
      m_redirect += "/";
    }
    */
    kDebugInfo( 7102, "REDIRECTION '%s'", m_redirect.ascii());
    // TODO emit redirection ?
  }

  m_bLoggedOn = true;

  ready();
}


/**
 * Called by @ref openConnection. It opens the control connection to the ftp server.
 *
 * @return true on success.
 */
bool Ftp::connect( const QString &host, unsigned short int port )
{
  ksockaddr_in sin;
  struct servent *pse;
  int on = 1;

  m_host = "";
  m_user = "";

  memset( &sin, 0, sizeof( sin ) );

  if ( port == 0 && ( pse = getservbyname( "ftp", "tcp" ) ) == NULL )
    port = 21;
  else if ( port == 0 )
    port = ntohs(pse->s_port);

  if (!KSocket::initSockaddr(&sin, host, port)) {
    error( ERR_UNKNOWN_HOST, host );
    return false;
  }

  sControl = socket( get_sin_family(sin), SOCK_STREAM, IPPROTO_TCP );
  if ( sControl == 0 ) {
    error( ERR_COULD_NOT_CREATE_SOCKET, host );
    return false;
  }
  if ( setsockopt( sControl, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on) ) == -1 )
  {
    ::close( sControl );
    error( ERR_COULD_NOT_CREATE_SOCKET, host );
    return false;
  }

  if ( ::connect(sControl, (struct sockaddr *)&sin, sizeof(sin)) == -1)
  {
    ::close( sControl );
    error( ERR_COULD_NOT_CONNECT, host );
    return false;
  }

  nControl = (netbuf*)calloc(1,sizeof(netbuf));
  if (nControl == NULL)
  {
    ::close( sControl );
    error( ERR_OUT_OF_MEMORY, "" );
    return false;
  }
  nControl->handle = sControl;

  if ( ! readresp( '2' ) )
  {
    ::close( sControl );
    free( nControl );
    return false; // error emitted by readresp
  }

  m_host = host;

  return true;
}

/**
 * Called by @ref openConnection. It logs us in.
 * @ref m_redirect is set to the current working directory
 * if logging on was successfull.
 *
 * @return true on success.
 */
bool Ftp::ftpLogin( const QString & user, const QString & _pass )
{
  assert( !m_bLoggedOn );

  m_user = user;
  QString pass = _pass;

  if ( !m_user.isEmpty() ) {
    QCString tempbuf = "user ";
    tempbuf += m_user;

    rspbuf[0] = '\0';

    if ( !ftpSendCmd( tempbuf, '3' ) ) {
      kDebugInfo( 7102, "1> %s", rspbuf );

      if ( rspbuf[0] == '2' )
        return true; /* no password required */
      return false;
    }

    if ( pass.isEmpty() ) {
      QString tmp = m_user + "@" + m_host;
      if ( !open_PassDlg( tmp, m_user, pass ) )
	return false;
    }
    kDebugInfo( 7102, "New pass is '%s'", pass.ascii());

    tempbuf = "pass ";
    tempbuf += pass;

    if ( !ftpSendCmd( tempbuf, '2' ) ) {
      kDebugInfo( 7102, "Wrong password");
      return false;
    }
  }

  kDebugInfo( 7102, "Login ok");

  // Okay, we're logged in. If this is IIS 4, switch dir listing style to Unix:
  // Thanks to jk@soegaard.net (Jens Kristian Søgaard) for this hint
  if( ftpSendCmd( "syst", '2' ) )
  {
    if( !strncmp( rspbuf, "215 Windows_NT version", 22 ) ) // should do for any version
    {
      ftpSendCmd( "site dirstyle", '2' );
      // Check if it was already in Unix style
      // Patch from Keith Refson <Keith.Refson@earth.ox.ac.uk>
      if( !strncmp( rspbuf, "200 MSDOS-like directory output is on", 37 ))
         //It was in Unix style already!
         ftpSendCmd( "site dirstyle", '2' );

    }
  }
  else
    kDebugWarning( 7102, "syst failed");

  // No way to know this...
  /*
  // Not interested in the current working directory ? => return with success
  if ( _redirect.isEmpty() )
    return true;
  */

  kDebugInfo( 7102, "Searching for pwd");

  // Get the current working directory
  if ( !ftpSendCmd( "pwd", '2' ) )
    return false;

  kDebugInfo( 7102, "2> %s", rspbuf );

  char *p = rspbuf;
  while ( isdigit( *p ) ) p++; // skip return code
  while ( *p == ' ' || *p == '\t' ) p++; // and leading spaces
  if ( *p != '"' ) // Look for first "
    return true;
  char *p2 = (char*)strchr( p + 1, '"' ); // Look for second "
  if ( p2 == 0L )
    return true;
  *p2 = 0;
  m_redirect = p + 1; // Extract path
  return true;
}


/**
 * ftpSendCmd - send a command (@p cmd) and wait for expected response
 *
 * @param expresp the expected first char. '\0' for no check
 *
 * return true if proper response received, false on error
 * or if @p expresp doesn't match
 */
bool Ftp::ftpSendCmd( const QCString& cmd, char expresp )
{
  assert( sControl > 0 );

  QCString buf = cmd;
  buf += "\r\n";

  kDebugInfo( 7102, "%s", cmd.data() );

  if ( ::write( sControl, buf.data(), buf.length() ) <= 0 )  {
    error( ERR_COULD_NOT_WRITE, "" );
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
    kDebugError(7102, "Keepalive not allowed");
  if ( setsockopt(sDatal, SOL_SOCKET,SO_LINGER, (char *) &lng,(int) sizeof (lng)) < 0 )
    kDebugError(7102, "Linger mode was not allowed.");
  return true;
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
    error( ERR_COULD_NOT_CREATE_SOCKET, "" );
    return false;
  }
  if ( setsockopt( sDatal, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on) ) == -1 )
  {
    ::close( sDatal );
    error( ERR_COULD_NOT_CREATE_SOCKET, "" );
    return false;
  }
  if ( setsockopt( sDatal, SOL_SOCKET, SO_LINGER, (char*)&lng, sizeof(lng) ) == -1 )
  {
    ::close( sDatal );
    error( ERR_COULD_NOT_CREATE_SOCKET, "" );
    return false;
  }

  sin.in.sin_port = 0;
  if ( bind( sDatal, &sin.sa, sizeof(sin) ) == -1 )
  {
    ::close( sDatal );
    sDatal = 0;
    error( ERR_COULD_NOT_BIND, m_host );
    return false;
  }

  if ( listen( sDatal, 1 ) < 0 )
  {
    error( ERR_COULD_NOT_LISTEN, m_host );
    ::close( sDatal );
    sDatal = 0;
    return 0;
  }

  if ( getsockname( sDatal, &sin.sa, &l ) < 0 )
    // error ?
    return false;

  sprintf(buf,"port %d,%d,%d,%d,%d,%d",
	  (unsigned char)sin.sa.sa_data[2],(unsigned char)sin.sa.sa_data[3],
	  (unsigned char)sin.sa.sa_data[4],(unsigned char)sin.sa.sa_data[5],
	  (unsigned char)sin.sa.sa_data[0],(unsigned char)sin.sa.sa_data[1]);

  return ftpSendCmd( buf, '2' );
}


/*
 * ftpAcceptConnect - wait for incoming connection
 * Used by @ref openCommand
 *
 * return -2 on error or timeout
 * otherwise returns socket descriptor
 */
int Ftp::ftpAcceptConnect()
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

bool Ftp::openCommand( const char *_command, const QString & _path, char _mode,
                       int errorcode, unsigned long _offset )
{
  QCString buf = "type ";
  buf += _mode;

  if ( !ftpSendCmd( buf, '2' ) )
  {
    error( ERR_COULD_NOT_CONNECT, "" );
    return false;
  }
  if ( !ftpOpenDataConnection() )
  {
    error( ERR_COULD_NOT_CONNECT, "" );
    return false;
  }

  if ( _offset > 0 ) {
    // send rest command if offset > 0, this applies to retr and stor commands
    char buf[100];
    sprintf(buf, "rest %ld", _offset);
    if ( !ftpSendCmd( buf, '3' ) ) {
      if ( rspbuf[0] != '3' ) // other errors were already emitted
	{
	  error( ERR_CANNOT_RESUME, _path );
	  return false;
	}
    }
  }

  QCString tmp = _command;

  if ( _path != 0L ) {
    tmp += " ";
    tmp += _path;
  }

  if ( !ftpSendCmd( tmp, '1' ) ) {
    // The error here depends on the command
    error( errorcode, _path );
    return false;
  }

  if ( ( sData = ftpAcceptConnect() ) < 0 )
  {
    error( ERR_COULD_NOT_ACCEPT, "" );
    return false;
  }

  return true;
}


bool Ftp::ftpCloseCommand()
{
  // first close data sockets (if opened), then read response that
  // we got for whatever was used in ftpOpenCommand ( should be 226 )
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
  if ( !readresp( '2' ) )
  {
    kDebugInfo( 7102, "Did not get transfer complete message");
    return false;
  }
  return true;
}

void Ftp::mkdir( const QString & path, int permissions )
{
  assert( m_bLoggedOn );

  QCString buf = "mkd ";
  buf += path.latin1();

  if ( ! ftpSendCmd( buf, '2' ) )
  {
    error( ERR_COULD_NOT_MKDIR, path );
    return;
  }

  if ( permissions != -1 )
    chmod( path, permissions );
  else
    finished();
}

void Ftp::rename( const QString & src, const QString & dst, bool overwrite )
{
  // The actual functionality is in ftpRename because put needs it
  if ( ftpRename( src, dst, overwrite ) )
    finished();
  else
    error( ERR_CANNOT_RENAME, src );
}

bool Ftp::ftpRename( const QString & src, const QString & dst, bool /* overwrite */ )
{
  // TODO honor overwrite
  assert( m_bLoggedOn );

  QCString cmd;
  cmd = "RNFR ";
  cmd += src;
  if ( !ftpSendCmd( cmd, '3') )
    return false;
  cmd = "RNTO ";
  cmd += dst;
  return ftpSendCmd( cmd, '2' );
}

void Ftp::del( const QString& path, bool isfile )
{
  assert( m_bLoggedOn );

  QCString cmd = isfile ? "DELE " : "RMD ";
  cmd += path;

  if ( !ftpSendCmd( cmd, '2' ) )
    error( ERR_CANNOT_DELETE, path );
  else
    finished();
}

void Ftp::chmod( const QString & path, int permissions )
{
  assert( m_bLoggedOn );

  QCString cmd = "SITE CHMOD ";

  char buf[10];
  // we need to do bit AND 777 to get permissions, in case
  // we were sent a full mode (unlikely)
  sprintf(buf, "%o ", permissions & 511 );

  cmd += buf;
  cmd += path;

  if ( !ftpSendCmd( cmd, '2' ) )
    error( ERR_CANNOT_CHMOD, path );
  else
    finished();
}

void Ftp::createUDSEntry( FtpEntry * e, UDSEntry & entry )
{
  assert(entry.count() == 0); // by contract :-)
  UDSAtom atom;
  atom.m_uds = UDS_NAME;
  atom.m_str = e->name;
  entry.append( atom );

  atom.m_uds = UDS_FILE_TYPE;
  atom.m_long = e->type;
  entry.append( atom );

  atom.m_uds = UDS_SIZE;
  atom.m_long = e->size;
  entry.append( atom );

  atom.m_uds = UDS_MODIFICATION_TIME;
  atom.m_long = e->date;
  entry.append( atom );

  atom.m_uds = UDS_ACCESS;
  atom.m_long = e->access;
  entry.append( atom );

  atom.m_uds = UDS_USER;
  atom.m_str = e->owner;
  entry.append( atom );

  atom.m_uds = UDS_GROUP;
  atom.m_str = e->group;
  entry.append( atom );

  if ( !e->link.isEmpty() )
  {
    atom.m_uds = UDS_LINK_DEST;
    atom.m_str = e->link;
    entry.append( atom );
  }

  /* atom.m_uds = UDS_ACCESS_TIME;
     atom.m_long = buff.st_atime;
     entry.append( atom );

     atom.m_uds = UDS_CREATION_TIME;
     atom.m_long = buff.st_ctime;
     entry.append( atom ); */
}

void Ftp::stat( const QString & path )
{
  kDebugInfo( 7102, "stat : %s", path.ascii() );

  // We can't stat root, but we know it's a dir.
  if ( path.isEmpty() || path == "/" ) {
    UDSEntry entry;
    UDSAtom atom;

    atom.m_uds = KIO::UDS_NAME;
    atom.m_str = "";
    entry.append( atom );

    atom.m_uds = KIO::UDS_FILE_TYPE;
    atom.m_long = S_IFDIR;
    entry.append( atom );

    atom.m_uds = KIO::UDS_ACCESS;
    atom.m_long = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    entry.append( atom );

    atom.m_uds = KIO::UDS_USER;
    atom.m_str = "root";
    entry.append( atom );
    atom.m_uds = KIO::UDS_GROUP;
    entry.append( atom );

    // no size

    statEntry( entry );
    finished();
    return;
  }

  // Argument to the list command (defaults to the directory containing the file)
  // Let's use KURL's function (even if building it as a local one)
  KURL tempurl( path );
  QString listarg = tempurl.directory();
  QString search = tempurl.filename();

  // Try cwd into it, if it works it's a dir (and then we'll use dir in the parent directory)
  // if it doesn't work, it's a file (and then we'll use dir filename)
  QCString tmp = "cwd ";
  tmp += path.latin1();
  if ( !ftpSendCmd( tmp, '\0' /* no builtin response check */ ) )
    // error already emitted, if e.g. transmission failure
    return;

  if ( rspbuf[0] == '5' )
  {
    // It is a file or it doesn't exist, use the name in the list command
    listarg = path;
    search = path;
  }

  if( !openCommand( "list", listarg, 'A', ERR_DOES_NOT_EXIST ) )
  {
    kDebugError( 7102, "COULD NOT LIST");
    return;
  }

  dirfile = fdopen( sData, "r" );
  if( !dirfile ) {
    error( ERR_DOES_NOT_EXIST, path );
    return;
  }

  kDebugInfo( 7102, "Starting of list was ok");

  assert( search != "" && search != "/" );

  FtpEntry *e;
  bool bFound = false;
  while( ( e = ftpReadDir() ) )
  {
    if ( !bFound && ( search == e->name ) ) {
      bFound = true;
      UDSEntry entry;
      createUDSEntry( e, entry );
      statEntry( entry );
    }

    kDebugInfo( 7102, "%s", e->name.ascii());
  }

  if ( !ftpCloseDir() )
    return; // error ?

  if ( !bFound )
  {
    error( ERR_DOES_NOT_EXIST, path );
    return;
  }

  kDebugInfo( 7102, "stat : finished successfully" );
  finished();
}


void Ftp::listDir( const QString & _path )
{
  QString path = _path;
  // Did we get a redirect and did not we specify a path ourselves ?
  if ( !m_redirect.isEmpty() && path.isEmpty() )
  {
    redirection( m_redirect );
    path = m_redirect;
  }

  kDebugInfo( 7102, "hunting for path '%s'", path.ascii());

  if (!ftpOpenDir( path ) )
  {
    if ( ftpSize( path, 'I' ) ) // is it a file ?
    {
      error( ERR_IS_FILE, path );
      return;
    }
    // not sure which to emit
    //error( ERR_DOES_NOT_EXIST, path );
    error( ERR_CANNOT_ENTER_DIRECTORY, path );
    return;
  }

  UDSEntry entry;
  FtpEntry * e;
  while( ( e = ftpReadDir() ) )
  {
    kDebugInfo( 7102, "%s", e->name.ascii());
    if ( S_ISDIR( (mode_t)e->type ) )
    {
        kDebugInfo( 7102, "is a dir" );
    }
    entry.clear();
    createUDSEntry( e, entry );
    listEntry( entry, false );
  }
  listEntry( entry, true ); // ready

  (void) ftpCloseDir();

  finished();
}


bool Ftp::ftpOpenDir( const QString & path )
{
  //QString path( _url.path(-1) );

  // We try to change to this directory first to see whether it really is a directory.
  // (And also to follow symlinks)
  QCString tmp = "cwd ";
  tmp += ( !path.isEmpty() ) ? path.latin1() : "/";

  if ( !ftpSendCmd( tmp, '2' ) )
    // We get '550', whether it's a file or doesn't exist...
    return false;

  // don't use the path in the list command
  // we changed into this directory anyway ("cwd"), so it's enough just to send "list"
  if( !openCommand( "list", 0L, 'A', ERR_CANNOT_ENTER_DIRECTORY ) )
    return false;

  dirfile = fdopen( sData, "r" );
  if( !dirfile )
    return false;

  kDebugInfo( 7102, "Starting of list was ok");

  return true;
}

FtpEntry *Ftp::ftpReadDir()
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
		    switch ( p_access[0] ) {
                        case 'd':
                            de.type = S_IFDIR;
                            break;
                        case 's':
                            de.type = S_IFSOCK;
                            break;
                        case 'b':
                            de.type = S_IFBLK;
                            break;
                        case 'c':
                            de.type = S_IFCHR;
                            break;
                        case 'l':
                            de.type = S_IFDIR;
        // links on ftp sites are often links to dirs, and we have no way to check that
        // let's do like Netscape : assume dirs (David)
        // we don't set S_IFLNK here.  de.link says it.
                            break;
                        default:
                            break;
                    }
		
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

                    // First get current time - we need the current month and year
                    time_t currentTime = time( 0L );
                    struct tm * tmptr = gmtime( &currentTime );
                    int currentMonth = tmptr->tm_mon;
                    //kDebugInfo( 7102, "Current time :%s", asctime( tmptr ) );
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
                      // otherwise, the year is implicit
                      // according to man ls, this happens when it is between than 6 months
                      // old and 1 hour in the future.
                      // So the year is : current year if tm_mon <= currentMonth+1
                      // otherwise current year minus one
                      // (The +1 is a security for the "+1 hour" at the end of the month issue)
                      if ( tmptr->tm_mon > currentMonth + 1 )
                        tmptr->tm_year--;

                      // and p_date_3 contains probably a time
                      char * semicolon;
                      if ( ( semicolon = (char*)strchr( p_date_3, ':' ) ) )
                      {
                        *semicolon = '\0';
                        tmptr->tm_min = atoi( semicolon + 1 );
                        tmptr->tm_hour = atoi( p_date_3 );
                      }
                      else
                        kDebugWarning( 7102, "Can't parse third field %s", p_date_3 );
                    }

                    //kDebugInfo( 7102, asctime( tmptr ) );
                    de.date = mktime( tmptr );
		    return( &de );
		  }
	  }
  return 0L;
}


bool Ftp::ftpCloseDir()
{
  if( dirfile )
  {
    kDebugInfo( 7102, "... closing");

    if ( ! ftpCloseCommand() )
      return false;

    fclose( dirfile );
    dirfile = 0L;

    //disconnect();
  } else
    kDebugInfo( 7102, "ftpCloseDir but no dirfile ??" );
  return true;
}

//////////// get, put ////////

void Ftp::get( const QString & path, const QString & /*query*/, bool /*reload*/ )
{
  // Old code used to start by stat'ing, just to make sure it exists
  // Waste of time, I'd say. (David)

  ftpSize( path, 'I' ); // try to find the size of the file

  unsigned long offset = 0; // looks like this was never set to something else...
  // Don't we want support for getting a file from a certain offset ? Hmm...

  if ( !openCommand( "retr", path, 'I', ERR_CANNOT_OPEN_FOR_READING, offset ) ) {
    kDebugWarning( 7102, "Can't open for reading");
    return;
  }

  // Read the size from the response string
  if ( strlen( rspbuf ) > 4 && m_size == 0 ) {
    // Patch from Alessandro Mirone <alex@greco2.polytechnique.fr>
    /*
    const char *p = rspbuf;
    const char *oldp = 0L;
    while ( *( p = strnextchr( p , '(' ) ) == '(' )
    {
      oldp = p;
      p++;
    }
    p = oldp;
    */
    // end patch
    // Same thing in one call, by David
    const char * p = strrchr( rspbuf, '(' );
    if ( p != 0L ) m_size = atol( p + 1 );
  }

  m_bytesLeft = m_size - offset;

  ready();
  // TODO gettingFile( usrc );
  totalSize( m_size );
  int processed_size = 0;
  time_t t_start = time( 0L );
  time_t t_last = t_start;

  char buffer[ 2048 ];
  QByteArray array;

  while( m_bytesLeft > 0 )
  {
    int n = ftpRead( buffer, 2048 );
    array.setRawData(buffer, n);
    data( array );
    array.resetRawData(buffer, n);

    processed_size += n;
    time_t t = time( 0L );
    if ( t - t_last >= 1 ) {
      processedSize( processed_size );
      speed( processed_size / ( t - t_start ) );
      t_last = t;
    }
  }

  data( QByteArray() );

  (void) ftpCloseCommand();
  // proceed even on error

  processedSize( m_size );
  time_t t = time( 0L );
  if ( t - t_start >= 1 )
    speed( processed_size / ( t - t_start ) );

  finished();
}

void Ftp::put( const QString& dest_orig, int permissions, bool overwrite, bool resume )
{
  kDebugInfo( 7102, "Put %s", debugString(dest_orig) );
  QString dest_part( dest_orig );
  dest_part += ".part";

  bool bMarkPartial = KProtocolManager::self().markPartial();

  // Don't use mark partial over anonymous FTP.
  // My incoming dir allows put but not rename...
  if (m_user == FTP_LOGIN)
      bMarkPartial = false;

  if ( ftpSize( dest_orig, 'I' ) )
  {
    if ( m_size == 0 ) {  // delete files with zero size
      QCString cmd = "DELE ";
      cmd += dest_orig;
      if ( !ftpSendCmd( cmd, '2' ) )
      {
        error( ERR_CANNOT_DELETE_PARTIAL, dest_orig );
        return;
      }
    } else if ( !overwrite && !resume ) {
      /*
        Waldo said we don't always know the size of what we're writing...
      if ( m_size == _size ) {
	error( ERR_DOES_ALREADY_EXIST_FULL, dest_orig );
      } else */
      error( ERR_FILE_ALREADY_EXIST, dest_orig );
      return;
    } else if ( bMarkPartial ) { // when using mark partial, append .part extension
      if ( !ftpRename( dest_orig, dest_part, true ) )
      {
        error( KIO::ERR_CANNOT_RENAME_PARTIAL, dest_orig );
        return;
      }
    }
  } else if ( ftpSize( dest_part, 'I' ) ) { // file with extension .part exists
    if ( m_size == 0 ) {  // delete files with zero size
      QCString cmd = "DELE ";
      cmd += dest_part;
      if ( !ftpSendCmd( cmd, '2' ) )
      {
        error( ERR_CANNOT_DELETE_PARTIAL, dest_orig );
        return;
      }
    } else if ( !overwrite && !resume ) {
      /*
        Waldo said we don't always know the size of what we're writing...
      if ( m_size == _size ) {
	error( ERR_DOES_ALREADY_EXIST_FULL, dest_orig );
      } else
      */
      error( ERR_FILE_ALREADY_EXIST, dest_orig );
      return;
    } else if ( !bMarkPartial ) { // when using mark partial, remove .part extension
      if ( !ftpRename( dest_part, dest_orig, true ) )
      {
        error( KIO::ERR_CANNOT_RENAME_PARTIAL, dest_orig );
        return;
      }
    }

  }

  QString dest;

  // if we are using marking of partial downloads -> add .part extension
  if ( bMarkPartial ) {
    kDebugInfo( 7102, "Adding .part extension to %s", dest_orig.ascii() );
    dest = dest_part;
  } else
    dest = dest_orig;

  unsigned long offset = 0;

  // set the mode according to offset
  if ( resume ) {
    offset = m_size;
    kDebugInfo( 7102, "Offset = %ld", offset );
  }

  if (! openCommand( "stor", dest, 'I', ERR_COULD_NOT_WRITE, offset ) )
    return;

  // We are ready for receiving data
  ready();

  kDebugInfo( 7102, "Put: Ready" );

  int result;
  // Loop until we got 'dataEnd'
  do
  {
    QByteArray buffer;
    dataReq(); // Request for data
    result = readData( buffer );
    if (result > 0)
    {
      ftpWrite( buffer.data(), buffer.size() );
    }
  }
  while ( result > 0 );

  if (result != 0) // error
  {
    (void) ftpCloseCommand(); // don't care about errors
    kDebugInfo( 7102, "Error during 'put'. Aborting.");
    if (bMarkPartial)
    {
      // Remove if smaller than minimum size
      if ( ftpSize( dest, 'I' ) &&
           ( m_size < (unsigned long) KProtocolManager::self().minimumKeepSize() ) )
      {
        QCString cmd = "DELE ";
        cmd += dest;
        (void) ftpSendCmd( cmd, '\0' );
      }
    }
    return;
  }

  if ( !ftpCloseCommand() )
  {
    error( KIO::ERR_COULD_NOT_WRITE, dest_orig);
    return;
  }

  // after full download rename the file back to original name
  if ( bMarkPartial )
  {
    kDebugInfo( 7102, "renaming dest (%s) back to dest_orig (%s)", dest.ascii(), dest_orig.ascii() );
    if ( !ftpRename( dest, dest_orig, true ) )
    {
      error( KIO::ERR_CANNOT_RENAME_PARTIAL, dest_orig );
      return;
    }
  }

  // set final permissions
  if ( permissions != -1 )
  {
    if ( m_user == FTP_LOGIN )
      kDebugInfo( 7102, "Trying to chmod over anonymous FTP ???" );
    chmod(dest_orig, permissions); // will emit error or finished
    return;
  }

  // We have done our job => finish
  finished();
}


/*
  This is related to "canResume" ... not sure how
  Old main.cc contained:
  if ( !ftp.ftpResume( 0 ) )
    m_bCanResume = false;

bool Ftp::ftpResume( unsigned long offset )
{
  char buf[64];
  sprintf(buf, "rest %ld", offset);
  if ( !ftpSendCmd( buf, '3' ) ) {
    error( ERR_CANNOT_RESUME, "" );
    return false;
  }
  return true;
}
*/

/** Use the SIZE command to get the file size.
    Warning : the size depends on the transfer mode, hence the second arg. */
bool Ftp::ftpSize( const QString & path, char mode )
{
  QCString buf;
  buf.sprintf("type %c", mode);
  if ( !ftpSendCmd( buf, '2' ) ) {
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
  if ( !ftpCloseCommand() )
    return false;

  //disconnect();

  return true;
}

size_t Ftp::ftpRead(void *buffer, long len)
{
  size_t n = ::read( sData, buffer, len );
  m_bytesLeft -= n;
  return n;
}

size_t Ftp::ftpWrite(void *buffer, long len)
{
  return( ::write( sData, buffer, len ) );
}

// Patch from Alessandro Mirone <alex@greco2.polytechnique.fr>
// Little helper function
/*
const char* strnextchr( const char * p , char c )
{
  while( *p != c && *p != 0L ) {
    p++;
  }
  return p;
}
*/
// end patch

extern "C" {
    SlaveBase *init_ftp() {
        return new Ftp();
    }
}
