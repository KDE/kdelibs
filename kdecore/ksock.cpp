/* This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)

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

/*
 * $Log$
 * Revision 1.32  1999/06/21 15:31:15  welk
 * continued the int -> unsigned short int change for port address what Alex
 * started (Yeah, konqueror shows html-pages again :-)
 *
 * Revision 1.31  1999/05/07 16:45:21  kulow
 * adding more explicit calls to ascii()
 *
 * Revision 1.30  1999/04/19 15:49:47  kulow
 * cleaning up yet some more header files (fixheaders is your friend).
 * Adding copy constructor to KPixmap to avoid casting while assingment.
 *
 * The rest of the fixes in kdelibs and kdebase I will commit silently
 *
 * Revision 1.29  1999/04/18 19:55:43  kulow
 * CVS_SILENT some more fixes
 *
 * Revision 1.28  1999/04/11 00:00:46  garbanzo
 * Introduce the new static member, initsockaddr
 *
 */

#include "ksock.h"

#include <qapplication.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYSENT_H
#include <sysent.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#ifdef HAVE_SYS_SOCKET_H
// on Linux/libc5, this includes linux/socket.h where SOMAXCONN is defined
#include <sys/socket.h>
#endif
// Play it safe, use a reasonable default, if SOMAXCONN was nowhere defined.
#ifndef SOMAXCONN
#warning Your header files do not seem to support SOMAXCONN
#define SOMAXCONN 5
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

#include <sys/resource.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <qsocketnotifier.h>

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108 // this is the value, I found under Linux
#endif

KSocket::KSocket( const char *_host, unsigned short int _port, int _timeout ) :
  sock( -1 ), readNotifier( 0L ), writeNotifier( 0L )
{
    timeOut = _timeout;
    domain = PF_INET;
    connect( _host, _port );
}

KSocket::KSocket( const char *_host, unsigned short int _port ) :
  sock( -1 ), readNotifier( 0L ), writeNotifier( 0L )
{
    timeOut = 30;
    domain = PF_INET;
    connect( _host, _port );
}

KSocket::KSocket( const char *_path ) :
  sock( -1 ), readNotifier( 0L ), writeNotifier( 0L )
{
  domain = PF_UNIX;
  connect( _path );
}

void KSocket::enableRead( bool _state )
{
  if ( _state )
    {
	  if ( !readNotifier  )
		{
		  readNotifier = new QSocketNotifier( sock, QSocketNotifier::Read );
		  QObject::connect( readNotifier, SIGNAL( activated(int) ), this, SLOT( slotRead(int) ) );
		}
	  else
	    readNotifier->setEnabled( true );
    }
  else if ( readNotifier )
	readNotifier->setEnabled( false );
}

void KSocket::enableWrite( bool _state )
{
  if ( _state )
    {
	  if ( !writeNotifier )
		{
		  writeNotifier = new QSocketNotifier( sock, QSocketNotifier::Write );
		  QObject::connect( writeNotifier, SIGNAL( activated(int) ), this, 
							SLOT( slotWrite(int) ) );
		}
	  else
	    writeNotifier->setEnabled( true );
    }
  else if ( writeNotifier )
	writeNotifier->setEnabled( false );
}

void KSocket::slotRead( int )
{
  char buffer[2];
  
  int n = recv( sock, buffer, 1, MSG_PEEK );
  if ( n <= 0 )
	emit closeEvent( this );
  else
	emit readEvent( this );
}

void KSocket::slotWrite( int )
{
  emit writeEvent( this );
}

/*
 * Initializes a sockaddr structure. Do this after creating a socket and
 * before connecting to any other socket. Here you must specify the
 * host and port you want to connect to.
 */
bool KSocket::init_sockaddr( const QString& hostname, unsigned short int port )
{
  if ( domain != PF_INET )
    return false;
  
  struct hostent *hostinfo;
  server_name.sin_family = AF_INET;
  server_name.sin_port = htons( port );
  hostinfo = gethostbyname( hostname.ascii() );
  
  if ( !hostinfo )
    {
	  warning("Unknown host %s.\n", hostname.ascii());
	  return false;	
    }
  server_name.sin_addr = *(struct in_addr*) hostinfo->h_addr;    
  
  return true;
}

