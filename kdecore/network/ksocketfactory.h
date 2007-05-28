/*
 * This file is part of the KDE libraries
 * Copyright (C) 2007 Thiago Macieira <thiago@kde.org>
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KSOCKETFACTORY_H
#define KSOCKETFACTORY_H

#include <kdecore_export.h>
#include <QtCore/QString>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QHostAddress>

class QTcpSocket;
class QTcpServer;
class QUdpSocket;
class QUrl;

/**
 * @namespace KSocketFactory
 * @brief KSocketFactory provides functions for opening sockets to remote hosts
 *
 * KSocketFactory is a socket-opener group of functions that must be
 * used whenever a KDE application wants to communicate with a remote
 * host. It will determine the necessary proxy and local KDE settings,
 * then open the connection. The typical use-case is:
 *
 * <code>
 *   d->socket = KSocketFactory::connectToHost("www.kde.org", "http");
 *   d->socket->setParent(this);
 *   QObject::connect(d->socket, SIGNAL(connected()),
 *                    this, SLOT(socketConnected()));
 * </code>
 *
 * Synchronous operation is not recommended, since it may lead to UI
 * deadlocks. It is preferred to return to the mainloop if the socket
 * is being created in the main GUI thread.
 *
 * However, if the socket is created in an auxiliary thread, it is
 * possible to use something equivalent to synchronous mode:
 *
 * <code>
 *   d->socket = KSocketFactory::synchronousConnectToHost("www.kde.org", "http");
 *   d->socket->setParent(this);
 * </code>
 *
 * All objects returned from these functions belong to the caller and
 * must be disposed of properly. Calling QObject::setParent() or
 * passing a parent object is the recommended way.
 *
 * @author Thiago Macieira <thiago@kde.org>
 */
namespace KSocketFactory
{
    /**
     * Initiates a TCP/IP socket connection to remote node (host) @p
     * host, using the @p protocol. Returns a QTcpSocket
     * that is connecting (i.e., in a state before
     * QAbstractSocket::Connected) in most cases, even if the target
     * service doesn't exist or if a connection failure happens.
     *
     * This function never returns 0. If the returned socket cannot
     * connect, it will emit the QAbstractSocket::error()
     * signal. Otherwise, the QAbstractSocket::connected() signal will
     * be emitted eventually.
     *
     * The @p protocol parameter <b>must</b> be a string representation
     * of the protocol being attempted, like "http", "ftp" or
     * "xmpp". It might be used to determine the proxy type -- for
     * instance, the proxy for HTTP connections might be different
     * from the proxy for other connections. Pass an empty QString if
     * the connection is of no established protocol.
     *
     * The @p port parameter can be used to specify the port
     * number lookup if the service is not a well-known service.
     *
     * @param protocol  the protocol this socket will use
     * @param host      the remote node (host) to connect to
     * @param port      the port number to connect to
     * @param parent    the parent object to be passed to the
     *                  QTcpSocket constructor
     * @threadsafe
     */
    KDECORE_EXPORT QTcpSocket *connectToHost(const QString &protocol, const QString &host,
                                             quint16 port, QObject *parent = 0);

    /**
     * @overload
     */
    KDECORE_EXPORT QTcpSocket *connectToHost(const QUrl &url, QObject *parent = 0);

    /**
     * @overload
     */
    KDECORE_EXPORT void connectToHost(QTcpSocket *socket, const QString &protocol,
                                      const QString &host, quint16 port);

    /**
     * @overload
     */
    KDECORE_EXPORT void connectToHost(QTcpSocket *socket, const QUrl &url);

    /**
     * This function behaves exactly like connectToHost() above, except
     * that the socket it returns is either in
     * QAbstractSocket::Connected state, or it has failed connecting
     * altogether.
     *
     * This function should be used if a synchronous connection is
     * necessary instead of calling
     * QAbstractSocket::waitForConnected(), since that may not work on
     * all objects returned from connectToHost().
     *
     * This function does not return 0. If the connection attempt
     * failed for some reason, the socket state will be
     * QAbstractSocket::UnconnectedState and QAbstractSocket::isValid
     * will return false. The socket error state and string will
     * contain more information.
     * @warning: This function may block.
     *
     * @param protocol  the protocol this socket will use
     * @param host      the remote node (host) to connect to
     * @param port      the port number to connect to
     * @param msecs     the time (in milliseconds) to wait while
     *                  attempting to connect
     * @param parent    the parent object to be passed to the
     *                  QTcpSocket constructor
     * @threadsafe
     */
    KDECORE_EXPORT QTcpSocket *synchronousConnectToHost(const QString &protocol,
                                                        const QString &host,
                                                        quint16 port, int msecs = 30000,
                                                        QObject *parent = 0);

    /**
     * @overload
     */
    KDECORE_EXPORT QTcpSocket *synchronousConnectToHost(const QUrl &url, int msecs = 30000,
                                                        QObject *parent = 0);

    /**
     * @overload
     */
    KDECORE_EXPORT void synchronousConnectToHost(QTcpSocket *socket, const QString &protocol,
                                                 const QString &host, quint16 port,
                                                 int msecs = 30000);

    /**
     * @overload
     */
    KDECORE_EXPORT void synchronousConnectToHost(QTcpSocket *socket, const QUrl &url,
                                                 int msecs = 30000);

    /**
     * Opens a TCP/IP socket for listening protocol @p protocol, binding
     * only at address @p address. The @p port parameter indicates the
     * port to be opened.
     *
     * This function does not return 0. If the opening of the socket
     * failed for some reason, it will return a QTcpServer object that
     * is not listening (QTcpServer::isListening() returns false),
     * with a properly set error string indicating the reason).
     *
     * Note that passing 0 as the default port number will cause the
     * operating system to automatically allocate a free port for the
     * socket.
     *
     * @param protocol  the protocol this socket will accept
     * @param address   the address to listen at
     * @param port      the default port for the service
     * @param parent    the parent object to be passed to the
     *                  QTcpServer constructor
     */
    KDECORE_EXPORT QTcpServer *listen(const QString &protocol, const QHostAddress &address = QHostAddress::Any,
                                      quint16 port = 0, QObject *parent = 0);

    // These functions below aren't done yet
    // Undocumented -> don't use!

    KDECORE_EXPORT QUdpSocket *datagramSocket(const QString &protocol, const QString &host, QObject *parent = 0);

    KDECORE_EXPORT QNetworkProxy proxyForConnection(const QString &protocol, const QString &host);
    KDECORE_EXPORT QNetworkProxy proxyForListening(const QString &protocol);
    KDECORE_EXPORT QNetworkProxy proxyForDatagram(const QString &protocol, const QString &host);
}

#endif
