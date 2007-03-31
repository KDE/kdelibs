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

class QTcpSocket;
class QTcpServer;
class QUdpSocket;
class QHostAddress;

/**
 * @class KSocketFactory
 * @brief KSocketFactory allows one to open sockets to remote hosts
 *
 * KSocketFactory is a socket-opener class that must be used whenever
 * a KDE application wants to communicate with a remote host. It will
 * determine the necessary proxy and local KDE settings, then open the
 * connection. The typical use-case is:
 *
 * <code>
 *   d->socket = KSocketFactory::connectionTo("www.kde.org", "http");
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
 *   d->socket = KSocketFactory::synchronousConnectionTo("www.kde.org", "http");
 *   d->socket->setParent(this);
 * </code>
 *
 * All objects returned from this class belong to the caller and must be
 * disposed of properly. Calling QObject::setParent() or passing a
 * parent object is the recommended way.
 *
 * @author Thiago Macieira <thiago@kde.org>
 */
class KDECORE_EXPORT KSocketFactory
{
public:
    /**
     * Initiates a TCP/IP socket connection to remote node (host) @p
     * node, using @p service as the service. Returns a QTcpSocket
     * that is connecting (i.e., in a state before
     * QAbstractSocket::Connected) in most cases, even if the target
     * service doesn't exist or if a connection failure happens.
     *
     * This function never returns 0. If the returned socket cannot
     * connect, it will emit the QAbstractSocket::error()
     * signal. Otherwise, the QAbstractSocket::connected() signal will
     * be emitted eventually.
     *
     * The @p service parameter <b>must</b> be a string representation
     * of the protocol being attempted, like "http", "ftp" or
     * "xmpp". It might be used to determine the proxy type -- for
     * instance, the proxy for HTTP connections might be different
     * from the proxy for other connections. Pass an empty QString if
     * the connection is of no established protocol.
     *
     * The @p defaultPort parameter can be used to specify the port
     * number lookup if the service is not a well-known service. It is
     * recommended to pass a default port in most cases. However, if
     * the service is known (for instance, "http"), the system
     * settings override the default port.
     *
     * @param node      the remote node (host) to connect to
     * @param service   the service that is wanted on the remote node
     * @param defaultPort the default port number
     * @param parent    the parent object to be passed to the
     *                  QTcpSocket constructor
     * @threadsafe
     */
    static QTcpSocket *connectionTo(const QString &node, const QString &service,
                                    int defaultPort = -1, QObject *parent = 0);

    /**
     * This function behaves exactly like connectionTo() above, except
     * that the socket it returns is either in
     * QAbstractSocket::Connected state, or it has failed connecting
     * altogether.
     *
     * This function should be used if a synchronous connection is
     * necessary instead of calling
     * QAbstractSocket::waitForConnected(), since that may not work on
     * all objects returned from connectionTo().
     *
     * This function does not return 0. If the connection attempt
     * failed for some reason, the socket state will be
     * QAbstractSocket::UnconnectedState and QAbstractSocket::isValid
     * will return false. The socket error state and string will
     * contain more information.
     * @warning: This function may block.
     *
     * @param node      the remote node (host) to connect to
     * @param service   the service that is wanted on the remote node
     * @param defaultPort the default port number
     * @param msecs     the time (in milliseconds) to wait while
     *                  attempting to connect
     * @param parent    the parent object to be passed to the
     *                  QTcpSocket constructor
     * @threadsafe
     */
    static QTcpSocket *synchronousConnectionTo(const QString &node, const QString &service,
                                               int defaultPort = -1, int msecs = 30000,
                                               QObject *parent = 0);

    /**
     * Opens a TCP/IP socket for listening service @p service, binding
     * only at address @p address. The @p defaultPort parameter is
     * used to specify the default port to be used if @p service is
     * not a well-known service. If it is, however, the system default
     * will take precedence.
     *
     * This function does not return 0. If the opening of the socket
     * failed for some reason, it will return a QTcpServer object that
     * is not listening (QTcpServer::isListening() returns false),
     * with a properly set error string indicating the reason).
     *
     * Note that passing an empty QString for @p service and 0 as the
     * default port number will cause the operating system to
     * automatically allocate a free port for the socket.
     *
     * @param address   the address to listen at
     * @param service   the service to listen for
     * @param defaultPort the default port for the service
     * @param parent    the parent object to be passed to the
     *                  QTcpServer constructor
     */
    static QTcpServer *listenAt(const QHostAddress &address, const QString &service,
                                int defaultPort = -1, QObject *parent = 0);

    /**
     * @overload
     * This function is exactly the same as the previous function if
     * it is called with QHostAddress::Any as the first parameter.
     */
    static QTcpServer *listenAt(const QString &service, int defaultPort = -1, QObject *parent = 0);

    static QUdpSocket *datagramSocket(const QString &node, const QString &service, QObject *parent = 0);

    static QNetworkProxy proxyForConnection(const QString &node, const QString &service);
    static QNetworkProxy proxyForListening(const QString &service);
    static QNetworkProxy proxyForDatagram(const QString &node, const QString &service);
};

#endif
