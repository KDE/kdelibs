/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1997 Torben Weis (weis@kde.org)
 *
 *  $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>
// on Linux/libc5, this includes linux/socket.h where SOMAXCONN is defined
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/un.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#include <netinet/in.h>

#include <arpa/inet.h>

#include "ksock.h"

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
// defines MAXDNAME under Solaris
#include <arpa/nameser.h>
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <cassert>

#ifdef HAVE_SYSENT_H
#include <sysent.h>
#endif

#if TIME_WITH_SYS_TIME
#include <time.h>
#endif

// Play it safe, use a reasonable default, if SOMAXCONN was nowhere defined.
#ifndef SOMAXCONN
#warning Your header files do not seem to support SOMAXCONN
#define SOMAXCONN 5
#endif

#include <qapplication.h>
#include <qsocketnotifier.h>

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108 // this is the value, I found under Linux
#endif

char *KSocket::cachedHostname = 0;
ksockaddr_in *KSocket::cachedServerName = 0;

// I moved this into here so we could accurately detect the domain, for
// posterity.  Really.
KSocket::KSocket( int _sock)
 : sock(_sock), readNotifier(0), writeNotifier(0)
{
  struct sockaddr_in sin;
  ksize_t len = sizeof(sin);

  memset(&sin, 0, len);

  // getsockname will fill in all the appropiate details, and
  // since sockaddr_in will exist everywhere and is somewhat compatible
  // with sockaddr_in6, we can use it to avoid needless ifdefs.
  getsockname(_sock, (struct sockaddr *)&sin, &len);

  // Now that we've got the domain, remember it
  domain = sin.sin_family;
}

KSocket::KSocket( const char *_host, unsigned short int _port, int _timeout ) :
  sock( -1 ), readNotifier( 0L ), writeNotifier( 0L )
{
    timeOut = _timeout;
#ifdef INET6
    // Setup the resolver to look for IPv6 addresses and then
    // as a fall back, look for IPv4.  This means we don't need further
    // ifdefs to use gethostbyname2. (Taken from rfc2133)
    res_init();
    _res.options |= RES_USE_INET6;

    // Also we do this because gethostbyname will return IPv4 mapped on to
    // IPv6 addresses when set like this and use 16 byte hostents, so we
    // should be safe and use sockaddr_in6, etc, and by setting the domain to
    // PF_INET6, anyone who checks our sockaddr stuff will avoid buffer
    // overflows by seeing that it's new style.
    domain = PF_INET6;
#else
    domain = PF_INET;
#endif
    connect( _host, _port );
}

KSocket::KSocket( const char *_path ) :
  sock( -1 ), readNotifier( 0L ), writeNotifier( 0L )
{
  timeOut = 0; // Not used
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
  return initSockaddr(&server_name, hostname.ascii(), port, domain);
}

/*
 * Connects the PF_UNIX domain socket to _path.
 */
bool KSocket::connect( const char *_path )
{
  if ( domain != PF_UNIX )
    qFatal( "Connecting a PF_INET socket to a PF_UNIX domain socket\n");

  assert(sock == -1);
  sock = ::socket(PF_UNIX,SOCK_STREAM,0);
  if (sock < 0)
	return false;

  unix_addr.sun_family = AF_UNIX;
  int l = strlen( _path );
  if ( l > UNIX_PATH_MAX - 1 )
  {
    qWarning( "Too long PF_UNIX domain name '%s'\n",_path);
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
#ifdef INET6
  if ( (domain != PF_INET6) && (domain != PF_INET) )
#else
  if ( domain != PF_INET )
#endif
    qFatal( "Connecting a PF_UNIX domain socket to a PF_INET domain socket\n");

  sock = ::socket(domain, SOCK_STREAM, 0);
  if (sock < 0)
	return false;

  if ( !init_sockaddr( _host, _port) ) {
	  ::close( sock );
	  sock = -1;
	  return false;
  }

  fcntl(sock, F_SETFL, ( fcntl(sock,F_GETFL)|O_NDELAY) );

  errno = 0;
  if (::connect(sock, (struct sockaddr*)(&server_name), sizeof(server_name))){
      if(errno != EINPROGRESS && errno != EWOULDBLOCK) {
          ::close( sock );
          sock = -1;
          return false;
      }
  } else
      return true;

  fd_set rd, wr;
  struct timeval timeout;

  int n = timeOut*10; // Timeout in 1/10th's of a second
  while(n--){
      FD_ZERO(&rd);
      FD_ZERO(&wr);
      FD_SET(sock, &rd);
      FD_SET(sock, &wr);

      timeout.tv_usec = 100*1000; // 1/10th sec
      timeout.tv_sec = 0;

      select(sock + 1, &rd, &wr, (fd_set *)0, &timeout);

      if (FD_ISSET(sock, &rd) || FD_ISSET(sock, &wr))
      {
         int errcode;
         ksize_t len = sizeof(errcode);
         int ret = getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&errcode, &len);
         if ((ret == -1) || (errcode != 0))
         {
            ::close(sock);
            sock = -1;
            return false;
         }
         return true;
      }
      qApp->processEvents();
      qApp->flushX();
  }
  qWarning("Timeout connecting socket...\n");
  ::close( sock );
  sock = -1;
  return false;
}

