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
 *
 * $Log$
 * Revision 1.15  1998/01/24 11:13:58  kulow
 * changed the order of the header files
 *
 * Revision 1.14  1998/01/24 00:12:02  kulow
 * added sys/socket.h
 *
 * Revision 1.13  1998/01/23 02:23:40  torben
 * Torben: Supports UNIX domain sockets now.
 *
 * Revision 1.12  1998/01/18 14:39:05  kulow
 * reverted the changes, Jacek commited.
 * Only the RCS comments were affected, but to keep them consistent, I
 * thought, it's better to revert them.
 * I checked twice, that only comments are affected ;)
 *
 * Revision 1.10  1997/12/18 01:56:25  torben
 * Torben: Secure string operations. Use instead of QString::sprintf
 *
 * Revision 1.9  1997/10/21 20:44:53  kulow
 * removed all NULLs and replaced it with 0L or "".
 * There are some left in mediatool, but this is not C++
 *
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
 * Sources imported
 *
 * Revision 1.4  1997/01/15 20:34:14  kalle
 * merged changes from 0.52
 *
 * Revision 1.3  1996/12/07 22:23:07  kalle
 * autoconf, documentation
 *
 * Revision 1.2  1996/12/07 18:32:00  kalle
 * RCS header
 *
 *
 * The KDE Socket Classes.
 *
 * Torben Weis
 * weis@stud.uni-frankfurt.de
 *
 * Part of the KDE Project.
 */

#ifndef KSOCK_H
#define KSOCK_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/socket.h> 
#include <sys/un.h>
#include <netinet/in.h>

#include <qsocknot.h>

/** 
 * A TCP/IP client socket. You can connect this socket to any internet address. 
 *
 * The socket gives you three signals: When ready for reading/writing or 
 * if the connection is broken.  Using socket() you get a file descriptor
 * which you can use with usual unix commands like write(..) or read(...). 
 * If you have already such a socket identifier you can construct a KSocket
 * on this identifier. 
 *  
 * If socket() delivers a value of -1 or less, the connection 
 * had no success.
 *
 * @author Torben Weis <weis@uni-frankfurt.de>
 * @version $Id$
 * @short a TCP/IP client socket.
 */
class KSocket : public QObject
{
    Q_OBJECT
public:
    /** 
     * Create a KSocket with the provided file descriptor.
     * @param _sock	the file descriptor to use.
     */
    KSocket( int _sock ) { sock = _sock; readNotifier = 0L; writeNotifier = 0L; }
    
    /** 
     * Create a socket and connect to a host.
     * @param _host	the remote host to which to connect.
     * @param _port	the port on the remote host.
     */
    KSocket( const char *_host, unsigned short int _port );
    
    /** 
     * Connects to a UNIX domain socket.
     * @param _path    the filename of the socket
     */
    KSocket( const char *_path );

    /** 
     * Destructor. Closes the socket if it is still open.
     */
    ~KSocket();
    
    /** 
     * Returns a file descriptor for this socket.
     */
    int socket() const { return sock; }
    
    /** 
     * Enable the socket for reading.
     *
     * If you enable read mode, the socket will emit the signal
     * readEvent whenever there is something to read out of this
     * socket.
     */
    void enableRead( bool );
    
    /** 
     * Enable the socket for writing.
     *
     * If you enable write mode, the socket will emit the signal
     * writeEvent whenever the socket is ready for writing.
     */
    void enableWrite( bool );
    
    /**
     * Return address.
     */
    unsigned long getAddr();
    
signals:
    /** 
     * Data has arrived for reading.
     *
     * This signal will only be raised if enableRead( TRUE ) was called
     * first.
     */
    void readEvent( KSocket * );
    
    /** 
     * Socket is ready for writing.
     *
     * This signal will only be raised if enableWrite( TRUE ) was called
     * first.
     */
    void writeEvent( KSocket * );
    
    /** 
     * Raised when the connection is broken.
     */
    void closeEvent( KSocket * );
    
public slots:
    /** 
     * Connected to the writeNotifier.
     */
    void slotWrite( int );
    
    /** 
     * Connected to the readNotifier.
     */
    void slotRead( int );
    
protected:
    bool connect( const char *_host, unsigned short int _port );
    bool connect( const char *_path );
  
    bool init_sockaddr( const char *hostname, unsigned short int port );
    
    struct sockaddr_in server_name;
    struct sockaddr_un unix_addr;

    /******************************************************
     * The file descriptor for this socket. sock may be -1.
     * This indicates that it is not connected.
     */
    int sock;

    int domain;
  
    QSocketNotifier *readNotifier;
    QSocketNotifier *writeNotifier;
};


/**
 * Monitor a port for incoming TCP/IP connections.
 *
 * You can use a KServerSocket to listen on a port for incoming
 * connections. When a connection arrived in the port, a KSocket
 * is created and the signal accepted is raised. Make sure you
 * always connect to this signal. If you dont the ServerSocket will
 * create new KSocket's and no one will delete them!
 *
 * If socket() is -1 or less the socket was not created properly.
 *
 * @author Torben Weis <weis@stud.uni-frankfurt.de>
 * @version $Id$
 * @short Monitor a port for incoming TCP/IP connections.
*/
class KServerSocket : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param _port	the port number to monitor for incoming connections.
     */
    KServerSocket( int _port );

    /**
     * Creates a UNIX domain server socket.
     */
    KServerSocket( const char *_path );
  
    /** 
     * Destructor. Closes the socket if it was not already closed.
     */
    ~KServerSocket();
    
    /** 
     * Get the file descriptor assoziated with the socket.
     */
    int socket() const { return sock; }

    /** 
     * Returns the port number which is being monitored.
     */
    unsigned short getPort();

    /** 
     * The address.
     */
    unsigned long getAddr();

public slots: 
    /** 
     * Called when someone connected to our port.
     */
    virtual void slotAccept( int );

signals:
    /**
     * A connection has been accepted.
     * It is your task to delete the KSocket if it is no longer needed.
     */
    void accepted( KSocket* );

protected:
    bool init( short unsigned int );
    bool init( const char* _path );
  
    /** 
     * Notifies us when there is something to read on the port.
     */
    QSocketNotifier *notifier;
    
    /** 
     * The file descriptor for this socket. sock may be -1.
     * This indicates that it is not connected.
     */    
    int sock;  

    int domain;
};

#endif