/*
 * Connects the PF_UNIX domain socket to _path.
 */
bool KSocket::connect( const char *_path )
{
  if ( domain != PF_UNIX )
    fatal( "Connecting a PF_INET socket to a PF_UNIX domain socket\n");
  
  sock = ::socket(PF_UNIX,SOCK_STREAM,0);
  if (sock < 0)
	return false;
  
  unix_addr.sun_family = AF_UNIX;
  int l = strlen( _path );
  if ( l > UNIX_PATH_MAX - 1 )
  {      
    warning( "Too long PF_UNIX domain name '%s'\n",_path);
    return false;
  }  
  strcpy( unix_addr.sun_path, _path );

  if ( 0 > ::connect( sock, (struct sockaddr*)(&unix_addr), 
					  sizeof( unix_addr ) ) )
  {
      ::close( sock );
      sock = -1;
      return false;
  }

  return true;
}

/*
 * Connects the socket to _host, _port.
 */
bool KSocket::connect( const QString& _host, unsigned short int _port )
{
  if ( domain != PF_INET )
    fatal( "Connecting a PF_UNIX domain socket to a PF_INET domain socket\n");

  sock = ::socket(PF_INET,SOCK_STREAM,0);
  if (sock < 0)
	return false;
  
  if ( !init_sockaddr( _host, _port) )
	{
	  ::close( sock );
	  sock = -1;
	  return false;
	}

  fcntl(sock,F_SETFL,(fcntl(sock,F_GETFL)|O_NDELAY));

  errno = 0;
  if (::connect(sock, (struct sockaddr*)(&server_name), sizeof(server_name))){
      if(errno != EINPROGRESS && errno != EWOULDBLOCK){
          ::close( sock );
          sock = -1;
          return false;
      }
  }else
      return true;

  fd_set rd, wr;
  struct timeval timeout;
  int ret = 0, n;

  n = timeOut;
  FD_ZERO(&rd);
  FD_ZERO(&wr);
  FD_SET(sock, &rd);
  FD_SET(sock, &wr);
//  printf("timeout=%d\n", n);
  while(n--){
      timeout.tv_usec = 0;
      timeout.tv_sec = 1;

      struct rlimit rlp;
      getrlimit(RLIMIT_NOFILE, &rlp); // getdtablesize() equivalent. David Faure.

      ret = select(rlp.rlim_cur, (fd_set *)&rd, (fd_set *)&wr, (fd_set *)0,
                   (struct timeval *)&timeout);
      // if(ret)
      //    return(true);

      switch (ret)
      {
	  case 0: break; // Timeout
	  case 1: case 2: return(true); // Success
	  default: // Error
	      ::close(sock);
	      sock = -1;
	      return false;
      }

      qApp->processEvents();
      qApp->flushX();
  }
  warning("Timeout connecting socket...\n");
  ::close( sock );
  sock = -1;
  return false;
}

unsigned long KSocket::getAddr()
{
  if ( domain != PF_INET )
    return 0;
  
  struct sockaddr_in name; ksize_t len = sizeof(name);
  getsockname(sock, (struct sockaddr *) &name, &len);
  return ntohl(name.sin_addr.s_addr);
}

bool KSocket::initSockaddr (struct sockaddr_in *server_name, const char *hostname, unsigned short int port)
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

KSocket::~KSocket()
{
    if ( readNotifier )
    {
	delete readNotifier;
    }
    if ( writeNotifier )
	delete writeNotifier; 
  
    ::close( sock ); 
}


KServerSocket::KServerSocket( const char *_path ) :
  sock( -1 )
{
  domain = PF_UNIX;
  
  if ( !init ( _path ) )
  {
    fatal("Error constructing PF_UNIX domain server socket\n");
    return;
  }
    
  notifier = new QSocketNotifier( sock, QSocketNotifier::Read );
  connect( notifier, SIGNAL( activated(int) ), this, SLOT( slotAccept(int) ) );
}

