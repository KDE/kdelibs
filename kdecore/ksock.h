/*
 * $Id$
 *
 * $Log$
 * Revision 1.8  1997/10/16 11:15:03  torben
 * Kalle: Copyright headers
 * kdoctoolbar removed
 *
 * Revision 1.7  1997/09/18 12:16:05  kulow
 * corrected some header dependencies. Removed most of them in drag.h and put
 * them in drag.cpp. Now it should compile even under SunOS 4.4.1 ;)
 *
 * Revision 1.6  1997/08/30 08:32:56  kdecvs
 * Coolo: changed the location of the include files to get rid of the
 * hardcoded HAVE_STDC_HEADERS
 *
 * Revision 1.5  1997/07/27 13:43:59  kalle
 * Even more SGI and SCC patches, security patch for kapp, various fixes for ksock
 *
 * Revision 1.4  1997/07/25 19:46:43  kalle
 * SGI changes
 *
 * Revision 1.3  1997/07/18 05:49:18  ssk
 * Taj: All kdecore doc now in javadoc format (hopefully).
 *
 * Revision 1.2  1997/06/25 14:22:13  ssk
 * Taj: updated some documentation.
 *
 * Revision 1.1.1.1  1997/04/13 14:42:42  cvsuser
 * Source imported
 *
 * Revision 1.1.1.1  1997/04/09 00:28:07  cvsuser
#include <stdio.h>
#include <errno.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#else
#error Declare functions needed from stdlib.h and string.h
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#else
#error Declare functions needed from unistd.h
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
 *
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
 * Revision 1.4  1997/01/15 20:34:14  kalle
 * merged changes from 0.52
 *
/// KSocket: Internet connections in KDE
/** This is a normal socket. You can connect this socket to any internet 
  address.
  The socket gives you three signals: When ready for reading/writing or if the
  connection is broken.
  Using socket() you get a file descriptor which you can use with usual unix
  commands like write(..) or read(...).
  If you have already such a socket identifier you can construct a KSocket on
  this identifier.
  If socket() delivers a value of -1 or less, the connection had no success.
  */
* @author Torben Weis <weis@uni-frankfurt.de>
* @version $Id$
  Q_OBJECT
*/
  /// Constructor with file descriptor
  /**
	Create a KSocket with the file descriptor _sock.
	*/
  KSocket( int _sock ) { sock = _sock; readNotifier = 0L; writeNotifier = 0L; }
* Create a KSocket with the provided file descriptor.
  /// Constructor with host and port
  /**
	Create a socket and connect to _host:_port.
	*/
* Create a socket and connect to a host.
* @param _host	the remote host to which to connect.
  /// Destructor, closes the socket.
  /**
	Close the socket.
	*/

/** 
  /// Return the file descriptor
  /**
	Returns a file descriptor for this socket.
	*/
  int socket() { return sock; }
/** 
  /// Enable the socket for reading.
  /**
	If you enable read mode, the socket will emit the signal
	readEvent whenever there is something to read out of this
    socket.
	*/
* If you enable read mode, the socket will emit the signal
* readEvent whenever there is something to read out of this
  /// Enable the socket for writing.
  /**
	If you enable write mode, the socket will emit the signal
	writeEvent whenever the socket is ready for writing.
	*/
*
* If you enable write mode, the socket will emit the signal
  /// Return address.
  long getAddr();
/**
  //@Man: signals
  //@{
* Return address.
  /// Data has arrived.
  /**
	Tells that data has arrived on the socket. You must call
	enableRead( TRUE ) before you get this signal.
	*/
*
* This signal will only be raised if enableRead( TRUE ) was called
  /// Socket is ready for writing.
  /**
	Tells that the socket is ready for writing. You must call
	enableWrite( TRUE ) before you get this signal.
	*/
*
* This signal will only be raised if enableWrite( TRUE ) was called
  /// The connection is broken.
  /**
	Tells you that the connection is broken.
	*/

/** 
  //@}

  //@Man: slots
  //@{
* Raised when the connection is broken.
 /// Slot for the writeNotifier
 /**
   Connected to the writeNotifier.
   */
 public slots:
/** 
  /// Slot for the readNotifier
  /**
	Connected to the readNotifier.
	*/

  //@}
/** 
* Connected to the readNotifier.
*/
  void slotRead( int );

     * Socket is ready for writing.
  bool connect( const char *_host, unsigned short int _port );
  bool init_sockaddr( const char *hostname, unsigned short int port );
  
  struct sockaddr_in server_name;
  
  /******************************************************
   * The file descriptor for this socket. sock may be -1.
   * This indicates that it is not connected.
   */
  int sock;
  
  QSocketNotifier *readNotifier;
/// KServerSocket: Listen on a port.
/**
  You can use a KServerSocket to listen on a port for incoming
  connections. When a connection arrived in the port, a KSocket
  is created and the signal accepted is raised. Make shure you
  always connect to this signal. If you dont the ServerSocket will
  create new KSocket's and no one will delete them!
  If socket() is -1 or less the socket was not created properly.
 */
* @author Torben Weis <weis@stud.uni-frankfurt.de>
* @version $Id$
  Q_OBJECT
    int sock;
  /// Constructor.
  /**
    Listen on port _port for incoming connections.
	*/
/**
* Constructor.
  /// Destructor.
  /**
	Close the port.
	*/

/** 
  /// Return the file descriptor.
  /**
	Get the file descriptor assoziated with the socket.
	*/
  int socket() { return sock; }
/** 
  /// Return the port.
  /**
	Get the port the server is listening on.
	*/
  int getPort();
/** 
  /// Return address.
  long getAddr();
/** 
  //@Man: slots
  //@{
  public slots: 
  /// Someone connects.
  /**
	Called when someone connected to our port.
	*/
     */
  //@}
/** 
  //@Man: signals
  //@{
  signals:
  /// A connection has been accepted.
  /**
	Tells you that someone connected to the port. It is your
    task to delete the KSocket if it is no longer needed.
	*/
    ~KServerSocket();
  //@}
/** A connection has been accepted.
* It is your task to delete the KSocket if it is no longer needed.
*/
  void accepted( KSocket* );
  /******************************************************
   * Notifies us when there is something to read on the port.
   */
     */
/** 
  /******************************************************
   * The file descriptor for this socket. sock may be -1.
   * This indicates that it is not connected.
   */    
/** 
* The file descriptor for this socket. sock may be -1.
* This indicates that it is not connected.
*/    
  int sock;  
     */
    virtual void slotAccept( int );

signals:
    /**
     * A connection has been accepted.

    void accepted( KSocket* );

protected:
    bool init( short unsigned int );
    bool init( const char* _path );
  
    /** 
     * Notifies us when there is something to read on the port.
     */
    QSocketNotifier *notifier;
     * The file descriptor for this socket. sock may be -1.
     * This indicates that it is not connected.
     */    
    int sock;  

    int domain;
};

#endif


