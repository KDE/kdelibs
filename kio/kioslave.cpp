#include "kioslave.h"
#include <stdio.h>
#include <dcopclient.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <ksock.h>
#include <sys/socket.h>
#include <kdebug.h>
#include <time.h>
#include <kstddirs.h>
#include <ltdl.h>
#include <qfile.h>
#include <klibloader.h>
#include "kio/global.h"
#include "kio/connection.h"
#include "kio/slavewrapper.h"

#include <X11/Xlib.h>

using namespace KIO;

struct SlaveEntry {
    QString ticket;
    KIO::Connection conn;
};

template class QList<SlaveEntry>;

KIODaemon::KIODaemon(int &argc, char **argv) :
    KUniqueApplication(argc, argv, "kioslave", false)
{
}

QString KIODaemon::createTicket()
{
    bool ok;
    QString tmp;
    do {
	tmp = "kioXXXXXX";
	for (int i = 3; i < 9; i++) {
	    int ran = random() % 36;
	    if (ran < 10)
		tmp.at(i) = '0' + ran;
	    else
		tmp.at(i) = 'a' + ran - 10;
	}
	ok = true;
	QListIterator<SlaveEntry> it(pendingSlaves);
	while (it.current()) {
	    if (tmp == it.current()->ticket) { // this is _soo_ unlikly ...
		ok = false;
		break;
	    }
	    ++it;
	}
    } while (!ok);

    return tmp;
}

void KIODaemon::connectSlave(const QString& ticket, const QString& path)
{
    SlaveEntry *theone = 0;

    QListIterator<SlaveEntry> it(pendingSlaves);
    while (it.current()) {
	if (it.current()->ticket == ticket) {
	    theone = it.current();
	    pendingSlaves.remove(it.current());
	    break;
	}
	++it;
    }

    if (!theone) {
	kDebugInfo(7016, "there is no such slave pending for %s", ticket.ascii());
	return;
    }

    QByteArray data;
    QDataStream str(data, IO_WriteOnly);
    str << path;

    theone->conn.send('C', data);
}

QString KIODaemon::createSlave(const QString& protocol)
{
    QString protocol_library = QString::fromLatin1("%1/.libs/kio_%2.la").arg(protocol).arg(protocol);
    if (!QFile::exists(protocol_library))
	 protocol_library = locate("lib", QString("kio_%1.la").arg(protocol));

    if (protocol_library.isNull()) {
	return "error: no such protocol";
    }

    int fd[2];

    if (::socketpair(PF_UNIX, SOCK_STREAM, 0, fd) == -1) {
	perror("socketpair");
	return QString::fromLatin1("error: pipe creation failed");
    }
    QApplication::flushX();

    int pid = fork();
    if (pid == -1) {
	perror("fork");
	return QString::fromLatin1("error: fork failed");
    }
    static char errors[] = { 'K', // OK
			     'L' // Loading failed
    };
    const int ERR_OK = 0;
    const int ERR_LOADING = 1;

    if (pid == 0) { // Child

	::close(ConnectionNumber(qt_xdisplay())); // make sure noone makes X connections to us
	::close(fd[0]);

	lt_dlhandle handle = lt_dlopen( protocol_library );

	if ( !handle ) {
	    kDebugInfo(7016, "trying to load support for %s failed with %s", protocol.ascii(), lt_dlerror() );
	    ::write(fd[1], errors + ERR_LOADING, 1);
	    exit(0);
	}
	
	KLibrary *lib = new KLibrary( protocol, protocol_library, handle );

	QCString symname;
	symname.sprintf("init_%s", protocol.latin1() );

	void* sym = lib->symbol( symname );
	if ( !sym )
	    exit(1);

	typedef SlaveBase* (*t_func)();
	t_func func = (t_func)sym;
	SlaveBase *serv = func();
	
	if( !serv ) {
	    kDebugInfo(7016, "KLibrary: The library does not offer a KDE compatible factory");
	    exit(1); // that was it
	}

	if (::write(fd[1], errors + ERR_OK, 1) != 1) {
	  perror("write");
	}
	
	SlaveWrapper *ksw = new SlaveWrapper(serv, fd[1]);
	ksw->dispatchLoop();

	exit(0);
	return QString::null; // never reached

    } else {
        ::close(fd[1]);

	// read the first character of the child to
	// see how it goes :/
	char result;
	if (::read(fd[0], &result, 1) != 1) {
	    perror("read");
	    return "error: pipe broken";
	}

	if (result == errors[ERR_OK]) {
	    SlaveEntry *slave = new SlaveEntry;
	    slave->ticket = createTicket();
	    slave->conn.init(fd[0], fd[0]);
	    pendingSlaves.append(slave);
	    return slave->ticket;
	}

	if (result == errors[ERR_LOADING]) {
	    return "error: loading failed";
	}

	// how can gcc know there are no more :/
	return QString::null;
    }
}

bool KIODaemon::process(const QCString &fun, const QByteArray &data,
			QCString &replyType, QByteArray &replyData)
{
    if (KUniqueApplication::process(fun, data, replyType, replyData))
	return true;

    QDataStream stream(data, IO_ReadOnly);
    QDataStream output(replyData, IO_WriteOnly);

    if (fun == "createSlave(QString)") {
	QString protocol;
	stream >> protocol;
	replyType = "QString";
	protocol = createSlave(protocol);
	output << protocol;
	return true;
    }

    if (fun == "connectSlave(QString,QString)") {
	QString ticket, path;
	stream >> ticket >> path;
	connectSlave(ticket, path);
	return true;
    }

    return false;
}

int main(int argc, char **argv)
{
  //  KCmdLineArgs::init(argc, argv, "kioslave",
  //  "a tool to start kio protocols", "0.0");

  // KUniqueApplication::addCmdLineOptions();

  if (!KUniqueApplication::start(argc, argv, "kioslave"))
    {
        fprintf(stderr, "kioslave already running!\n");
	return 0;
    }

    KIODaemon k(argc,argv);
    return k.exec(); // keep running
}

#include "kioslave.moc"
