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

#include "ksocketfactory.h"

#include <QSslSocket>
#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>
#include <QUrl>

#include "klocalizedstring.h"

#include <config-network.h>

using namespace KSocketFactory;

class _k_internal_QTcpSocketSetError: public QAbstractSocket
{
public:
    _k_internal_QTcpSocketSetError(); // not defined anywhere!
    using QAbstractSocket::setSocketError;
    using QAbstractSocket::setSocketState;
    using QAbstractSocket::setErrorString;
};

static inline void setError(QAbstractSocket *socket, QAbstractSocket::SocketError error,
                            const QString &errorString)
{
    _k_internal_QTcpSocketSetError *hackSocket = static_cast<_k_internal_QTcpSocketSetError *>(socket);
    hackSocket->setSocketError(error);
    hackSocket->setErrorString(errorString);
}

void KSocketFactory::connectToHost(QTcpSocket *socket, const QString &protocol, const QString &host,
                                   quint16 port)
{
    if (!socket)
        return;

    socket->setProxy(proxyForConnection(protocol, host));
    socket->connectToHost(host, port);
}

void KSocketFactory::connectToHost(QTcpSocket *socket, const QUrl &url)
{
    connectToHost(socket, url.scheme(), url.host(), url.port());
}

QTcpSocket *KSocketFactory::connectToHost(const QString &protocol, const QString &host, quint16 port,
                                          QObject *parent)
{
    // ### TO-DO: find a way to determine if we should use QSslSocket or plain QTcpSocket
    QTcpSocket *socket = new QSslSocket(parent);
    connectToHost(socket, protocol, host, port);
    return socket;
}

QTcpSocket *KSocketFactory::connectToHost(const QUrl &url, QObject *parent)
{
    return connectToHost(url.scheme(), url.host(), url.port(), parent);
}

void KSocketFactory::synchronousConnectToHost(QTcpSocket *socket, const QString &protocol,
                                              const QString &host, quint16 port, int msecs)
{
    if (!socket)
        return;

    connectToHost(socket, protocol, host, port);
    if (!socket->waitForConnected(msecs))
        setError(socket, QAbstractSocket::SocketTimeoutError,
                 i18n("Timed out trying to connect to remote host"));
}

void KSocketFactory::synchronousConnectToHost(QTcpSocket *socket, const QUrl &url, int msecs)
{
    synchronousConnectToHost(socket, url.scheme(), url.host(), url.port(), msecs);
}

QTcpSocket *KSocketFactory::synchronousConnectToHost(const QString &protocol, const QString &host,
                                                     quint16 port, int msecs, QObject *parent)
{
    QTcpSocket *socket = connectToHost(protocol, host, port, parent);
    if (!socket->waitForConnected(msecs))
        setError(socket, QAbstractSocket::SocketTimeoutError,
                 i18n("Timed out trying to connect to remote host"));
    return socket;
}

QTcpSocket *KSocketFactory::synchronousConnectToHost(const QUrl &url, int msecs, QObject *parent)
{
    return synchronousConnectToHost(url.scheme(), url.host(), url.port(), msecs, parent);
}

QTcpServer *KSocketFactory::listen(const QString &protocol, const QHostAddress &address, quint16 port,
                                   QObject *parent)
{
    QTcpServer *server = new QTcpServer(parent);
    server->setProxy(proxyForListening(protocol));
    server->listen(address, port);
    return server;
}

QUdpSocket *KSocketFactory::datagramSocket(const QString &protocol, const QString &host, QObject *parent)
{
    QUdpSocket *socket = new QUdpSocket(parent);
    // ### do something else?
    socket->setProxy(proxyForDatagram(protocol, host));
    return socket;
}

QNetworkProxy KSocketFactory::proxyForConnection(const QString &, const QString &)
{
    return QNetworkProxy::NoProxy;
}

QNetworkProxy KSocketFactory::proxyForListening(const QString &)
{
    return QNetworkProxy::NoProxy;
}

QNetworkProxy KSocketFactory::proxyForDatagram(const QString &, const QString &)
{
    return QNetworkProxy::NoProxy;
}
