/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1997 Torben Weis (weis@kde.org)
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
 */

/*
 * The KDE Socket Classes.
 *
 * Torben Weis
 * weis@stud.uni-frankfurt.de
 *
 * Part of the KDE Project.
 */

#ifndef KSOCK_H
#define KSOCK_H

#include <qobject.h>
#include <sys/types.h>
// we define STRICT_ANSI to get rid of some warnings in glibc
#ifndef __STRICT_ANSI__
#define __STRICT_ANSI__
#define _WE_DEFINED_IT_
#endif
#include <sys/socket.h>
#ifdef _WE_DEFINED_IT_
#undef __STRICT_ANSI__
#undef _WE_DEFINED_IT_
#endif
#include <sys/un.h>
#include <netinet/in.h>
class QSocketNotifier;

#ifdef INET6
typedef sockaddr_in6 ksockaddr_in;
#else
typedef sockaddr_in ksockaddr_in;
#endif

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
    KSocket( int _sock )
	: sock(_sock), domain(0), 
	readNotifier(0), writeNotifier(0) {}
    
    /** 
     * Create a socket and connect to a host.
     * @param _host	the remote host to which to connect.
     * @param _port	the port on the remote host.
     */
    KSocket( const char *_host, unsigned short int _port );
    KSocket( const char *_host, unsigned short int _port, int timeOut);
    
    /** 
     * Connects to a UNIX domain socket.
     * @param _path    the filename of the socket
     */
    KSocket( const char * _path );

    /** 
     * Destructor. Closes the socket if it is still open.
     */
    ~KSocket();

    /**
      *  A small wrapper around gethostbyname and such
      */
    static bool initSockaddr(struct ksockaddr_in *server_name, const char *hostname, unsigned short int port, int domain = PF_INET);
    
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
     * This signal will only be raised if enableRead( true ) was called
     * first.
     */
    void readEvent( KSocket * );
    
    /** 
     * Socket is ready for writing.
     *
     * This signal will only be raised if enableWrite( true ) was called
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
    bool connect( const QString& _host, unsigned short int _port );
    bool connect( const char *_path );
  
    bool init_sockaddr( const QString& hostname, unsigned short int port );
    
    struct ksockaddr_in server_name;
    struct sockaddr_un unix_addr;

    /******************************************************
     * The file descriptor for this socket. sock may be -1.
     * This indicates that it is not connected.
     */
    int sock;

    int domain;
  
    QSocketNotifier *readNotifier;
    QSocketNotifier *writeNotifier;

private:
    int timeOut;
    
    KSocket(const KSocket&);
    KSocket& operator=(const KSocket&);
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
    KServerSocket( unsigned short int _port );

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
    unsigned short int getPort();

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
    bool init( const char *_path );
  
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

private:
    KServerSocket(const KServerSocket&);
    KServerSocket& operator=(const KServerSocket&);
};

#endif
