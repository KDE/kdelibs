/*
   Copyright (c) 2000 Bernd Johannes Wuebben <wuebben@math.cornell.edu>
   Copyright (c) 2000 Stephan Kulow <coolo@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "smtp.h"

#include <sys/utsname.h>
#include <unistd.h>
#include <stdio.h>

#include <kdebug.h>

SMTP::SMTP(char *serverhost, unsigned short int port, int timeout)
{
    struct utsname uts;

    serverHost = serverhost;
    hostPort = port;
    timeOut = timeout * 1000;

    senderAddress = "user@example.net";
    recipientAddress = "user@example.net";
    messageSubject = "(no subject)";
    messageBody = "empty";
    messageHeader = "";

    connected = false;
    finished = false;

    sock = 0L;
    state = Init;
    serverState = None;

    uname(&uts);
    domainName = uts.nodename;


    if(domainName.isEmpty())
        domainName = "somemachine.example.net";

    kDebug() << "SMTP object created";

    connect(&connectTimer, SIGNAL(timeout()), this, SLOT(connectTimerTick()));
    connect(&timeOutTimer, SIGNAL(timeout()), this, SLOT(connectTimedOut()));
    connect(&interactTimer, SIGNAL(timeout()), this, SLOT(interactTimedOut()));

    // some sendmail will give 'duplicate helo' error, quick fix for now
    connect(this, SIGNAL(messageSent()), SLOT(closeConnection()));
}

SMTP::~SMTP()
{
    if(sock){
        delete sock;
        sock = 0L;
    }
    connectTimer.stop();
    timeOutTimer.stop();
}

void SMTP::setServerHost(const QString& serverhost)
{
    serverHost = serverhost;
}

void SMTP::setPort(unsigned short int port)
{
    hostPort = port;
}

void SMTP::setTimeOut(int timeout)
{
    timeOut = timeout;
}

void SMTP::setSenderAddress(const QString& sender)
{
    senderAddress = sender;
    int index = senderAddress.indexOf('<');
    if (index == -1)
        return;
    senderAddress = senderAddress.mid(index + 1);
    index =  senderAddress.indexOf('>');
    if (index != -1)
        senderAddress = senderAddress.left(index);
    senderAddress = senderAddress.simplified();
    while (1) {
        index =  senderAddress.indexOf(' ');
        if (index != -1)
            senderAddress = senderAddress.mid(index + 1); // take one side
        else
            break;
    }
    index = senderAddress.indexOf('@');
    if (index == -1)
        senderAddress.append("@localhost"); // won't go through without a local mail system

}

void SMTP::setRecipientAddress(const QString& recipient)
{
    recipientAddress = recipient;
}

void SMTP::setMessageSubject(const QString& subject)
{
    messageSubject = subject;
}

void SMTP::setMessageBody(const QString& message)
{
    messageBody = message;
}

void SMTP::setMessageHeader(const QString &header)
{
    messageHeader = header;
}

void SMTP::openConnection(void)
{
    kDebug() << "started connect timer";
    connectTimer.setSingleShot(true);
    connectTimer.start(100);
}

void SMTP::closeConnection(void)
{
    socketClosed();
}

void SMTP::sendMessage(void)
{
    if(!connected)
        connectTimerTick();
    if(state == Finished && connected){
        kDebug() << "state was == Finished\n";
        finished = false;
        state = In;
        writeString = QString::fromLatin1("helo %1\r\n").arg(domainName);
        sock->write(writeString.toAscii().constData(), writeString.length());
    }
    if(connected){
        kDebug() << "enabling read on sock...\n";
        interactTimer.setSingleShot(true);
        interactTimer.start(timeOut);
    }
}

void SMTP::connectTimerTick(void)
{
    connectTimer.stop();
//    timeOutTimer.start(timeOut, true);

    kDebug() << "connectTimerTick called...";

    if(sock){
        delete sock;
        sock = 0L;
    }

    kDebug() << "connecting to " << serverHost << ":" << hostPort << " ..... ";
    sock = KSocketFactory::connectToHost("smtp", serverHost, hostPort, this);

    connected = true;
    finished = false;
    state = Init;
    serverState = None;

    connect(sock, SIGNAL(readyRead()), this, SLOT(socketReadyToRead()));
    connect(sock, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(socketError(QAbstractSocket::SocketError)));
    connect(sock, SIGNAL(disconnected()), this, SLOT(socketClosed()));
    timeOutTimer.stop();
    kDebug() << "connected";
}

void SMTP::connectTimedOut(void)
{
    timeOutTimer.stop();

    kDebug() << "socket connection timed out";
    socketClosed();
    emit error(ConnectTimeout);
}

void SMTP::interactTimedOut(void)
{
    interactTimer.stop();

    kDebug() << "time out waiting for server interaction";
    socketClosed();
    emit error(InteractTimeout);
}

void SMTP::socketReadyToRead()
{
    int n, nl;

    kDebug() << "socketRead() called...";
    interactTimer.stop();

    if (!sock)
        return;

    n = sock->read(readBuffer, SMTP_READ_BUFFER_SIZE-1);
    if (n < 0)
	return;
    readBuffer[n] = 0;
    lineBuffer += readBuffer;
    nl = lineBuffer.indexOf('\n');
    if(nl == -1)
        return;
    lastLine = lineBuffer.left(nl);
    lineBuffer = lineBuffer.right(lineBuffer.length() - nl - 1);
    processLine(&lastLine);
    if(connected) {
        interactTimer.setSingleShot(true);
        interactTimer.start(timeOut);
    }
}

void SMTP::socketError(QAbstractSocket::SocketError socketError)
{
    kDebug() << socketError << sock->errorString();
    Q_UNUSED(socketError);
    emit error(ConnectError);
    socketClosed();
}

void SMTP::socketClosed()
{
    timeOutTimer.stop();
    kDebug() << "connection terminated";
    connected = false;
    if (sock)
        sock->deleteLater();
    sock = 0;
    emit connectionClosed();
}

void SMTP::processLine(QString *line)
{
    int i, stat;
    QString tmpstr;

    i = line->indexOf(' ');
    tmpstr = line->left(i);
    if(i > 3)
        kDebug() << "warning: SMTP status code longer than 3 digits: " << tmpstr;
    stat = tmpstr.toInt();
    serverState = (SMTPServerStatus)stat;
    lastState = state;

    kDebug() << "smtp state: [" << stat << "][" << *line << "]";

    switch(stat){
    case Greet:     //220
        state = In;
        writeString = QString::fromLatin1("helo %1\r\n").arg(domainName);
        kDebug() << "out: " << writeString;
        sock->write(writeString.toAscii().constData(), writeString.length());
        break;
    case Goodbye:   //221
        state = Quit;
        break;
    case Successful://250
        switch(state){
        case In:
            state = Ready;
            writeString = QString::fromLatin1("mail from: %1\r\n").arg(senderAddress);
            kDebug() << "out: " << writeString;
            sock->write(writeString.toAscii().constData(), writeString.length());
            break;
        case Ready:
            state = SentFrom;
            writeString = QString::fromLatin1("rcpt to: %1\r\n").arg(recipientAddress);
             kDebug() << "out: " << writeString;
            sock->write(writeString.toAscii().constData(), writeString.length());
            break;
        case SentFrom:
            state = SentTo;
            writeString = QLatin1String("data\r\n");
             kDebug() << "out: " << writeString;
            sock->write(writeString.toAscii().constData(), writeString.length());
            break;
        case Data:
            state = Finished;
            finished = true;
            emit messageSent();
            break;
        default:
            state = CError;
            kDebug() << "smtp error (state error): [" << lastState << "]:[" << stat << "][" << *line << "]";
            socketClosed();
            emit error(Command);
            break;
        }
        break;
    case ReadyData: //354
        state = Data;
        writeString = QString::fromLatin1("Subject: %1\r\n").arg(messageSubject);
        writeString += messageHeader;
        writeString += "\r\n";
        writeString += messageBody;
        writeString += QLatin1String(".\r\n");
        kDebug() << "out: " << writeString;
        sock->write(writeString.toAscii().constData(), writeString.length());
        break;
    case Error:     //501
        state = CError;
        kDebug() << "smtp error (command error): [" << lastState << "]:[" << stat << "][" << *line << "]\n";
        socketClosed();
        emit error(Command);
        break;
    case Unknown:   //550
        state = CError;
        kDebug() << "smtp error (unknown user): [" << lastState << "]:[" << stat << "][" << *line << "]";
        socketClosed();
        emit error(UnknownUser);
        break;
    default:
        state = CError;
        kDebug() << "unknown response: [" << lastState << "]:[" << stat << "][" << *line << "]";
        socketClosed();
        emit error(UnknownResponse);
    }
}

#include "smtp.moc"
