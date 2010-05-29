/* $Id: smtp.cpp 532141 2006-04-21 10:38:02Z cartman $ */

#include <sys/utsname.h>
#include <unistd.h>
#include <stdio.h>

#include <kdebug.h>

#include "smtp.h"

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
    state = INIT;
    serverState = NONE;

    uname(&uts);
    domainName = uts.nodename;


    if(domainName.isEmpty())
        domainName = "somemachine.example.net";

    kdDebug() << "SMTP object created" << endl;

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
    int index = senderAddress.find('<');
    if (index == -1)
        return;
    senderAddress = senderAddress.mid(index + 1);
    index =  senderAddress.find('>');
    if (index != -1)
        senderAddress = senderAddress.left(index);
    senderAddress = senderAddress.simplifyWhiteSpace();
    while (1) {
        index =  senderAddress.find(' ');
        if (index != -1)
            senderAddress = senderAddress.mid(index + 1); // take one side
        else
            break;
    }
    index = senderAddress.find('@');
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
    kdDebug() << "started connect timer" << endl;
    connectTimer.start(100, true);
}

void SMTP::closeConnection(void)
{
    socketClose(sock);
}

void SMTP::sendMessage(void)
{
    if(!connected)
        connectTimerTick();
    if(state == FINISHED && connected){
        kdDebug() << "state was == FINISHED\n" << endl;
        finished = false;
        state = IN;
        writeString = QString::fromLatin1("helo %1\r\n").arg(domainName);
        write(sock->socket(), writeString.ascii(), writeString.length());
    }
    if(connected){
        kdDebug() << "enabling read on sock...\n" << endl;
        interactTimer.start(timeOut, true);
        sock->enableRead(true);
    }
}
#include <stdio.h>

void SMTP::connectTimerTick(void)
{
    connectTimer.stop();
//    timeOutTimer.start(timeOut, true);

    kdDebug() << "connectTimerTick called..." << endl;

    if(sock){
        delete sock;
        sock = 0L;
    }

    kdDebug() << "connecting to " << serverHost << ":" << hostPort << " ..... " << endl;
    sock = new KSocket(serverHost.ascii(), hostPort);

    if(sock == 0L || sock->socket() < 0) {
        timeOutTimer.stop();
        kdDebug() << "connection failed!" << endl;
        socketClose(sock);
        emit error(CONNECTERROR);
        connected = false;
        return;
    }
    connected = true;
    finished = false;
    state = INIT;
    serverState = NONE;

    connect(sock, SIGNAL(readEvent(KSocket *)), this, SLOT(socketRead(KSocket *)));
    connect(sock, SIGNAL(closeEvent(KSocket *)), this, SLOT(socketClose(KSocket *)));
    //    sock->enableRead(true);
    timeOutTimer.stop();
    kdDebug() << "connected" << endl;
}

void SMTP::connectTimedOut(void)
{
    timeOutTimer.stop();

    if(sock)
	sock->enableRead(false);
    kdDebug() << "socket connection timed out" << endl;
    socketClose(sock);
    emit error(CONNECTTIMEOUT);
}

void SMTP::interactTimedOut(void)
{
    interactTimer.stop();

    if(sock)
        sock->enableRead(false);
    kdDebug() << "time out waiting for server interaction" << endl;
    socketClose(sock);
    emit error(INTERACTTIMEOUT);
}

void SMTP::socketRead(KSocket *socket)
{
    int n, nl;

    kdDebug() << "socketRead() called..." << endl;
    interactTimer.stop();

    if(socket == 0L || socket->socket() < 0)
        return;
    n = read(socket->socket(), readBuffer, SMTP_READ_BUFFER_SIZE-1 );

    if(n < 0)
        return;

    readBuffer[n] = '\0';
    lineBuffer += readBuffer;
    nl = lineBuffer.find('\n');
    if(nl == -1)
        return;
    lastLine = lineBuffer.left(nl);
    lineBuffer = lineBuffer.right(lineBuffer.length() - nl - 1);
    processLine(&lastLine);
    if(connected)
        interactTimer.start(timeOut, true);
}

