/* This file is part of the KDE project
 *
 * Copyright (C) 2001 - 2003 Mirko Boehm <mirko@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 
 * This file declares a server socket that initiates SSL encrypted
 * connections.

 * $ Author: Mirko Boehm $
 * $ Copyright: (C) 2003, Mirko Boehm $
 * $ Contact: mirko@kde.org
 *            http://www.kde.org
 *            http://www.hackerbuero.org $

 * $Id$
*/

#ifndef SSLSERVERSOCKET_H
#define SSLSERVERSOCKET_H

extern "C" {
#include <openssl/ssl.h>
}

#include <qobject.h>
#include <qhostaddress.h>
#include <qsocketdevice.h>
#include <qsocketnotifier.h>

// #include "public.h"

/** This code mostly resembles what is in QServerSocket. Because of
    its unaccessable parts declared private, I had to write a new
    class. Anyway, the functionality is different enough to keep it
    that way.

    Anyway, see QServerSocket to get the idea what this class does.
*/

class SSLServerSocket : public QObject
{
  Q_OBJECT
public:
  /** Construct a SSLServerSocket. So far, the auxilliary OpenSSL
  structs (SSL_METHOD and SSL_CTX) have not been encapsulated in C++
  objects. If these will become more widely used, I might change
  that.
  @arg meth needs to be initialized, so does @arg ctx. The other
  arguments do not diffe from QServerSocket.
  */
  SSLServerSocket(SSL_METHOD *meth_, SSL_CTX *ctx_,
		  int port, int backlog=1, QObject *parent=0,
		  const char *name=0);
  /** The dtor.
   */
  ~SSLServerSocket();
  /** Initialize the SSL method and context used. This method does NOT
      initialize the SSL library, as we cannot expect to be the only SSL
      user in the programs context. So make sure to call
      SSL_load_error_strings() and SSL_library_init() somewhere
      before.
      This method is executed once.
      @return False if either SSL method or context where not created
      successfully
  */
  virtual bool Init();
  /** True if the socket was successfully created.
   */
  bool ok() const;
  /** Returns the port number the server listens.
   */
  int port() const;
  /** Returns the operating system socket.
   */
  int socket() const;
  /** Set the socket to sock. Bind and listen should already have been
      called for sock.
      Note that the SSL accept is performed by the SSLIODevice's
      used.
  */
  void setSocket(int sock);
  /** Returns the address this server listens to.
   */
  QHostAddress address() const;
  /** Overload this pure virtual method to accept new connections.
   */
  virtual void newConnection(int socket)=0;
protected:
  /** Returns the internal socket device.
   */
  QSocketDevice *socketDevice();
  static bool Initialized;
  /** The SSL method used. */
  SSL_METHOD *meth;
  /** The SSL context used. */
  SSL_CTX *ctx;
  /** A SSL object, only valid after a connection came in. */
  SSL *ssl;
  /** The socket device used to listen.
   */
  QSocketDevice *sd;
  /** The socket notifier.
   */
  QSocketNotifier *sn;
protected slots:
  /** Handle incoming connections before calling newConnection().
   */
  void incomingConnection(int socket);
};

#endif // SSLSERVERSOCKET_H