KServerSocket::KServerSocket( unsigned short int _port ) :
  sock( -1 )
{
  domain = PF_INET;

  if ( !init ( _port ) )
  {
    fatal("Error constructing\n");
    return;
  }
    
  notifier = new QSocketNotifier( sock, QSocketNotifier::Read );
  connect( notifier, SIGNAL( activated(int) ), this, SLOT( slotAccept(int) ) );
}

bool KServerSocket::init( const char *_path )
{
  if ( domain != PF_UNIX )
    return false;
  
  struct sockaddr_un name;
    
  sock = ::socket( PF_UNIX, SOCK_STREAM, 0 );
  if (sock < 0)
  {
    warning( "Could not create socket\n");
    return false;
  }

  unlink(_path );   
  name.sun_family = AF_UNIX;
  int l = strlen( _path );
  if ( l > UNIX_PATH_MAX - 1 )
  {      
    warning( "Too long PF_UNIX domain name '%s'\n",_path);
    return false;
  }  
  strcpy( name.sun_path, _path );

    
  if ( bind( sock, (struct sockaddr*) &name,sizeof( name ) ) < 0 )
  {
    warning("Could not bind to socket\n");
    ::close( sock );
    sock = -1;
    return false;
  }
  
  if ( chmod( _path, 0600) < 0 )
  {
    warning("Could not setupt premissions for server socket\n");
    ::close( sock );
    sock = -1;
    return false;
  }
               
  if ( listen( sock, SOMAXCONN ) < 0 )
  {
    warning("Error listening on socket\n");
    ::close( sock );
    sock = -1;
    return false;
  }

  return true;
}

bool KServerSocket::init( unsigned short int _port )
{
  if ( domain != PF_INET )
    return false;
  
  struct sockaddr_in name;
    
  sock = ::socket( PF_INET, SOCK_STREAM, 0 );
  if (sock < 0)
  {
    warning( "Could not create socket\n");
    return false;
  }
    
  name.sin_family = AF_INET;
  name.sin_port = htons( _port );
  name.sin_addr.s_addr = htonl(INADDR_ANY);
    
  if ( bind( sock, (struct sockaddr*) &name,sizeof( name ) ) < 0 )
    {
	  warning("Could not bind to socket\n");
	  ::close( sock );
	  sock = -1;
	  return false;
    }
    
  if ( listen( sock, SOMAXCONN ) < 0 )
    {
	  warning("Error listening on socket\n");
	  ::close( sock );
	  sock = -1;
	  return false;
    }

  return true;
}

unsigned short int KServerSocket::getPort()
{
  if ( domain != PF_INET )
    return false;

  struct sockaddr_in name; ksize_t len = sizeof(name);
  getsockname(sock, (struct sockaddr *) &name, &len);
  return ntohs(name.sin_port);
}

unsigned long KServerSocket::getAddr()
{
  if ( domain != PF_INET )
    return false;

  struct sockaddr_in name; ksize_t len = sizeof(name);
  getsockname(sock, (struct sockaddr *) &name, &len);
  return ntohl(name.sin_addr.s_addr);
}

void KServerSocket::slotAccept( int )
{
  if ( domain == PF_INET )
  {      
    struct sockaddr_in clientname;
    int new_sock;
    
    ksize_t size = sizeof(clientname);
    
    if ((new_sock = accept (sock, (struct sockaddr *) &clientname, &size)) < 0)
    {
      warning("Error accepting\n");
      return;
    }

    emit accepted( new KSocket( new_sock ) );
  }
  else if ( domain == PF_UNIX )
  {      
    struct sockaddr_un clientname;
    int new_sock;
    
    ksize_t size = sizeof(clientname);
    
    if ((new_sock = accept (sock, (struct sockaddr *) &clientname, &size)) < 0)
    {
      warning("Error accepting\n");
      return;
    }

    emit accepted( new KSocket( new_sock ) );
  }
}

KServerSocket::~KServerSocket()
{
  if ( notifier )
	delete notifier; 
  
  close( sock ); 
}

#include "ksock.moc"
