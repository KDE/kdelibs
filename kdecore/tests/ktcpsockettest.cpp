/*
 * This file is part of the KDE libraries
 * Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>
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

#include "ktcpsockettest.h"
#include <QtCore/QDebug>
#include <QtCore/QThread>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include "ktcpsocket.h"

/* TODO items:
 - test errors including error strings
 - test overriding errors
 - test the most important SSL operations (full coverage is very hard)
 - test readLine()
 - test nonblocking, signal based usage
 - test that waitForDisconnected() writes out all buffered data
 - (test local and peer address and port getters)
 - test isValid(). Its documentation is less than clear :(
 */

static const quint16 testPort = 22342;

class ServerThread : public QThread
{
public:
    Server *volatile server;
    ServerThread()
     : server(0) {}
    ~ServerThread() { wait(100); }
protected:
    virtual void run();
};

KTcpSocketTest::KTcpSocketTest()
{
    server = 0;
}

KTcpSocketTest::~KTcpSocketTest()
{
    delete server;
}

void KTcpSocketTest::invokeOnServer(const char *method)
{
    QMetaObject::invokeMethod(server, method, Qt::QueuedConnection);
    QTest::qWait(1); //Enter the event loop
}


void ServerThread::run()
{
    server = new Server(testPort);
    exec(); //Start the event loop; this won't return.
}


Server::Server(quint16 _port)
 : listener(0),
   socket(0),
   port(_port)
{
    listener = new QTcpServer();
    listener->listen(QHostAddress("127.0.0.1"), testPort);
}

void Server::cleanupSocket()
{
    Q_ASSERT(socket);
    socket->close();
    socket->deleteLater();
    socket = 0;
}


void KTcpSocketTest::initTestCase()
{
    ServerThread *st = new ServerThread();
    st->start();
    while (!st->server)
        ;
    //Let the other thread initialize its event loop or whatever; there were problems...
    QTest::qWait(200);
    server = st->server;
}


void KTcpSocketTest::connectDisconnect()
{
    invokeOnServer("connectDisconnect");

    KTcpSocket *s = new KTcpSocket(this);
    QCOMPARE(s->openMode(), QIODevice::NotOpen);
    QCOMPARE(s->error(), KTcpSocket::UnknownError);

    s->connectToHost("127.0.0.1", testPort);
    QCOMPARE(s->state(), KTcpSocket::ConnectingState);
    QVERIFY(s->openMode() & QIODevice::ReadWrite);
    const bool connected = s->waitForConnected(150);
    QVERIFY(connected);
    QCOMPARE(s->state(), KTcpSocket::ConnectedState);

    s->waitForDisconnected(150);
    //ClosingState occurs only when there is buffered data
    QCOMPARE(s->state(), KTcpSocket::UnconnectedState);

    s->deleteLater();
}

void Server::connectDisconnect()
{
    listener->waitForNewConnection(10000, 0);
    socket = listener->nextPendingConnection();

    cleanupSocket();
}


#define TESTDATA QByteArray("things and stuff and a bag of chips")

void KTcpSocketTest::read()
{
    invokeOnServer("read");

    KTcpSocket *s = new KTcpSocket(this);
    s->connectToHost("127.0.0.1", testPort);
    s->waitForConnected(40);
    s->waitForReadyRead(40);
    QCOMPARE((int)s->bytesAvailable(), TESTDATA.size());
    QCOMPARE(s->readAll(), TESTDATA);
    s->deleteLater();
}

void Server::read()
{
    listener->waitForNewConnection(10000, 0);
    socket = listener->nextPendingConnection();

    socket->write(TESTDATA);
    socket->waitForBytesWritten(150);
    cleanupSocket();
}


void KTcpSocketTest::write()
{
    invokeOnServer("write");

    KTcpSocket *s = new KTcpSocket(this);
    s->connectToHost("127.0.0.1", testPort);
    s->waitForConnected(40);
    s->write(TESTDATA);
    QCOMPARE((int)s->bytesToWrite(), TESTDATA.size());
    s->waitForReadyRead(150);
    QCOMPARE((int)s->bytesAvailable(), TESTDATA.size());
    QCOMPARE(s->readAll(), TESTDATA);

    s->write(TESTDATA);
    QCOMPARE((int)s->bytesToWrite(), TESTDATA.size());
    s->disconnectFromHost();
    //Test closing with pending data to transmit (pending rx data comes later)
    QCOMPARE(s->state(), KTcpSocket::ClosingState);
    s->waitForDisconnected(150);
    QCOMPARE(s->state(), KTcpSocket::UnconnectedState);

    s->deleteLater();
}

void Server::write()
{
    listener->waitForNewConnection(10000, 0);
    socket = listener->nextPendingConnection();

    socket->waitForReadyRead(40);
    socket->write(socket->readAll()); //echo
    socket->waitForBytesWritten(150);

    socket->waitForReadyRead(40);
    socket->write(socket->readAll());
    cleanupSocket();
}


