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

#include <string.h>
#include <QVarLengthArray>
#include <QSslSocket>
#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>

#ifdef Q_OS_WIN
# include <winsock2.h>
#else
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <errno.h>
#endif

#include "config-network.h"

class _k_internal_QTcpSocketSetError: public QAbstractSocket
{
public:
    using QAbstractSocket::setSocketError;
    using QAbstractSocket::setSocketState;
    using QAbstractSocket::setErrorString;
};

static int systemServicePort(const char *servname, const char *protoname)
{
#if defined(Q_OS_WIN) || defined(Q_OS_NETBSD)
    // Windows appears to have a buffer allocated per-thread
    // See http://msdn2.microsoft.com/en-us/library/ms738538.aspx

    // NetBSD has a reentrant function as well (buffer allocated per-thread)
    // See http://bugs.kde.org/show_bug.cgi?id=127253

    struct servent *se = getservbyname(servname, protoname);
    if (se)
        return se->s_port;
    return -1;

#elif !defined(HAVE_GETSERVBYNAME_R)
# ifdef __GNUC__
#  warning "Your system does not support getservbyname_r: this might cause problems for you"
# endif

    Q_UNUSED(servname); Q_UNUSED(protoname);
    return -1;

#elif defined(Q_OS_OPENBSD)
    // OpenBSD uses an HP/IBM/DEC API
    struct servent servbuf;
    struct servent_data sdata;
    ::memset(&sdata, 0, sizeof sdata);
    if (getservbyname_r(servname, protoname, &servbuf, &sdata) == 0)
        return ntohs(servbuf->s_port);
    return -1;

#else
    QVarLengthArray<char, 1024> buf(1024);
    struct servent servbuf;
    struct servent *se;
    do {
        int error = 0;
# ifdef Q_OS_SOLARIS
        // Solaris uses a 5 argument getservbyname_r which returns struct *servent or NULL
        se = getservbyname_r(servname, protoname, &servbuf, buf.data(), buf.size());
        if (!se)
            error = errno;
# else
        // Default getservbyname_r function
        error = getservbyname_r(servname, protoname, &servbuf, buf.data(), buf.size(), &se);
# endif
        if (error == ERANGE) {
            se = 0;
            buf.resize(buf.size() + 1024);
        } else if (error) {
            // Another error?
            break;
        }
    } while (se);

    if (se)
        return ntohs(se->s_port);
    return -1;
#endif
}

static int wellKnownPorts(const QString &s)
{
    if (s.isEmpty())
        return -1;

    QByteArray service = s.toLatin1();
    switch (service.at(0)) {
    case 'f':
        if (service == "ftp")
            return 21;
        return -1;

    case 'h':
        if (service == "http")
            return 80;
        if (service == "https")
            return 443;
        return -1;

    case 'i':
        if (service == "imap")
            return 143;
        if (service == "imaps")
            return 993;
        if (service == "ipp")
            return 631;
        return -1;

    case 'l':
        if (service == "ldap")
            return 389;
        if (service == "ldaps")
            return 636;
        return -1;

    case 'n':
        if (service == "nntp")
            return 119;
        if (service == "ntp")
            return 123;
        return -1;

    case 'p':
        if (service == "pop3" || service == "pop-3")
            return 110;
        if (service == "pop3s")
            return 995;
        return -1;

    case 's':
        if (service == "ssh")
            return 22;
        return -1;

    case 't':
        if (service == "telnet")
            return 23;
        return -1;

    default:
        return -1;
    }
}

static int servicePort(const QString &service, int defaultPort, const char *protoname)
{
    int realPort = -1;
    if (!service.isEmpty())
        realPort = systemServicePort(service.toLatin1(), protoname);
    if (realPort == -1)
        realPort = defaultPort;
    if (realPort == -1)
        realPort = wellKnownPorts(service);
    return realPort;
}

static inline void setError(QAbstractSocket *socket, QAbstractSocket::SocketError error,
                            const QString &errorString)
{
    _k_internal_QTcpSocketSetError *hackSocket = static_cast<_k_internal_QTcpSocketSetError *>(socket);
    hackSocket->setSocketError(error);
    hackSocket->setErrorString(errorString);
}

QTcpSocket *KSocketFactory::connectionTo(const QString &node, const QString &service, int defaultPort,
                                         QObject *parent)
{
    // ### TO-DO: find a way to determine if we should use QSslSocket or plain QTcpSocket
    QTcpSocket *socket = new QSslSocket(parent);

    int realPort = servicePort(service, defaultPort, "tcp");
    if (realPort == -1 || realPort > 65535) {
        setError(socket, QAbstractSocket::HostNotFoundError,
                 QString::fromLatin1("Service '%1' is not known and no default port was specified")
                 .arg(service));
        return socket;
    }

    socket->setProxy(proxyForConnection(node, service.isEmpty() ? QString::number(realPort) : service));
    socket->connectToHost(node, realPort);
    return socket;
}

QTcpSocket *KSocketFactory::synchronousConnectionTo(const QString &node, const QString &service,
                                                    int defaultPort, int msecs, QObject *parent)
{
    QTcpSocket *socket = connectionTo(node, service, defaultPort, parent);
    socket->waitForConnected(msecs);
    return socket;
}

QTcpServer *KSocketFactory::listenAt(const QHostAddress &address, const QString &service,
                                     int defaultPort, QObject *parent)
{
    QTcpServer *server = new QTcpServer(parent);
    int realPort = servicePort(service, defaultPort, "tcp");
    if (realPort == -1 || realPort > 65535) {
#if 0
        // ### FIXME: find a way to do this for QTcpServer
        setError(socket, QAbstractSocket::HostNotFoundError,
                 QString::fromLatin1("Service '%1' is not known and no default port was specified")
                 .arg(service));
#endif
        return server;
    }

    server->setProxy(proxyForListening(service.isEmpty() ? QString::number(realPort) : service));
    server->listen(address, realPort);
    return server;
}

QUdpSocket *KSocketFactory::datagramSocket(const QString &node, const QString &service, QObject *parent)
{
    QUdpSocket *socket = new QUdpSocket(parent);
    int realPort = servicePort(service, 0, "udp");
    if (realPort == -1 || realPort > 65535) {
        setError(socket, QAbstractSocket::HostNotFoundError,
                 QString::fromLatin1("Service '%1' is not known and no default port was specified")
                 .arg(service));
        return socket;
    }

    // ### do something else?
    socket->setProxy(proxyForDatagram(node, service.isEmpty() ? QString::number(realPort) : service));
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