unsigned long KSocket::ipv4_addr()
{
  if ( domain != PF_INET )
    return 0;

  sockaddr_in name; ksize_t len = sizeof(name);
  getsockname(sock, (struct sockaddr *) &name, &len);
  if (name.sin_family == AF_INET) // It's IPv4
    return ntohl(name.sin_addr.s_addr);
#ifdef INET6
  else if (name.sin_family == AF_INET6) // It's IPv6 Ah.
    return 0;
#endif
  else // We dunno what it is
    return 0;
}

bool KSocket::initSockaddr (ksockaddr_in *server_name, const char *hostname, unsigned short int port, int domain)
{
  if (
#ifdef INET6
     ( domain != PF_INET6 ) &&
#endif
     ( domain != PF_INET ) )
    return false;

  if (cachedHostname && (strcmp(hostname, cachedHostname) == 0))
  {
     memcpy( server_name, cachedServerName, sizeof(ksockaddr_in));
#ifdef INET6
     server_name->sin6_port = htons( port );
#else
     get_sin_pport(server_name) = htons( port );
#endif
     return true;
  }

  memset(server_name, 0, sizeof(ksockaddr_in));

  struct hostent *hostinfo;
  hostinfo = gethostbyname( hostname );

  if ( hostinfo == 0L )
    return false;

#ifdef INET6
  if (domain == PF_INET6) {
    server_name->sin6_family = hostinfo->h_addrtype;
    server_name->sin6_port = htons( port );
    memcpy(&server_name->sin6_addr, hostinfo->h_addr_list[0], hostinfo->h_length);
  } else
#endif
  {
    get_sin_pfamily(server_name) = hostinfo->h_addrtype;
    get_sin_pport(server_name) = htons( port );
    memcpy(&get_sin_paddr(server_name), hostinfo->h_addr_list[0], hostinfo->h_length);
  }

  // update our primitive cache
  delete [] cachedHostname;
  cachedHostname = new char[ strlen(hostname)+1 ];
  strcpy(cachedHostname, hostname);

  if (!cachedServerName)
     cachedServerName = new ksockaddr_in;
  memcpy( cachedServerName, server_name, sizeof(ksockaddr_in));

  return true;
}

KSocket::~KSocket()
{
	delete readNotifier;
	delete writeNotifier;
	if (sock != -1)
      ::close( sock );
}


KServerSocket::KServerSocket( const char *_path ) :
  notifier( 0L ), sock( -1 )
{
  domain = PF_UNIX;

  if ( !init ( _path ) )
  {
    qFatal("Error constructing PF_UNIX domain server socket\n");
    return;
  }

  notifier = new QSocketNotifier( sock, QSocketNotifier::Read );
  connect( notifier, SIGNAL( activated(int) ), this, SLOT( slotAccept(int) ) );
}