#define HTTPREQUEST QByteArray("GET / HTTP/1.1\nHost: www.example.com\n\n")

void KTcpSocketTest::states()
{
    //A connection to a real internet host
    KTcpSocket *s = new KTcpSocket(this);
    connect(s, SIGNAL(hostFound()), this, SLOT(states_hostFound()));
    QCOMPARE(s->state(), KTcpSocket::UnconnectedState);
    s->connectToHost("www.example.com", 80);
    QCOMPARE(s->state(), KTcpSocket::HostLookupState);
    s->write(HTTPREQUEST);
    QCOMPARE(s->state(), KTcpSocket::HostLookupState);
    s->waitForBytesWritten(2500);
    QCOMPARE(s->state(), KTcpSocket::ConnectedState);
    s->waitForReadyRead(2500);
    //I actually assume that the page delivered by example.com will not change for years
    QVERIFY(s->bytesAvailable() > 200);

    s->write(HTTPREQUEST);
    s->waitForReadyRead();
    s->close();
    //What happens is that during waitForReadyRead() the write buffer is written out
    //completely so that the socket can shut down without having to wait for writeout.
    QCOMPARE((int)s->state(), (int)KTcpSocket::UnconnectedState);

    //Now again an internal connection
    invokeOnServer("states");

    s->connectToHost("127.0.0.1", testPort);
    QCOMPARE(s->state(), KTcpSocket::ConnectingState);
    s->waitForConnected(40);
    QCOMPARE(s->state(), KTcpSocket::ConnectedState);

    s->write(HTTPREQUEST);
    s->waitForReadyRead();
    QCOMPARE((int)s->bytesAvailable(), HTTPREQUEST.size()); //for good measure...
    QCOMPARE(s->state(), KTcpSocket::ConnectedState);

    s->waitForDisconnected(40);
    QCOMPARE(s->state(), KTcpSocket::UnconnectedState);

    disconnect(s, SIGNAL(hostFound()));

    QByteArray requestProlog("GET /  HTTP/1.1\r\n"         //exact copy of a real HTTP query
                             "Connection: Keep-Alive\r\n"  //not really...
                             "User-Agent: Mozilla/5.0 (compatible; Konqueror/3.96; Linux) "
                              "KHTML/3.96.0 (like Gecko)\r\n"
                             "Pragma: no-cache\r\n"
                             "Cache-control: no-cache\r\n"
                             "Accept: text/html, image/jpeg, image/png, text/*, image/*, */*\r\n"
                             "Accept-Encoding: x-gzip, x-deflate, gzip, deflate\r\n"
                             "Accept-Charset: utf-8, utf-8;q=0.5, *;q=0.5\r\n"
                             "Accept-Language: en-US, en\r\n"
                             "Host: ");
    QByteArray requestEpilog("\r\n\r\n");
    //Test rapid connection and disconnection to different hosts
    const char *hosts[] = {"www.google.de", "www.spiegel.de", "www.stern.de", "www.laut.de"};
    for (int i = 0; i < 20; i++) {
        QCOMPARE(s->state(), KTcpSocket::UnconnectedState);
        s->connectToHost(hosts[i % 4], 80);
        QCOMPARE(s->state(), KTcpSocket::HostLookupState);
        //weave the host address into the HTTP request
        QByteArray request(requestProlog);
        request.append(hosts[i % 4]);
        request.append(requestEpilog);
        qDebug("%s", hosts[i % 4]);
        s->write(request);

        QCOMPARE(s->state(), KTcpSocket::HostLookupState);
        s->waitForBytesWritten(-1);
        QCOMPARE(s->state(), KTcpSocket::ConnectedState);
        s->waitForReadyRead(-1);
        QVERIFY(s->bytesAvailable() > 100);
        if (i % 5) {
            s->readAll();
            QVERIFY(s->bytesAvailable() == 0);
        } else {
            char dummy[4];
            s->read(dummy, 1);
            QVERIFY(s->bytesAvailable() > 100 - 1);
        }
        s->disconnectFromHost();
        if (s->state() != KTcpSocket::UnconnectedState)
            s->waitForDisconnected(-1);
        if (i % 2)
            s->close();     //close() is not very well defined for sockets so just check that it
                            //does no harm
    }

    s->deleteLater();
}

void KTcpSocketTest::states_hostFound()
{
    QCOMPARE(static_cast<KTcpSocket *>(sender())->state(), KTcpSocket::ConnectingState);
}

void Server::states()
{
    listener->waitForNewConnection(10000, 0);
    socket = listener->nextPendingConnection();

    socket->waitForReadyRead(40);
    socket->write(socket->readAll()); //echo
    socket->waitForBytesWritten(150);

    cleanupSocket();
}


void KTcpSocketTest::errors()
{
    //invokeOnServer("errors");
}

void Server::errors()
{
    listener->waitForNewConnection(10000, 0);
    socket = listener->nextPendingConnection();

    cleanupSocket();
}


QTEST_MAIN(KTcpSocketTest)

#include "ktcpsockettest.moc"
