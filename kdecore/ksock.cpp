/*
 * $Id$
 * $Log$
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
 * 			Added KFloater

#ifdef ALPHA
#define ksize_t size_t
#else
#define ksize_t int
#endif

KSocket::KSocket( const char *_host, unsigned short int _port )
#include <errno.h>
  if ( !connect ( _host, _port ) )
    {
	  sock = -1;
	  return;
    }
    
  readNotifier = 0L; 
  writeNotifier = 0L;
#include <stdlib.h>
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
	  if ( readNotifier == 0L )
#include <sysent.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
	    readNotifier->setEnabled( TRUE );
# include <sys/time.h>
  else if ( readNotifier == 0L )
	readNotifier->setEnabled( FALSE );
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
	  if ( writeNotifier == 0L )
#include <netinet/in.h>
#include <arpa/inet.h>
		  QObject::connect( readNotifier, SIGNAL( activated(int) ), this, 
#endif

KSocket::KSocket( const char *_host, unsigned short int _port ) :
	    writeNotifier->setEnabled( TRUE );
{
  else if ( writeNotifier == 0L )
	writeNotifier->setEnabled( FALSE );
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
  if ( hostinfo == 0L )
  else if ( writeNotifier )
	writeNotifier->setEnabled( false );
	  return FALSE;	

void KSocket::slotRead( int )
{
  return TRUE;
  
  int n = recv( sock, buffer, 1, MSG_PEEK );
  if ( n <= 0 )
	emit closeEvent( this );
  else
	emit readEvent( this );
}

void KSocket::slotWrite( int )
	return FALSE;
  emit writeEvent( this );
}
	return FALSE;
  if ( domain != PF_INET )
    return false;
  
	return FALSE;    
  if ( !hostinfo )
  return TRUE;
	  warning("Unknown host %s.\n",hostname);
	  return false;	
long KSocket::getAddr()
  }

  return true;
}
 */
bool KSocket::connect( const char *_host, unsigned short int _port )
{
  if ( domain != PF_INET )
  if ( readNotifier != 0L )

  if ( writeNotifier != 0L )
  if (sock < 0)
  if ( readNotifier )
  close( sock ); 
  if ( writeNotifier )
	  return false;
	}
KServerSocket::KServerSocket( int _port )
  if ( 0 > ::connect( sock, (struct sockaddr*)(&server_name), 
					  sizeof( server_name ) ) )
	{
	  sock = -1;
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
	  return FALSE;
    if ( readNotifier )
    {
	delete readNotifier;
    }
    if ( writeNotifier )
  }
    
  notifier = new QSocketNotifier( sock, QSocketNotifier::Read );

	  return FALSE;
    }

  if ( !init ( _port ) )
  {
    fatal("Error constructing\n");
	  return FALSE;

bool KServerSocket::init( unsigned short int _port )
  return TRUE;
  if ( domain != PF_INET )
    {
int KServerSocket::getPort()
	  return false;
    }
  sock = ::socket( PF_INET, SOCK_STREAM, 0 );
  if (sock < 0)
  {
    warning( "Could not create socket\n");
long KServerSocket::getAddr()
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
  if ( notifier != 0L )
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