void SMTP::socketClose(KSocket *socket)
{
    timeOutTimer.stop();
    disconnect(sock, SIGNAL(readEvent(KSocket *)), this, SLOT(socketRead(KSocket *)));
    disconnect(sock, SIGNAL(closeEvent(KSocket *)), this, SLOT(socketClose(KSocket *)));
    socket->enableRead(false);
    kdDebug() << "connection terminated" << endl;
    connected = false;
    if(socket){
        delete socket;
        socket = 0L;
        sock = 0L;
    }
    emit connectionClosed();
}

void SMTP::processLine(QString *line)
{
    int i, stat;
    QString tmpstr;

    i = line->find(' ');
    tmpstr = line->left(i);
    if(i > 3)
        kdDebug() << "warning: SMTP status code longer then 3 digits: " << tmpstr << endl;
    stat = tmpstr.toInt();
    serverState = (SMTPServerStatus)stat;
    lastState = state;

    kdDebug() << "smtp state: [" << stat << "][" << *line << "]" << endl;

    switch(stat){
    case GREET:     //220
        state = IN;
        writeString = QString::fromLatin1("helo %1\r\n").arg(domainName);
        kdDebug() << "out: " << writeString << endl;
	write(sock->socket(), writeString.ascii(), writeString.length());
        break;
    case GOODBYE:   //221
        state = QUIT;
        break;
    case SUCCESSFUL://250
        switch(state){
        case IN:
            state = READY;
            writeString = QString::fromLatin1("mail from: %1\r\n").arg(senderAddress);
            kdDebug() << "out: " << writeString << endl;
            write(sock->socket(), writeString.ascii(), writeString.length());
            break;
        case READY:
            state = SENTFROM;
            writeString = QString::fromLatin1("rcpt to: %1\r\n").arg(recipientAddress);
             kdDebug() << "out: " << writeString << endl;
            write(sock->socket(), writeString.ascii(), writeString.length());
            break;
        case SENTFROM:
            state = SENTTO;
            writeString = QString::fromLatin1("data\r\n");
             kdDebug() << "out: " << writeString << endl;
            write(sock->socket(), writeString.ascii(), writeString.length());
            break;
        case DATA:
            state = FINISHED;
            finished = true;
            sock->enableRead(false);
            emit messageSent();
            break;
        default:
            state = CERROR;
            kdDebug() << "smtp error (state error): [" << lastState << "]:[" << stat << "][" << *line << "]" << endl;
            socketClose(sock);
            emit error(COMMAND);
            break;
        }
        break;
    case READYDATA: //354
        state = DATA;
        writeString = QString::fromLatin1("Subject: %1\r\n").arg(messageSubject);
        writeString += messageHeader;
        writeString += "\r\n";
        writeString += messageBody;
        writeString += QString::fromLatin1(".\r\n");
        kdDebug() << "out: " << writeString;
        write(sock->socket(), writeString.ascii(), writeString.length());
        break;
    case ERROR:     //501
        state = CERROR;
        kdDebug() << "smtp error (command error): [" << lastState << "]:[" << stat << "][" << *line << "]\n" << endl;
        socketClose(sock);
        emit error(COMMAND);
        break;
    case UNKNOWN:   //550
        state = CERROR;
        kdDebug() << "smtp error (unknown user): [" << lastState << "]:[" << stat << "][" << *line << "]" << endl;
        socketClose(sock);
        emit error(UNKNOWNUSER);
        break;
    default:
        state = CERROR;
        kdDebug() << "unknown response: [" << lastState << "]:[" << stat << "][" << *line << "]" << endl;
        socketClose(sock);
        emit error(UNKNOWNRESPONSE);
    }
}

#include "smtp.moc"
