/*
 * $Id$
 * $Log$
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
 *
 * Revision 1.1.1.1  1997/04/13 14:42:41  cvsuser
 * Source imported
 *
 * Revision 1.1.1.1  1997/04/09 00:28:06  cvsuser
 * Sources imported
 *
 * Revision 1.4  1997/01/15 20:31:41  kalle
 * merged changes from 0.52
 *
 * Revision 1.3  1996/12/07 18:31:00  kalle
 * Qt debugging functions instead of printf and formatting
 *
 * Revision 1.2  1996/12/07 18:22:56  kalle
 * RCS-Header
 *
 *
 * The KDE Socket Classes.
 *
 * Torben Weis
 * weis@stud.uni-frankfurt.de
 *
 * Last change: 10.11.96
 *
 * Part of the KDE Project.
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
  sock( -1 ), readNotifier( NULL ), writeNotifier( NULL )
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
#endif

KSocket::KSocket( const char *_host, unsigned short int _port ) :
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
  }

  return true;
}
 */
bool KSocket::connect( const char *_host, unsigned short int _port )
{
  if ( domain != PF_INET )
    fatal( "Connecting a PF_UNIX domain socket to a PF_INET domain socket\n");

  sock = ::socket(PF_INET,SOCK_STREAM,0);
  if (sock < 0)
  if ( readNotifier )
	  sock = -1;
  if ( writeNotifier )
	  return false;
	}
  ::close( sock ); 
  
  if ( 0 > ::connect( sock, (struct sockaddr*)(&server_name), 
					  sizeof( server_name ) ) )
	{
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

KSocket::~KSocket()
{
    if ( readNotifier )
    {
	delete readNotifier;
    }
    if ( writeNotifier )
  }
    
  notifier = new QSocketNotifier( sock, QSocketNotifier::Read );

    {
	  fatal("Error constructing\n");
	  return;
    }

  if ( !init ( _port ) )
  {
    fatal("Error constructing\n");
    return;
    return false;
  }

bool KServerSocket::init( unsigned short int _port )
{
  if ( domain != PF_INET )
    {
	  warning( "Could not create socket\n");
	  return false;
    }
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

unsigned short KServerSocket::getPort()
{
  if ( domain != PF_INET )
    return false;

  struct sockaddr_in name; ksize_t len = sizeof(name);

unsigned long KServerSocket::getAddr()
{
  if ( domain != PF_INET )
    return false;

  struct sockaddr_in name; ksize_t len = sizeof(name);
  struct sockaddr_in clientname;
  int new_sock;
    
  ksize_t size = sizeof(clientname);
  else if ( domain == PF_UNIX )
  if ((new_sock = accept (sock, (struct sockaddr *) &clientname, &size)) < 0)
    struct sockaddr_un clientname;
	  warning("Error accepting\n");
	  return;
    ksize_t size = sizeof(clientname);
    
  emit accepted( new KSocket( new_sock ) );
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


