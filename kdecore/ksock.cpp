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
 * $Id$
 * $Log$
 * Revision 1.17  1998/01/23 11:25:28  kulow
 * Solaris doesn't define UNIX_PATH_MAX
 *
 * Revision 1.16  1998/01/23 02:23:38  torben
 * Torben: Supports UNIX domain sockets now.
 *
 * Revision 1.15  1998/01/18 14:39:03  kulow
 * reverted the changes, Jacek commited.
 * Only the RCS comments were affected, but to keep them consistent, I
 * thought, it's better to revert them.
 * I checked twice, that only comments are affected ;)
 *
 * Revision 1.13  1997/12/18 01:56:24  torben
 * Torben: Secure string operations. Use instead of QString::sprintf
 *
 * Revision 1.12  1997/11/29 17:58:48  kalle
 * Alpha patches
 *
 * Revision 1.11  1997/11/09 01:52:47  torben
 * Torben: Fixed port number bug
 *
 * Revision 1.10  1997/10/21 20:44:52  kulow
 * removed all NULLs and replaced it with 0L or "".
 * There are some left in mediatool, but this is not C++
 *
 * Revision 1.9  1997/10/16 11:15:03  torben
 * Kalle: Copyright headers
 * kdoctoolbar removed
 *
 * Revision 1.8  1997/09/18 12:16:04  kulow
 * corrected some header dependencies. Removed most of them in drag.h and put
 * them in drag.cpp. Now it should compile even under SunOS 4.4.1 ;)
 *
 * Revision 1.7  1997/08/30 08:32:54  kdecvs
 * Coolo: changed the location of the include files to get rid of the
 * hardcoded HAVE_STDC_HEADERS
 *
 * Revision 1.6  1997/07/27 13:43:58  kalle
 * Even more SGI and SCC patches, security patch for kapp, various fixes for ksock
 *
 * Revision 1.5  1997/07/17 18:43:18  kalle
 * Kalle: new stopsign.xpm
 * 			KConfig: environment variables are resolved in readEntry()
 * 			(meaning you can write() an entry with an environment
 * 			variable and read it back in, and the value gets properly
 * 			expanded).
 * 			Fixed three bugs in ksock.cpp
 * 			Added KFloater
 * 			Added KCombo
 * 			Added KLineEdit
 * 			New KToolbar
 * 			New KToplevelWidget
 *
 * Revision 1.4  1997/05/30 20:04:38  kalle
 * Kalle:
 * 30.05.97:	signal handler for reaping zombie help processes reinstalls itself
 * 		patch to KIconLoader by Christian Esken
 * 		slightly better look for KTabCtl
 * 		kdecore Makefile does not expect current dir to be in path
 * 		Better Alpha support
 *
 * Revision 1.3  1997/05/09 15:10:11  kulow
 * Coolo: patched ltconfig for FreeBSD
 * removed some stupid warnings
 *
 * Revision 1.2  1997/04/28 06:57:46  kalle
 * Various widgets moved from apps to libs
 * Added KSeparator
 * Several bugs fixed
 * Patches from Matthias Ettrich
 * Made ksock.cpp more alpha-friendly
 * Removed XPM-Loading from KPixmap
 * Reaping zombie KDEHelp childs
 * WidgetStyle of KApplication objects configurable via kdisplay
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ksock.moc"
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
#include <sys/socket.h>
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
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define UNIX_PATH_MAX 108 // this is the value, I found under Linux
#endif

KSocket::KSocket( const char *_host, unsigned short int _port ) :
  sock( -1 ), readNotifier( 0L ), writeNotifier( 0L )
{
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
bool KSocket::init_sockaddr( const char *hostname, unsigned short int port )
{
  if ( domain != PF_INET )
    return false;
  
  struct hostent *hostinfo;
  server_name.sin_family = AF_INET;
  server_name.sin_port = htons( port );
  hostinfo = gethostbyname( hostname );
  
  if ( !hostinfo )
    {
	  warning("Unknown host %s.\n",hostname);
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
bool KSocket::connect( const char *_host, unsigned short int _port )
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
  
  if ( 0 > ::connect( sock, (struct sockaddr*)(&server_name), 
					  sizeof( server_name ) ) )
	{
	  ::close( sock );
	  sock = -1;
	  return false;
	}

  return true;
}

unsigned long KSocket::getAddr()
{
  if ( domain != PF_INET )
    return 0;
  
  struct sockaddr_in name; ksize_t len = sizeof(name);
  getsockname(sock, (struct sockaddr *) &name, &len);
  return ntohl(name.sin_addr.s_addr);
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

KServerSocket::KServerSocket( int _port ) :
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

  unlink(_path);   
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
  
  if ( chmod( _path, 0666) < 0 )
  {
    warning("Could not setupt premissions for server socket\n");
    ::close( sock );
    sock = -1;
    return false;
  }
               
  if ( listen( sock, 1000 ) < 0 )
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
    
  if ( listen( sock, 1000 ) < 0 )
    {
	  warning("Error listening on socket\n");
	  ::close( sock );
	  sock = -1;
	  return false;
    }

  return true;
}

unsigned short KServerSocket::getPort()
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