KServerSocket::KServerSocket( unsigned short int _port ) :
  notifier( 0L ), sock( -1 )
{
  domain = PF_INET;

  if ( !init ( _port ) )
  {
    // fatal("Error constructing\n");
    return;
  }

  notifier = new QSocketNotifier( sock, QSocketNotifier::Read );
  connect( notifier, SIGNAL( activated(int) ), this, SLOT( slotAccept(int) ) );
}

bool KServerSocket::init( const char *_path )
{
  if ( domain != PF_UNIX )
    return false;

  int l = strlen( _path );
  if ( l > UNIX_PATH_MAX - 1 )
  {
    qWarning( "Too long PF_UNIX domain name '%s'\n",_path);
    return false;
  }

  sock = ::socket( PF_UNIX, SOCK_STREAM, 0 );
  if (sock < 0)
  {
    qWarning( "Could not create socket\n");
    return false;
  }

  unlink(_path );

  struct sockaddr_un name;
  name.sun_family = AF_UNIX;
  strcpy( name.sun_path, _path );

  if ( bind( sock, (struct sockaddr*) &name,sizeof( name ) ) < 0 )
  {
    qWarning("Could not bind to socket\n");
    ::close( sock );
    sock = -1;
    return false;
  }

  if ( chmod( _path, 0600) < 0 )
  {
    qWarning("Could not setupt premissions for server socket\n");
    ::close( sock );
    sock = -1;
    return false;
  }

  if ( listen( sock, SOMAXCONN ) < 0 )
  {
    qWarning("Error listening on socket\n");
    ::close( sock );
    sock = -1;
    return false;
  }

  return true;
}

bool KServerSocket::init( unsigned short int _port )
{
  if (
#ifdef INET6
      ( domain != PF_INET6 ) &&
#endif
      ( domain != PF_INET  ) )
    return false;

  sock = ::socket( domain, SOCK_STREAM, 0 );
  if (sock < 0)
  {
    qWarning( "Could not create socket\n");
    return false;
  }

  if (domain == AF_INET) {

    sockaddr_in name;

    name.sin_family = domain;
    name.sin_port = htons( _port );
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    if ( bind( sock, (struct sockaddr*) &name,sizeof( name ) ) < 0 ) {
	  qWarning("Could not bind to socket\n");
	  ::close( sock );
	  sock = -1;
	  return false;
    }
  }
#ifdef INET6
  else if (domain == AF_INET6) {
    sockaddr_in6 name;

    name.sin6_family = domain;
    name.sin6_flowinfo = 0;
    name.sin6_port = htons(_port);
    memcpy(&name.sin6_addr, &in6addr_any, sizeof(in6addr_any));

    if ( bind( sock, (struct sockaddr*) &name,sizeof( name ) ) < 0 ) {
	  warning("Could not bind to socket\n");
	  ::close( sock );
	  sock = -1;
	  return false;
    }
  }
#endif

  if ( listen( sock, SOMAXCONN ) < 0 )
    {
	  qWarning("Error listening on socket\n");
	  ::close( sock );
	  sock = -1;
	  return false;
    }

  return true;
}

unsigned short int KServerSocket::port()
{
  if ( domain != PF_INET )
    return false;

  ksockaddr_in name; ksize_t len = sizeof(name);
  getsockname(sock, (struct sockaddr *) &name, &len);
  return ntohs(get_sin_port(name));
}

unsigned long KServerSocket::ipv4_addr()
{
  if ( domain != PF_INET )
    return 0;

  sockaddr_in name; ksize_t len = sizeof(name);
  getsockname(sock, (struct sockaddr *) &name, &len);
  if (name.sin_family == AF_INET) // It's IPv4
    return ntohl(name.sin_addr.s_addr);
#ifdef INET6
  else if (name.sin_family == AF_INET6) // It's IPv6 Ah.
    return 0;
#endif
  else // We dunno what it is
    return 0;
}

void KServerSocket::slotAccept( int )
{
  if ( domain == PF_INET )
  {
    ksockaddr_in clientname;
    int new_sock;

    ksize_t size = sizeof(clientname);

    if ((new_sock = accept (sock, (struct sockaddr *) &clientname, &size)) < 0)
    {
      qWarning("Error accepting\n");
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
      qWarning("Error accepting\n");
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
