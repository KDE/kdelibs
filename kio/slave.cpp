#include "kio/slave.h"
#include <ksock.h>
#include <time.h>
#include <kdebug.h>
#include <klocale.h>
#include <dcopclient.h>
#include <kio/global.h>
#include <stdlib.h>
#include <kapp.h>
#include <stdio.h>
#include "kio/kservice.h"

#include <config.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp"
#endif

using namespace KIO;

void Slave::accept(KSocket *socket)
{
    printf("got a socket\n");
    slaveconn.init(socket);
    delete serv;
    serv = 0;
}

Slave::Slave(KServerSocket *socket, const QString &protocol)
  : SlaveInterface(&slaveconn), serv(socket), contacted(false)
{
    m_protocol = protocol;
    dead = false;
    contact_started = time(0);
    connect(serv, SIGNAL(accepted( KSocket* )),
	    SLOT(accept(KSocket*) ) );
}

void Slave::gotInput( int )
{
    if (!dispatch())
    {
        QString arg = m_protocol;
        if (!m_host.isEmpty())
            arg += "://"+m_host;
        emit error(ERR_SLAVE_DIED, arg);
        emit slaveDied(this);
    }
}

void Slave::gotAnswer( int )
{
    int cmd;
    QByteArray data;
    bool ok = true;

    if (slaveconn.read( &cmd, data ) == -1)
	ok = false;

    if (ok && (cmd != MSG_CONNECTED))
        ok = false;

    if (ok)
    {
	emit connected();
        slaveconn.connect(this, SLOT(gotInput(int)));
    }
    else
    {
        slaveconn.close();
        // TODO: Report start up error to someone who is interested
        dead = true;
    }
}

void Slave::kill()
{
    dead = true; // OO can be such simple.
}

void Slave::openConnection( const QString &host, int port,
                            const QString &user, const QString &passwd)
{
    m_host = host;
    m_port = port;
    m_user = user;
    m_passwd = passwd;

    slaveconn.connect(this, SLOT(gotAnswer(int)));

    QByteArray data;
    QDataStream stream( data, IO_WriteOnly );
    stream << m_host << m_port << m_user << m_passwd;
    slaveconn.send( CMD_CONNECT, data );
}


Slave* Slave::createSlave( const KURL& url, int& error, QString& error_text )
{
    kDebugInfo("createSlave for %s", debugString(url.url()));

    DCOPClient *client = kapp->dcopClient();
    if (!client->isAttached())
	client->attach();

    // Check kioslave is running
    if (!client->isApplicationRegistered( "kioslave" ))
    {
        kDebugInfo("Trying to start kioslave");
#if 1
        // Launch the kioslave service
        QString error;
        QCString dcopName;
        if (KService::startServiceByDesktopName( "kioslave",
		QString::null, dcopName, error))
        {
           kDebugError("Can't launch kioslave: '%s'", error.ascii());
           return 0;
        }
        if (dcopName != "kioslave")
        {
           kDebugError("Error launching kioslave: got '%s' but expected '%s'",
		dcopName.data(), "kioslave");
           return 0;
        }
#else
        system("kioslave");
#endif
    }

    QByteArray params, reply;
    QCString replyType;
    QDataStream stream(params, IO_WriteOnly);
    stream << url.protocol();

    if (!client->call("kioslave", "kioslave", "createSlave(QString)", params, replyType, reply)) {
	error_text = i18n("can't talk to kioslave");
	error = KIO::ERR_INTERNAL;
	return 0;
    }

    QDataStream output(reply, IO_ReadOnly);
    QString ticket;
    output >> ticket;

    if (ticket.left(5) == QString::fromLatin1("error")) {
	error_text = ticket.mid(6);
	error = KIO::ERR_INTERNAL;
	return 0;
    }

    QCString domainname;
    domainname.sprintf(_PATH_TMP"/kio_%i_XXXXXX", (int)getuid());

    // If your plattform doesn't know mkstemp create a static function in KTempFile and
    // let this code use it
    if (mkstemp(domainname.data()) == -1) {
	error_text = strerror(errno);
	error = KIO::ERR_INTERNAL;
	return 0;
    }

    KServerSocket *kss = new KServerSocket(domainname);
    stream.device()->reset(); // flushes the buffer

    stream << ticket;

    // may crunch it - but what for? The user is the only one to write to the socket
    stream << QString::fromLatin1(domainname);

    // send it the data and let it connect to us as soon as we're back in the event loop
    if (!client->send("kioslave", "kioslave", "connectSlave(QString,QString)", params)) {
	error_text = i18n("can't talk to kioslave");
	error = KIO::ERR_INTERNAL;
	return 0;
    }

    return new Slave(kss, url.protocol());
}

#include "slave.moc"
